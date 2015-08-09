/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "document.hpp"
#include "mainwindow.hpp"
#include "ap240_constants.hpp"
#include <ap240/digitizer.hpp>
#include <adlog/logger.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/waveform.hpp>
#include <adinterface/controlserver.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/float.hpp>
#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
#include <adportable/semaphore.hpp>
#include <adportable/sgfilter.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/waveform_processor.hpp>
#include <qtwrapper/settings.hpp>
#include <app/app_version.h>
#include <coreplugin/documentmanager.h>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <chrono>
#include <deque>
#include <fstream>
#include <string>
#include <thread>

using namespace ap240;

document * document::instance_ = 0;
std::mutex document::mutex_;

namespace ap240 {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "ap240";
        }
    };

    class histogram {
        histogram( const histogram & ) = delete;
        histogram& operator = ( const histogram& ) = delete;
        std::mutex mutex_;
    public:
        histogram() : trigger_count_(0) {}

        void clear() {
            std::lock_guard< std::mutex > lock( mutex_ );
            meta_.actualPoints = 0;
        }

        void append( const threshold_result& result ) {

            std::lock_guard< std::mutex > lock( mutex_ );

            if ( meta_.actualPoints != result.data->meta_.actualPoints ||
                 !adportable::compare<double>::approximatelyEqual( meta_.initialXOffset, result.data->meta_.initialXOffset ) ||
                 !adportable::compare<double>::approximatelyEqual( meta_.xIncrement, result.data->meta_.xIncrement ) ) {
                
                trigger_count_ = 0;
                meta_ = result.data->meta_;
                
                data_.resize( meta_.actualPoints );
                std::fill( data_.begin(), data_.end(), 0 );
                std::cout << "histogramo clear" << std::endl;
            }
            std::for_each( result.index.begin(), result.index.end(), [this]( uint32_t idx ){
                    data_[ idx ] ++; });
            ++trigger_count_;
        }

        size_t trigger_count() const { return trigger_count_; }

        size_t getHistogram( std::vector< std::pair<double, uint32_t> >& histgram, ap240::metadata& meta ) {
            std::lock_guard< std::mutex > lock( mutex_ );
            meta = meta_;
            histgram.clear();
            double t0 = meta_.initialXOffset;
            for ( auto it = data_.begin(); it < data_.end(); ++it ) {
                if ( *it ) {
                    double t = meta_.initialXOffset + std::distance( data_.begin(), it ) * meta_.xIncrement;
                    histgram.push_back( std::make_pair( t, *it ) );
                }
            }
            return trigger_count_;
        }
        
    private:
        // keep metadata
        ap240::metadata meta_;
        std::atomic< size_t > trigger_count_;
        std::vector< uint32_t > data_;
    };

    class document::impl {

        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        std::string time_datafile_;
        std::string hist_datafile_;        

    public:
        impl() : worker_stop_( false )
               , work_( io_service_ )
               , histogram_( std::make_shared< histogram >() ) {
            
            time_datafile_ = ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data/ap240_time_data.txt" ).string();
            hist_datafile_ = ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data/ap240_histgram.txt" ).string();
            
        }
        
        ~impl() {
            stop();
        }
        
        inline boost::asio::io_service& io_service() { return io_service_; }
        inline const std::string& hist_datafile() const { return hist_datafile_; };
        
        adportable::semaphore sema_;
        bool worker_stop_;
        std::chrono::steady_clock::time_point time_handled_;
        std::vector< std::thread > threads_;
        std::mutex que_mutex_;
        
        std::deque< std::pair<
                        std::shared_ptr< const waveform >
                        , std::shared_ptr< const waveform >
                        > > que_;

        inline void clearHistogram() {
            histogram_->clear();
        }

        inline size_t getHistogram( std::vector< std::pair< double, uint32_t > >& data, ap240::metadata& meta ) {
            return histogram_->getHistogram( data, meta );
        }
        
    private:
        std::mutex que2_mutex_;        
        std::deque< std::pair< std::shared_ptr< threshold_result>
                               , std::shared_ptr< threshold_result > > > que2_;
        std::shared_ptr< histogram > histogram_;

    public:

        std::array< ap240::threshold_method, 2 > thresholds_;

        inline document::waveforms_t findWaveform() {
            std::lock_guard< std::mutex > lock( que2_mutex_ );
            if ( que2_.empty() )
                return waveforms_t( 0, 0 );
            return que2_.back();
        }

        void run() {
            if ( threads_.empty() ) {
                unsigned ncores = std::thread::hardware_concurrency();
                if ( ncores == 0 )
                    ncores = 4;
                while( ncores-- )
                    threads_.push_back( adportable::asio::thread( [this] { io_service_.run(); } ) );
                threads_.push_back( adportable::asio::thread( [this] { worker(); } ) );
            }
        }
        
        void stop() {
            io_service_.stop();
            worker_stop_ = true;
            sema_.signal();
            for ( auto& t: threads_ )
                t.join();
        }

        void find_threshold_timepoints( const waveform& data, const ap240::threshold_method& method
                                        , std::vector< uint32_t >& elements, std::vector<double>& processed ) {

            bool findUp = method.slope == ap240::threshold_method::CrossUp;
            bool flag;
            size_t nfilter = size_t( method.response_time / data.meta_.xIncrement );

            if ( method.use_filter ) {

                processed.resize( data.size() );

                if ( data.meta_.dataType == 1 ) {
                    for ( size_t i = 0; i < data.size(); ++i )
                        processed[ i ] = data.toVolts( *(data.begin<int8_t>() + i) );
                } else if ( data.meta_.dataType == 4 ) {
                    for ( size_t i = 0; i < data.size(); ++i )
                        processed[ i ] = data.toVolts( *(data.begin<int32_t>() + i) );
                }

                if ( method.filter == ap240::threshold_method::SG_Filter ) {
                    adportable::SGFilter filter( method.sgPoints & 01 ); // make odd
                    for ( size_t i = method.sgPoints / 2 + 1; i < data.size() - method.sgPoints / 2 - 1; ++i )
                        processed[ i ] = filter( &processed[i] );
                    
                } else if ( method.filter == ap240::threshold_method::DFT_Filter ) {
                    adcontrols::waveform::fft::lowpass_filter( processed, data.meta_.xIncrement, method.cutOffMHz * 1.0e6 );
                }
                double level = ( method.threshold_level / 1000.0 );
                auto it = processed.begin();
                while ( it != processed.end() ) {
                    if ( ( it = adportable::waveform_processor().find_threshold_element( it, processed.end(), level, flag ) ) != processed.end() ) {
                        if ( flag == findUp )
                            elements.push_back( std::distance( processed.begin(), it ) );
                        std::advance( it, nfilter );
                    }
                }
                return;
            }
            
            if ( data.meta_.dataType == 1 ) { // sizeof(int8_t)
                double level = ( ( method.threshold_level / 1000.0 ) + data.meta_.scaleOffset ) / data.meta_.scaleFactor;
                
                typedef int8_t T;
                auto it = data.begin<T>();
                while ( it != data.end<T>() ) {
                    if ( ( it = adportable::waveform_processor().find_threshold_element( it, data.end<T>(), level, flag ) ) != data.end<T>() ) {
                        if ( flag == findUp )
                            elements.push_back( std::distance( data.begin<T>(), it ) );
                        std::advance( it, nfilter );
                    }
                }
                
            } else if ( data.meta_.dataType == 4 ) { // sizeof(int32_t)
                // scaleFactor = Volts/LSB  (1.0V FS = 0.00390625)
                double level_per_trigger = ( ( method.threshold_level / 1000.0 ) + data.meta_.scaleOffset ) / data.meta_.scaleFactor;
                double level = level_per_trigger * data.meta_.actualAverages;
                
                typedef int32_t T;
                auto it = data.begin<T>();
                while ( it != data.end<T>() ) {
                    if ( ( it = adportable::waveform_processor().find_threshold_element( it, data.end<T>(), level, flag ) ) != data.end<T>() ) {
                        if ( flag == findUp )                        
                            elements.push_back( std::distance( data.begin<T>(), it ) );
                        std::advance( it, nfilter );                        
                    }
                }
            }
        }

        void handle_waveform( std::pair<std::shared_ptr< const waveform >, std::shared_ptr< const waveform > > pair ) {

            std::pair< std::shared_ptr< threshold_result >, std::shared_ptr< threshold_result > > results;

            if ( pair.first ) {
                
                results.first = std::make_shared< threshold_result >( pair.first );

                if ( thresholds_[0].enable ) {

                    find_threshold_timepoints( *pair.first, thresholds_[ 0 ], results.first->index, results.first->processed );
                    histogram_->append( *results.first );

                    auto& w = *pair.first;
                    
                    std::lock_guard< std::mutex > lock( document::mutex_ );
                    
                    std::ofstream of( time_datafile_, std::ios_base::out | std::ios_base::app );
                    of << boost::format("\n%d, %.8lf, ") % w.serialnumber_ % w.meta_.initialXTimeSeconds
                       << w.timeSinceEpoch_
                       << boost::format(", %.8e, %.8e" ) % w.meta_.scaleFactor % w.meta_.scaleOffset
                       << boost::format(", %.8e" ) % w.meta_.initialXOffset;
                    
                    for ( auto& idx: results.first->index ) {
                        auto v = w[ idx ];
                        of << boost::format(", %.14le, %d" ) % v.first % v.second;
                    }
                }
            }
                
            if ( pair.second ) { //&& thresholds_[1].enable ) {
                results.second = std::make_shared< threshold_result >( pair.second );
                if ( thresholds_[1].enable )
                    find_threshold_timepoints( *pair.second, thresholds_[ 1 ], results.second->index, results.second->processed );
            }
            
            do {
                std::lock_guard< std::mutex > lock( que2_mutex_ );
                if ( que2_.size() > 1536 )
                    que2_.erase( que2_.begin(), que2_.begin() + 512 );
                que2_.push_back( results );
            } while( 0 );

            sema_.signal();
        }
        
        void worker() {
            while ( true ) {

                sema_.wait();

                if ( worker_stop_ )
                    return;

                auto tp = std::chrono::steady_clock::now();
                if ( std::chrono::duration_cast<std::chrono::milliseconds>( tp - time_handled_ ).count() > 200 ) {
                    time_handled_ = tp;
                    emit document::instance()->on_waveform_received();
                }
                
            }
        }
    };

}
    
