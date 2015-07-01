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
#include <adportable/profile.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/serializer.hpp>
#include <qtwrapper/settings.hpp>
#include <app/app_version.h>
#include <coreplugin/documentmanager.h>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <chrono>
#include <string>

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

    namespace detail {
        struct remover {
            ~remover() {
                if ( document::instance_ ) {
                    std::lock_guard< std::mutex > lock( document::mutex_ );
                    if ( document::instance_ )
                        delete document::instance_;
                }
            };
            static remover _remover;
        };
    }

    class document::exec {
    public:
        std::chrono::system_clock::time_point tp_start_;
        uint64_t inject_time_point_;
        ap240::method ap240_;
        std::shared_ptr< adcontrols::ControlMethod > ctrlm_;
        adcontrols::ControlMethod::const_iterator nextIt_;

        exec() : tp_start_( std::chrono::system_clock::now() )
               , inject_time_point_(0) {
        }

        bool prepare_for_run( const adcontrols::ControlMethod& m ) {
            using adcontrols::controlmethod::MethodItem;
            ctrlm_ = std::make_shared< adcontrols::ControlMethod >( m );
            ctrlm_->sort();
            nextIt_ = std::find_if( ctrlm_->begin(), ctrlm_->end(), [] ( const MethodItem& mi ){
                    return mi.modelname() == "ap240";
                });
            if ( nextIt_ != ctrlm_->end() ) {
                adportable::serializer< ap240::method >::deserialize( ap240_, nextIt_->data(), nextIt_->size() );
                return true;
            }
            return false;
        }
    };

}
    
document::document() : digitizer_( new ap240::digitizer )
                     , exec_( new exec() )
                     , device_status_( 0 )
                     , cm_( std::make_shared< adcontrols::ControlMethod >() )
                     , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                 , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                 , QLatin1String( "ap240" ) ) )
{
}

document::~document()
{
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
    digitizer_->connect_waveform( boost::bind( &document::waveform_handler, this, _1, _2 ) );
    digitizer_->peripheral_initialize();
}

