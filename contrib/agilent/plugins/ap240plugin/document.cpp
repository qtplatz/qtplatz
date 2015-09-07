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

#include "ap240_constants.hpp"
#include "document.hpp"
#include "icontrollerimpl.hpp"
#include "mainwindow.hpp"
#include <ap240/digitizer.hpp>
#include <ap240/histogram.hpp>
#include <ap240spectrometer/threshold_result.hpp>
#include <adlog/logger.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/waveform.hpp>
#include <adextension/isequenceimpl.hpp>
#include <adinterface/controlserver.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/debug.hpp>
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
#include <atomic>
#include <chrono>
#include <deque>
#include <fstream>
#include <string>
#include <thread>

using namespace ap240;

namespace ap240 {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "ap240";
        }
    };

    class document::impl {

        boost::asio::io_service io_service_;
        boost::asio::io_service::work work_;
        std::string time_datafile_;
        std::string hist_datafile_;     
        std::atomic<int> postCount_;
        std::atomic<size_t> waveform_proc_count_;        
        std::atomic<size_t> waveform_post_count_;
        std::atomic<uint32_t> worker_data_serialnumber_;
        std::unique_ptr< adextension::iSequenceImpl > iSequenceImpl_;
        std::shared_ptr< ap240::iControllerImpl > iControllerImpl_;
        
    public:
        static std::atomic< document * > instance_;
        static std::mutex mutex_;

    public:
        impl() : worker_stop_( false )
               , work_( io_service_ )
               , histogram_( std::make_shared< histogram >() )
               , postCount_( 0 )
               , round_trip_( 0 )
               , waveform_post_count_( 0 )
               , waveform_proc_count_( 0 )
               , worker_data_serialnumber_( 0 )
               , iSequenceImpl_( new adextension::iSequenceImpl() )
               , iControllerImpl_( std::make_shared< ap240::iControllerImpl >() ) {
            
            time_datafile_ = ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data/ap240_time_data.txt" ).string();
            hist_datafile_ = ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data/ap240_histogram.txt" ).string();
            
        }
        
        ~impl() {
            stop();
            std::cout << "########### document::impl DTOR ##############" << std::endl;
        }
        
        inline boost::asio::io_service& io_service() { return io_service_; }
        inline const std::string& hist_datafile() const { return hist_datafile_; };

        inline std::atomic<size_t>& waveform_post_count() { return waveform_post_count_; }
        inline std::atomic<size_t>& waveform_proc_count() { return waveform_proc_count_; }
        
        adportable::semaphore sema_;
        bool worker_stop_;
        std::chrono::microseconds round_trip_;
        std::chrono::steady_clock::time_point time_handled_;
        std::vector< std::thread > threads_;
        std::mutex que_mutex_;
        
        std::vector< std::pair<
                        std::shared_ptr< const ap240x::waveform >
                        , std::shared_ptr< const ap240x::waveform >
                        > > que_;

        inline void clearHistogram() {
            histogram_->clear();
        }

        inline size_t getHistogram( std::vector< std::pair< double, uint32_t > >& data
                                    , ap240x::metadata& meta, uint32_t& serialnumber, uint64_t& timeSinceEpoch ) {
            return histogram_->getHistogram( data, meta, serialnumber, timeSinceEpoch );
        }

        inline double triggers_per_sec() const {
            return histogram_->triggers_per_sec();
        }

        inline void waveform_drawn() {
            if ( postCount_ ) {
                --postCount_;
                round_trip_ = std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::steady_clock::now() - time_handled_ );
            }
        }

        inline size_t unprocessed_trigger_counts() const { return waveform_post_count_ - waveform_proc_count_; }
        
        inline adextension::iSequenceImpl * iSequence() { return iSequenceImpl_.get(); }
        inline ap240::iControllerImpl * iController() { return iControllerImpl_.get(); }
        
    private:
        typedef std::pair< std::shared_ptr< ap240x::threshold_result >, std::shared_ptr< ap240x::threshold_result > > threshold_result_pair_t;
        
        std::mutex que2_mutex_;
        std::vector< threshold_result_pair_t > que2_;
        std::shared_ptr< histogram > histogram_;
        std::array< std::shared_ptr< adcontrols::threshold_method >, 2 > threshold_methods_;
        
    public:

        inline bool set_threshold_method( int ch, const adcontrols::threshold_method& m ) {

            if ( ch < threshold_methods_.size() ) {
                if ( auto prev = threshold_method( ch ) ) { //threshold_methods_[ ch ] ) {
                    namespace ap = adportable;

                    if ( prev->enable != m.enable ||
                         (!ap::compare<double>::approximatelyEqual( prev->threshold_level, m.threshold_level )) ||
                         (!ap::compare<double>::approximatelyEqual( prev->response_time, m.response_time )) ||
                         prev->slope != m.slope ||
                         prev->use_filter != m.use_filter ) {
                        
                        // clear histogram except for time_resolution change, which is for histogram calculation resolution
                        histogram_->clear();
                    }

                    if ( m.use_filter ) {
                        if ( ( prev->filter != m.filter ) ||
                             ( ( m.filter == adcontrols::threshold_method::SG_Filter ) && 
                               ( !ap::compare<double>::approximatelyEqual( prev->sgwidth, m.sgwidth ) ) ) ||
                             ( ( m.filter == adcontrols::threshold_method::DFT_Filter ) && 
                               ( ( !ap::compare<double>::approximatelyEqual( prev->cutoffHz, m.cutoffHz ) ) ||
                                 ( m.complex_ != prev->complex_ ) ) ) ) {
                            // clear histogram except for time_resolution change, which is for histogram calculation resolution
                            histogram_->clear();                                 
                        }
                    }
                }
                std::lock_guard< std::mutex > lock( mutex_ );                
                threshold_methods_[ ch ] = std::make_shared< adcontrols::threshold_method >( m );
                return true;
            }
            return false;
        }

        inline std::shared_ptr< const adcontrols::threshold_method> threshold_method( int ch ) const {
            if ( ch < threshold_methods_.size() ) {
                std::lock_guard< std::mutex > lock( mutex_ );
                return threshold_methods_[ ch ];
            }
            return 0;
        }

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

        void find_threshold_timepoints( const ap240x::waveform& data, const adcontrols::threshold_method& method
                                        , std::vector< uint32_t >& elements, std::vector<double>& processed ) {

            const bool findUp = method.slope == adcontrols::threshold_method::CrossUp;
            const size_t nfilter = size_t( method.response_time / data.meta_.xIncrement );

            bool flag;

            if ( method.use_filter ) {

                processed.resize( data.size() );

                if ( data.meta_.dataType == 1 ) {
                    for ( size_t i = 0; i < data.size(); ++i )
                        processed[ i ] = data.toVolts( *(data.begin<int8_t>() + i) );
                } else if ( data.meta_.dataType == 4 ) {
                    for ( size_t i = 0; i < data.size(); ++i )
                        processed[ i ] = data.toVolts( *(data.begin<int32_t>() + i) );
                }

                if ( method.filter == adcontrols::threshold_method::SG_Filter ) {

                    adcontrols::waveform::sg::lowpass_filter( processed.size(), processed.data(), data.meta_.xIncrement, method.sgwidth );

                } else if ( method.filter == adcontrols::threshold_method::DFT_Filter ) {
                    if ( method.complex_ )
                        adcontrols::waveform::fft4c::lowpass_filter( processed.size(), processed.data(), data.meta_.xIncrement, method.cutoffHz );
                    else
                        adcontrols::waveform::fft4g::lowpass_filter( processed.size(), processed.data(), data.meta_.xIncrement, method.cutoffHz );
                }

                double level = method.threshold_level;
                auto it = processed.begin();
                while ( it != processed.end() ) {
                    if ( ( it = adportable::waveform_processor().find_threshold_element( it, processed.end(), level, flag ) ) != processed.end() ) {
                        if ( flag == findUp )
                            elements.push_back( std::distance( processed.begin(), it ) );
                        std::advance( it, nfilter );
                    }
                }
            } else {
            
                if ( data.meta_.dataType == 1 ) { // sizeof(int8_t)
                    double level = ( method.threshold_level + data.meta_.scaleOffset ) / data.meta_.scaleFactor;
                    
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
                    double level_per_trigger = ( method.threshold_level + data.meta_.scaleOffset ) / data.meta_.scaleFactor;
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
        }

        void handle_waveform( std::pair<std::shared_ptr< const ap240x::waveform >, std::shared_ptr< const ap240x::waveform > > pair ) {

            if ( !pair.first && !pair.second ) // empty
                return;

            threshold_result_pair_t results;
            std::array< std::shared_ptr< adcontrols::threshold_method >, 2 > methods;
            do {
                std::lock_guard< std::mutex > lock( mutex_ );
                methods[0] = threshold_methods_[ 0 ];
                methods[1] = threshold_methods_[ 1 ];                
            } while ( 0 );

            if ( pair.first ) {
                
                results.first = std::make_shared< ap240x::threshold_result >( pair.first );

                if ( methods[0]->enable ) {
                    
                    find_threshold_timepoints( *pair.first, *methods[0], results.first->indecies(), results.first->processed() );
                    histogram_->append( *results.first );

                }
            }
                
            if ( pair.second ) {

                results.second = std::make_shared< ap240x::threshold_result >( pair.second );
                
                if ( methods[1]->enable )
                    find_threshold_timepoints( *pair.second, *methods[1], results.second->indecies(), results.second->processed() );
            }
            
            do {
                std::lock_guard< std::mutex > lock( que2_mutex_ );
                que2_.push_back( results );
            } while( 0 );

            waveform_proc_count_++;
            sema_.signal();
        }
        
        void worker() {

            std::chrono::milliseconds cycleTime( 200 );

            while ( true ) {

                sema_.wait();
                if ( worker_stop_ )
                    return;

                auto tp = std::chrono::steady_clock::now();

                if ( std::chrono::duration_cast<std::chrono::milliseconds>( tp - time_handled_ ) > cycleTime ) {

                    if ( postCount_ == 0 ) {
                        if ( cycleTime < round_trip_ ) {
                            ADTRACE() << "Computer is too slow for update spectrum view: round-trip=" << round_trip_.count() << " ms";
                            cycleTime += std::chrono::milliseconds( 100 );
                        }
                        
                        time_handled_ = tp;
                        ++postCount_;
                        emit document::instance()->on_waveform_received();
                    }
                    
                    std::vector< threshold_result_pair_t > list;
                    do {
                        std::lock_guard< std::mutex > lock( que2_mutex_ );
                        if ( que2_.size() > 3 ) {
                            list.resize( que2_.size() - 2 );
                            std::copy( que2_.begin(), que2_.begin() + list.size(), list.begin() );
                            que2_.erase( que2_.begin(), que2_.begin() + list.size() );
                        }
                    } while( 0 );

                    if ( !list.empty() ) {
                        // std::cout << "list count: " << list.size() << " from: "
                        //           << list.front().first->data_->serialnumber_ << ", " << list.back().first->data_->serialnumber_ << std::endl;

                        std::ofstream of( time_datafile_, std::ios_base::out | std::ios_base::app );
                        if ( !of.fail() ) {
                            std::for_each( list.begin(), list.end(), [&of]( const threshold_result_pair_t& p ){
                                    if ( p.first )
                                        of << *p.first;
                                });
                        }
                    }
                }
            }
        }
    };

}