document::document() : impl_( new impl() )
                     , digitizer_( new ap240::digitizer )
                     , device_status_( 0 )
                     , method_( std::make_shared< ap240::method >() )
                     , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                 , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                 , QLatin1String( "ap240" ) ) )
{
}

document::~document()
{
    delete impl_;
    delete digitizer_;
}

document *
document::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new document;
    }
    return instance_;
}

void
document::ap240_connect()
{
    digitizer_->connect_reply( boost::bind( &document::reply_handler, this, _1, _2 ) );
    digitizer_->connect_waveform( boost::bind( &document::waveform_handler, this, _1, _2, _3 ) );
    digitizer_->peripheral_initialize();
    impl_->run();
}

void
document::prepare_for_run()
{
    using adcontrols::controlmethod::MethodItem;

    ap240::method m;
    MainWindow::instance()->getControlMethod( m );
    digitizer_->peripheral_prepare_for_run( m );
}

void
document::ap240_start_run()
{
	digitizer_->peripheral_run();
}

void
document::ap240_stop()
{
	digitizer_->peripheral_stop();
}

void
document::ap240_trigger_inject()
{
	digitizer_->peripheral_trigger_inject();
}

int32_t
document::device_status() const
{
    return device_status_;
}

void
document::reply_handler( const std::string& method, const std::string& reply )
{
	emit on_reply( QString::fromStdString( method ), QString::fromStdString( reply ) );
    if ( method == "InitialSetup" && reply == "success" ) {
        device_status_ = controlserver::eStandBy;
        emit on_status( device_status_ );
    }
}