void
document::prepare_for_run()
{
    using adcontrols::controlmethod::MethodItem;

    MainWindow::instance()->getControlMethod( *cm_ );

    if ( exec_->prepare_for_run( *cm_ ) ) {
        digitizer_->peripheral_prepare_for_run( *exec_->ctrlm_ );
        // while .. if other item on initial condition exists.
    }
    else
        QMessageBox::information( 0, "ap240::document", QString( "Preparing for run withouth method " ) );
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
document::waveform_handler( const waveform * p, ap240::method& )
{
    auto ptr = p->shared_from_this();
    std::lock_guard< std::mutex > lock( mutex_ );
    while ( que_.size() >= 32 )
        que_.pop_front();
	que_.push_back( ptr );
    emit on_waveform_received();
    return false;
}

std::shared_ptr< const waveform >
document::findWaveform( uint32_t serialnumber )
{
    (void)serialnumber;
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( que_.empty() )
        return 0;
	std::shared_ptr< const waveform > ptr = que_.back();
    //ADTRACE() << "findWaveform: " << ptr->serialnumber_;
    //if ( serialnumber == (-1) )
    return ptr;
	/*
	auto it = std::find_if( que_.begin(), que_.end(), [=]( std::shared_ptr< const waveform >& p ){ return p->serialnumber_ == serialnumber; });
    if ( it != que_.end() )
        return *it;
    */
	return 0;
}

#if 0
const ap240::method&
document::method() const
{
    return *method_;
}
#endif

// static
bool
document::toMassSpectrum( adcontrols::MassSpectrum& sp, const waveform& waveform )
{
    using namespace adcontrols::metric;

    sp.setCentroid( adcontrols::CentroidNone );

    adcontrols::MSProperty prop = sp.getMSProperty();
    adcontrols::MSProperty::SamplingInfo info( 0
                                               , uint32_t( waveform.meta_.initialXOffset / waveform.meta_.xIncrement + 0.5 )
                                               , uint32_t( waveform.d_.size() )
                                               , waveform.method_.nbr_of_averages + 1
                                               , 0 );
    info.fSampInterval( 1.0 / waveform.method_.samp_rate );
    prop.acceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    
    prop.setTimeSinceInjection( waveform.meta_.initialXTimeSeconds );
    prop.setDataInterpreterClsid( "ap240" );

    ap240::device_data data;
    data.ident = *waveform.ident_;
    data.meta = waveform.meta_;
    std::string ar;
    adportable::binary::serialize<>()( data, ar );
    prop.setDeviceData( ar.data(), ar.size() );

    // prop.setDeviceData(); TBA
    sp.setMSProperty( prop );
    sp.resize( waveform.d_.size() );
	int idx = 0;
    for ( auto y: waveform.d_ )
        sp.setIntensity( idx++, y );
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

    boost::filesystem::path mfile( dir / "default.cmth" );
    adcontrols::ControlMethod cm;
    if ( load( QString::fromStdWString( mfile.wstring() ), cm ) ) {
        setControlMethod( cm, QString() ); // don't save default name
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
    MainWindow::instance()->getControlMethod( *cm_ );
    boost::filesystem::path fname( dir / "default.cmth" );
    save( QString::fromStdWString( fname.wstring() ), *cm_ );
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
document::load( const QString& filename, adcontrols::ControlMethod& m )
{
    QFileInfo fi( filename );

    if ( fi.exists() ) {
        adfs::filesystem fs;
        if ( fs.mount( filename.toStdWString().c_str() ) ) {
            adfs::folder folder = fs.findFolder( L"/ControlMethod" );
        
            auto files = folder.files();
            if ( !files.empty() ) {
                auto file = files.back();
                try {
                    file.fetch( m );
                }
                catch ( std::exception& ex ) {
                    QMessageBox::information( 0, "acquire -- Open default process method"
                                              , (boost::format( "Failed to open last used process method file: %1% by reason of %2% @ %3% #%4%" )
                                                 % filename.toStdString() % ex.what() % __FILE__ % __LINE__).str().c_str() );
                    return false;
                }
                return true;
            }
        }
    }
    return false;
}

bool
document::save( const QString& filename, const adcontrols::ControlMethod& m )
{
    adfs::filesystem file;

    if ( !file.create( filename.toStdWString().c_str() ) ) {
        ADTRACE() << "Error: \"" << filename.toStdString() << "\" can't be created";
        return false;
    }
    
    adfs::folder folder = file.addFolder( L"/ControlMethod" );
    adfs::file adfile = folder.addFile( filename.toStdWString(), filename.toStdWString() );
    try {
        adfile.dataClass( adcontrols::ControlMethod::dataClass() );
        adfile.save( m );
    } catch ( std::exception& ex ) {
        ADTRACE() << "Exception: " << boost::diagnostic_information( ex );
        return false;
    }
    adfile.commit();

#if 0 // adcontrols can't archive into xml format
    QFileInfo xmlfile( filename + ".xml" );
    if ( xmlfile.exists() )
        QFile::remove( xmlfile.absoluteFilePath() );

    std::wstringstream o;
    try {
        adcontrols::ControlMethod::xml_archive( o, m );
    } catch ( std::exception& ex ) {
        ADDEBUG() << boost::diagnostic_information( ex );
    }
    pugi::xml_document doc;
    doc.load( o );
    doc.save_file( xmlfile.absoluteFilePath().toStdString().c_str() );
#endif

    return true;
}

std::shared_ptr< adcontrols::ControlMethod >
document::controlMethod() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return cm_;
}

void
document::setControlMethod( const adcontrols::ControlMethod& m, const QString& filename )
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        cm_ = std::make_shared< adcontrols::ControlMethod >( m );
    } while(0);

    if ( ! filename.isEmpty() ) {
        ctrlmethod_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, filename );
    }

    emit onControlMethodChanged( filename );
}