std::atomic< document * > document::impl::instance_( 0 );
std::mutex document::impl::mutex_;

document::document() : impl_( new impl() )
                     , digitizer_( new ap240::digitizer )
                     , device_status_( 0 )
                     , method_( std::make_shared< ap240x::method >() )
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
    document * tmp = impl::instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( impl::mutex_ );
        tmp = impl::instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new document();
            std::atomic_thread_fence( std::memory_order_release );
            impl::instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
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
    using adcontrols::ControlMethod::MethodItem;

    ap240x::method m;
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
document::waveform_handler( const ap240x::waveform * ch1, const ap240x::waveform * ch2, ap240x::method& )
{
    impl_->waveform_post_count()++;
    
    auto pair = std::make_pair( ( ch1 ? ch1->shared_from_this() : 0 ), ( ch2 ? ch2->shared_from_this() : 0 ) );

    impl_->io_service().post( [this,pair](){ impl_->handle_waveform( pair ); } );

    // Cache one second of data when 1kHz trigger rate
    std::lock_guard< std::mutex > lock( impl_->que_mutex_ );
    if ( impl_->que_.size() > 1500 ) {
        auto tail = impl_->que_.begin();
        std::advance( tail, 500 );
        impl_->que_.erase( impl_->que_.begin(), tail );
    }
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
document::toMassSpectrum( adcontrols::MassSpectrum& sp, const ap240x::waveform& waveform )
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
    data.ident = waveform.ident_;
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

    // std::wcout << L"########## document::appendOnFile(" << path << L", " << title << L") id=" << id << std::endl;
    // std::cout << "ms size: " << ms.size() << std::endl;
    
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
    ap240x::method m;
    if ( load( QString::fromStdWString( mfile.wstring() ), m ) )
        setControlMethod( m, QString() ); // don't save default name
    
    try {
        std::vector< adcontrols::threshold_method > x;
        std::wifstream inf( boost::filesystem::path( dir / "ap240_slope_time_method.xml" ).string() );
        boost::archive::xml_wiarchive ar( inf );
        ar >> boost::serialization::make_nvp( "threshold_methods", x );
        for ( size_t i = 0; i < x.size(); ++i )
            set_threshold_method( int( i ), x[ i ] );
    } catch( ... ) {
        set_threshold_method( 0, adcontrols::threshold_method() );
        set_threshold_method( 1, adcontrols::threshold_method() );
    }
}