bool
document::waveform_handler( const waveform * ch1, const waveform * ch2, ap240::method& )
{
    auto pair = std::make_pair( ( ch1 ? ch1->shared_from_this() : 0 ), ( ch2 ? ch2->shared_from_this() : 0 ) );

    impl_->io_service().post( [this,pair](){ impl_->handle_waveform( pair ); } );

    std::lock_guard< std::mutex > lock( impl_->que_mutex_ );
    if ( impl_->que_.size() > 1500 )
        impl_->que_.erase( impl_->que_.begin(), impl_->que_.begin() + 500 );
    impl_->que_.push_back( pair );

    return false; // no protocol-acquisition handled.
}

document::waveforms_t
document::findWaveform( uint32_t serialnumber )
{
    (void)serialnumber;
    return impl_->findWaveform(); // for GUI display purpose
}

// static
bool
document::toMassSpectrum( adcontrols::MassSpectrum& sp, const waveform& waveform )
{
    using namespace adcontrols::metric;

    sp.setCentroid( adcontrols::CentroidNone );
    
    adcontrols::MSProperty prop = sp.getMSProperty();
    adcontrols::MSProperty::SamplingInfo info( 0
                                               , uint32_t( waveform.meta_.initialXOffset / waveform.meta_.xIncrement + 0.5 )
                                               , uint32_t( waveform.size() )
                                               , waveform.method_.hor_.nbrAvgWaveforms
                                               , 0 );
    info.fSampInterval( waveform.meta_.xIncrement );
    prop.acceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    
    prop.setTimeSinceInjection( waveform.meta_.initialXTimeSeconds );
    prop.setTimeSinceEpoch( waveform.timeSinceEpoch_ );
    prop.setDataInterpreterClsid( "ap240" );

    ap240::device_data data;
    data.ident = *waveform.ident_;
    data.meta = waveform.meta_;
    std::string ar;
    adportable::binary::serialize<>()( data, ar );
    prop.setDeviceData( ar.data(), ar.size() );

    // prop.setDeviceData(); TBA
    sp.setMSProperty( prop );
    sp.resize( waveform.size() );
	int idx = 0;
    if ( waveform.meta_.dataType == 1 ) {
        for ( auto y = waveform.begin<int8_t>(); y != waveform.end<int8_t>(); ++y )
            sp.setIntensity( idx++, *y );
    } else {
        double dbase, rms;
        double tic = adportable::spectrum_processor::tic( waveform.size(), waveform.begin<int32_t>(), dbase, rms );
        
        for ( auto y = waveform.begin<int32_t>(); y != waveform.end<int32_t>(); ++y )
            sp.setIntensity( idx++, *y - dbase );
        
    }
    // mass array tba
	return true;
}

