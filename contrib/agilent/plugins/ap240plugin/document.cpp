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
#include <adinterface/controlserver.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adportable/asio/thread.hpp>
#include <adportable/profile.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/serializer.hpp>
#include <adportable/semaphore.hpp>
#include <qtwrapper/settings.hpp>
#include <app/app_version.h>
#include <coreplugin/documentmanager.h>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
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

    class document::impl {
    public:
        impl() : worker_stop_( false ) {
        }
        
        ~impl() {
            stop();
        }
        
        adportable::semaphore sema_;
        bool worker_stop_;
        std::chrono::steady_clock::time_point time_handled_;
        std::vector< std::thread > threads_;
        std::deque< std::pair<
                        std::shared_ptr< const waveform >
                        , std::shared_ptr< const waveform >
                        > > que_;

        std::array< ap240::threshold_method, 2 > thresholds_;

        void run() {
            if ( threads_.empty() )
                threads_.push_back( adportable::asio::thread( [this] { worker(); } ) );
        }
        
        void stop() {
            worker_stop_ = true;
            sema_.signal();
            for ( auto& t: threads_ )
                t.join();
        }
        
        void worker() {
            while( true ) {

                sema_.wait();

                if ( worker_stop_ )
                    return;

                if ( sema_.count() )
                    std::cout << "sema count: " << sema_.count() << std::endl;

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

    std::lock_guard< std::mutex > lock( mutex_ );

    while ( impl_->que_.size() > 512 )
        impl_->que_.pop_front();

    impl_->que_.push_back( pair );
    impl_->sema_.signal();

    return false; // no protocol-acquisition handled.
}

document::waveforms_t
document::findWaveform( uint32_t serialnumber )
{
    (void)serialnumber;

    std::lock_guard< std::mutex > lock( mutex_ );
    if ( impl_->que_.empty() )
        return waveforms_t( 0, 0 );

	return impl_->que_.back();
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
    boost::filesystem::path fname( dir / "ap240.xmth" );
    save( QString::fromStdWString( fname.wstring() ), m );
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
    } catch( ... ) {
        std::cout << "ap240::method load failed" << std::endl;
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
    std::cout << "set_threshold_method(" << ch << ", " << m.threshold << std::endl;
    if ( ch < impl_->thresholds_.size() )
        impl_->thresholds_[ ch ] = m;
}

const ap240::threshold_method&
document::threshold_method( int ch ) const
{
    if ( ch < impl_->thresholds_.size() )
        return impl_->thresholds_[ ch ];
    throw std::runtime_error(0);
}