void
document::finalClose()
{
    ADDEBUG() << "########### document::finalClose ##############";
    impl_->stop();
    
    boost::filesystem::path dir = user_preference::path( settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "ap240::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }
    ap240x::method m;
    MainWindow::instance()->getControlMethod( m );
    boost::filesystem::path fname( dir / "ap240.xml" );
    save( QString::fromStdWString( fname.wstring() ), m );

    std::vector< adcontrols::threshold_method > x{ *impl_->threshold_method(0), *impl_->threshold_method(1) };
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
document::load( const QString& filename, ap240x::method& m )
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
document::save( const QString& filename, const ap240x::method& m )
{
    std::wofstream outf( filename.toStdString() );

    boost::archive::xml_woarchive ar( outf );
    ar << boost::serialization::make_nvp( "ap240_method", m );
    return true;
}

std::shared_ptr< ap240x::method >
document::controlMethod() const
{
    std::lock_guard< std::mutex > lock( impl::mutex_ );
    return method_;
}

void
document::setControlMethod( const ap240x::method& m, const QString& filename )
{
    do {
        std::lock_guard< std::mutex > lock( impl::mutex_ );
        method_ = std::make_shared< ap240x::method >( m );
        digitizer_->peripheral_prepare_for_run( m );
    } while(0);

    if ( ! filename.isEmpty() ) {
        ctrlmethod_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, filename );
    }

    emit onControlMethodChanged( filename );
}

void
document::set_threshold_method( int ch, const adcontrols::threshold_method& m )
{
    if ( impl_->set_threshold_method( ch, m ) )
        emit on_threshold_method_changed( ch );
}

std::shared_ptr< const adcontrols::threshold_method >
document::threshold_method( int ch ) const
{
    return impl_->threshold_method( ch );
}


std::shared_ptr< adcontrols::MassSpectrum >
document::getHistogram( double resolution ) const
{
    ap240x::metadata meta;
    std::vector< std::pair< double, uint32_t > > hist;

    auto sp = std::make_shared< adcontrols::MassSpectrum >();    
    uint32_t serialnumber;
    uint64_t timeSinceEpoch;

    if ( size_t trigCount = impl_->getHistogram( hist, meta, serialnumber, timeSinceEpoch ) ) {
        
        using namespace adcontrols::metric;
        
        sp->setCentroid( adcontrols::CentroidNative );
        
        adcontrols::MSProperty prop = sp->getMSProperty();
        adcontrols::MSProperty::SamplingInfo info( 0
                                                   , uint32_t( meta.initialXOffset / meta.xIncrement + 0.5 )
                                                   , uint32_t( meta.actualPoints ) // this is for acq. time range calculation
                                                   , uint32_t( trigCount )
                                                   , 0 );
        info.fSampInterval( meta.xIncrement );
        prop.acceleratorVoltage( 3000 );
        prop.setSamplingInfo( info );
        
        prop.setTimeSinceInjection( meta.initialXTimeSeconds );
        prop.setTimeSinceEpoch( timeSinceEpoch );
        prop.setDataInterpreterClsid( "ap240" );
        
        {
            ap240::device_data data;
            data.meta = meta;
            std::string ar;
            adportable::binary::serialize<>()( data, ar );
            prop.setDeviceData( ar.data(), ar.size() );
        }
        
        sp->setMSProperty( prop );

        if ( resolution > meta.xIncrement ) {

            std::vector< double > times, intens;
            histogram::average( hist, resolution, times, intens );
            sp->resize( times.size() );
            sp->setTimeArray( times.data() );
            sp->setIntensityArray( intens.data() );
        } else {
            sp->resize( hist.size() );            
            for ( size_t idx = 0; idx < hist.size(); ++idx ) {
                sp->setTime( idx, hist[idx].first );
                sp->setIntensity( idx, hist[idx].second );
            }
        }
    }
    return sp;
}

void
document::save_histogram( size_t tickCount, const adcontrols::MassSpectrum& hist )
{
    std::ofstream of( impl_->hist_datafile(), std::ios_base::out | std::ios_base::app );

    const double * times = hist.getTimeArray();
    const double * intens = hist.getIntensityArray();
    const adcontrols::MSProperty& prop = hist.getMSProperty();

    of << boost::format("\n%d, %.8lf, %.14le") % tickCount % prop.timeSinceInjection() % prop.instTimeRange().first;
    for ( size_t i = 0; i < hist.size(); ++i )
        of << boost::format(", %.14le, %d" ) % times[ i ] % uint32_t( intens[ i ] );

}

void
document::waveform_drawn()
{
    impl_->waveform_drawn();
}

double
document::triggers_per_second() const
{
    return impl_->triggers_per_sec();
}

size_t
document::unprocessed_trigger_counts() const
{
    return impl_->unprocessed_trigger_counts();
}

adextension::iSequenceImpl *
document::iSequence()
{
    return impl_->iSequence();
}

ap240::iControllerImpl *
document::iController()
{
    return impl_->iController();
}