// static
bool
document::appendOnFile( const std::wstring& path
                        , const std::wstring& title
                        , const adcontrols::MassSpectrum& ms, std::wstring& id )
{
    adfs::filesystem fs;

    std::wcout << L"########## document::appendOnFile(" << path << L", " << title << L") id=" << id << std::endl;
    std::cout << "ms size: " << ms.size() << std::endl;
    
	if ( ! boost::filesystem::exists( path ) ) {
		if ( ! fs.create( path.c_str() ) )
			return false;
	} else {
		if ( ! fs.mount( path.c_str() ) )
			return false;
	}
	adfs::folder folder = fs.addFolder( L"/Processed/Spectra" );

    if ( folder ) {
		adfs::file file = folder.addFile( adfs::create_uuid(), title );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            id = file.id();
            if ( file.save( ms ) ) //adfs::cpio< adcontrols::MassSpectrum >::save( ms, file ) )
				file.commit();
        }
	}
    return true;
    
}

void

document::initialSetup()
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );

    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "ap240::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    QString path = recentFile( Constants::GRP_DATA_FILES, false );
    if ( path.isEmpty() ) {
        path = QString::fromStdWString( ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data" ).generic_wstring() );
    } else {
        path = QFileInfo( path ).path();
    }
    // fake project directory for help initial openfiledialog location
    Core::DocumentManager::setProjectsDirectory( path );
    Core::DocumentManager::setUseProjectsDirectory( true );

    boost::filesystem::path mfile( dir / "ap240.xml" );
    ap240::method m;
    if ( load( QString::fromStdWString( mfile.wstring() ), m ) )
        setControlMethod( m, QString() ); // don't save default name
    
    try {
        std::vector< ap240::threshold_method > x;
        std::wifstream inf( boost::filesystem::path( dir / "ap240_slope_time_method.xml" ).string() );
        boost::archive::xml_wiarchive ar( inf );
        ar >> boost::serialization::make_nvp( "threshold_methods", x );
        for ( size_t i = 0; i < x.size(); ++i )
            set_threshold_method( int( i ), x[ i ] );
        std::cout << "############ ap240::threshold_method load success" << std::endl;        
    } catch( ... ) {
        std::cout << "############ ap240::threshold_method load failed" << std::endl;
    }
}

void
document::finalClose()
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "ap240::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }
    ap240::method m;
    MainWindow::instance()->getControlMethod( m );
    boost::filesystem::path fname( dir / "ap240.xml" );
    save( QString::fromStdWString( fname.wstring() ), m );

    std::vector< ap240::threshold_method > x{ impl_->thresholds_[0], impl_->thresholds_[1] };
    std::wofstream outf( boost::filesystem::path( dir / "ap240_slope_time_method.xml" ).string() );
    boost::archive::xml_woarchive ar( outf );
    ar << boost::serialization::make_nvp( "threshold_methods", x );
    
}

void
document::addToRecentFiles( const QString& filename )
{
    qtwrapper::settings(*settings_).addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, filename );
}

QString
document::recentFile( const char * group, bool dir_on_fail )
{
    if ( group == 0 )
        group = Constants::GRP_DATA_FILES;

    QString file = qtwrapper::settings( *settings_ ).recentFile( group, Constants::KEY_FILES );
    if ( !file.isEmpty() )
        return file;

    if ( dir_on_fail ) {
        file = Core::DocumentManager::currentFile();
        if ( file.isEmpty() )
            file = qtwrapper::settings( *settings_ ).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );

        if ( !file.isEmpty() ) {
            QFileInfo fi( file );
            return fi.path();
        }
        return QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() );
    }
    return QString();
}

bool
document::load( const QString& filename, ap240::method& m )
{
    try {

        std::wifstream inf( filename.toStdString() );
        boost::archive::xml_wiarchive ar( inf );
        
        ar >> boost::serialization::make_nvp( "ap240_method", m );

        return true;

    } catch( ... ) {
        std::cout << "############# ap240::method load failed" << std::endl;
    }
    return false;
}

bool
document::save( const QString& filename, const ap240::method& m )
{
    std::wofstream outf( filename.toStdString() );

    boost::archive::xml_woarchive ar( outf );
    ar << boost::serialization::make_nvp( "ap240_method", m );
    return true;
}

std::shared_ptr< ap240::method >
document::controlMethod() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return method_;
}

void
document::setControlMethod( const ap240::method& m, const QString& filename )
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        method_ = std::make_shared< ap240::method >( m );
        digitizer_->peripheral_prepare_for_run( m );
    } while(0);

    if ( ! filename.isEmpty() ) {
        ctrlmethod_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, filename );
    }

    emit onControlMethodChanged( filename );
}

void
document::set_threshold_method( int ch, const ap240::threshold_method& m )
{
    if ( ch < impl_->thresholds_.size() ) {
        impl_->thresholds_[ ch ] = m;
        impl_->clearHistogram();
        emit on_threshold_method_changed( ch );
        
    }
}

const ap240::threshold_method&
document::threshold_method( int ch ) const
{
    if ( ch < impl_->thresholds_.size() )
        return impl_->thresholds_[ ch ];
    throw std::runtime_error(0);
}


std::shared_ptr< adcontrols::MassSpectrum >
document::getHistogram() const
{
    ap240::metadata meta;
    std::vector< std::pair< double, uint32_t > > hist;

    size_t trigCount = impl_->getHistogram( hist, meta );
    auto sp = std::make_shared< adcontrols::MassSpectrum >();

    using namespace adcontrols::metric;

    sp->setCentroid( adcontrols::CentroidNative );
    
    adcontrols::MSProperty prop = sp->getMSProperty();
    adcontrols::MSProperty::SamplingInfo info( 0
                                               , uint32_t( meta.initialXOffset / meta.xIncrement + 0.5 )
                                               , uint32_t( meta.actualPoints ) // this is for acq. time range calculation
                                               , trigCount
                                               , 0 );
    info.fSampInterval( meta.xIncrement );
    prop.acceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    
    prop.setTimeSinceInjection( meta.initialXTimeSeconds );
    prop.setTimeSinceEpoch( 0 );
    prop.setDataInterpreterClsid( "ap240" );

    // ap240::device_data data;
    // // data.ident = *waveform.ident_;
    // data.meta = waveform.meta_;
    // std::string ar;
    // adportable::binary::serialize<>()( data, ar );
    // prop.setDeviceData( ar.data(), ar.size() );

    // prop.setDeviceData(); TBA
    sp->setMSProperty( prop );
    sp->resize( hist.size() );
    size_t idx = 0;
    for ( const auto& d: hist ) {
        sp->setTime( idx, d.first );
        sp->setIntensity( idx, d.second );
        ++idx;
    }
	return sp;
}

void
document::save_histgram( size_t tickCount, const adcontrols::MassSpectrum& hist )
{
    std::ofstream of( impl_->hist_datafile(), std::ios_base::out | std::ios_base::app );

    const double * times = hist.getTimeArray();
    const double * intens = hist.getIntensityArray();
    const adcontrols::MSProperty& prop = hist.getMSProperty();

    of << boost::format("\n%d, %.8lf, %.14le") % tickCount % prop.timeSinceInjection() % prop.instTimeRange().first;
    for ( size_t i = 0; i < hist.size(); ++i )
        of << boost::format(", %.14le, %d" ) % times[ i ] % uint32_t( intens[ i ] );

}
