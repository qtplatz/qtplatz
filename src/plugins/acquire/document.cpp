/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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
#include "constants.hpp"
#include "fsm.hpp"
#include "mainwindow.hpp"
#include "mastercontroller.hpp"
#include "masterreceiver.hpp"
#include "masterobserver.hpp"
#include "task.hpp"
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/samplerun.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <adinterface/automaton.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/date_string.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <qtwrapper/settings.hpp>
#include <xmlparser/pugixml.hpp>
#include <app/app_version.h>
#include <coreplugin/documentmanager.h>
#include <QFileInfo>
#include <QSettings>
#include <QMessageBox>
#include <boost/date_time.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/msm/front/functor_row.hpp>
#include <boost/msm/front/euml/common.hpp>
#include <boost/msm/front/euml/operator.hpp>
#include <boost/msm/front/euml/state_grammar.hpp>
#include <atomic>
#include <future>


namespace acquire {
    
    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "acquire";
        }
    };

    class document::impl {
    public:
        impl() {
            connected_.clear();
        }

        ~impl() {
        }

        inline fsm::acquire& fsm() { return fsm_; }

        MasterController * masterController() {
            static std::once_flag flag;
            std::call_once( flag, [this] () { masterController_ = std::make_shared< MasterController >(); } );
            return masterController_.get();
        }

        MasterObserver * masterObserver() {
            static std::once_flag flag;
            std::call_once( flag, [this] () { masterObserver_ = std::make_shared< MasterObserver >(); } );
            return masterObserver_.get();
        }
        
        MasterReceiver * masterReceiver() {
            static std::once_flag flag;
            std::call_once( flag, [this] () { receiver_ = std::make_shared< MasterReceiver >( masterController() ); } );
            return receiver_.get();
        }
        
    public:
        std::atomic_flag connected_;
        std::vector< std::shared_ptr< adextension::iController > > activeControllers_;

    private:
        fsm::acquire fsm_;
        std::shared_ptr< MasterController > masterController_;
        std::shared_ptr< MasterObserver > masterObserver_;
        std::shared_ptr< MasterReceiver > receiver_;
    };

}

using namespace acquire;

std::atomic< document * > document::instance_(0);
std::mutex document::mutex_;

document::~document()
{
    delete impl_;
}

document::document(QObject *parent) : QObject(parent)
                                    , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                                , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                                , QLatin1String( "acquire" ) ) )
                                    , cm_( std::make_shared< adcontrols::ControlMethod::Method >() )
                                    , sampleRun_( std::make_shared< adcontrols::SampleRun >() )
                                    , impl_( new impl() )
{
}

void
document::actionConnect( bool applyMethod )
{
    // When press 'connect' button on Acquire's MainWindow, directory goes to MasterController::connect 
    // then call this from MainWindow.

    if ( impl_->connected_.test_and_set( std::memory_order_acquire ) == false ) {

        task::instance()->open();

        if ( *( impl_->fsm().current_state() ) == 0 ) {

            impl_->fsm().start();

            std::vector< std::shared_ptr< adextension::iController > > vec;
            MainWindow::instance()->findInstControllers( vec );

            for ( auto inst : vec ) 
                connect( inst.get(), &adextension::iController::message, MainWindow::instance(), &MainWindow::iControllerMessage );

            if ( applyMethod ) {
                // Do here when 'Correct' button on Acquire view pressed

                std::vector< std::future<bool> > futures;
                for ( auto& iController : vec ) {
                    // fire 'connect' trigger to all controllers
                    futures.push_back( std::async( [iController] () {
                        return iController->connect() && iController->wait_for_connection_ready(); } ) );
                }
                
                size_t i = 0;
                for ( auto& future : futures ) {
                    if ( future.get() )
                        impl_->activeControllers_.push_back( vec[ i ] );
                    ++i;
                }

                futures.clear();
                auto cm = MainWindow::instance()->getControlMethod();
                for ( auto iController : impl_->activeControllers_ ) {
                    if ( auto session = iController->getInstrumentSession() )
                        futures.push_back( std::async( std::launch::async, [session, cm] () {
                                    return session->initialize() && session->prepare_for_run( cm ); } ) );
                }
                task::instance()->post( futures );
            }

            // connect all data streams
            if ( auto masterObserver = impl_->masterObserver() ) {

                auto ptr( masterObserver->shared_from_this() );

                for ( auto& iController : vec ) {

                    if ( auto session = iController->getInstrumentSession() ) {

                        // connect to each controller session for instrument state change
                        session->connect( impl_->masterReceiver(), "acquire.master" );

                        if ( auto observer = session->getObserver() ) {
                            ADDEBUG() << iController->module_name().toStdString() << "  --> Added to MasterObserver";
                            ptr->addSibling( observer );
                        }
                    }
                }
            }

        }
    }
}

void
document::addToRecentFiles( const QString& filename )
{
    qtwrapper::settings(*settings_).addRecentFiles( constants::GRP_DATA_FILES, constants::KEY_FILES, filename );
}

document * 
document::instance()
{
    typedef document T;

    T * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new T();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

void
document::initialSetup()
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );

    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "dataproc::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    QString path = recentFile( constants::GRP_DATA_FILES, false );
    if ( path.isEmpty() ) {
        path = QString::fromStdWString( ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data" ).generic_wstring() );
    } else {
        path = QFileInfo( path ).path();
    }

    Core::DocumentManager::setUseProjectsDirectory( true );

    boost::filesystem::path mfile( dir / "default.cmth" );
    adcontrols::ControlMethod::Method cm;
    if ( load( QString::fromStdWString( mfile.wstring() ), cm ) )
        setControlMethod( cm, QString() ); // don't save default name

    boost::filesystem::path sfile( dir / "samplerun.sequ" );
    adcontrols::SampleRun sr;
    if ( load( QString::fromStdWString( sfile.wstring() ), sr ) ) {
        //boost::filesystem::path path( sr.dataDirectory() );
        boost::filesystem::path path( adportable::profile::user_data_dir< char >() );
        path /= "data";
        path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
        sr.dataDirectory( path.normalize().wstring().c_str() );

        setSampleRun( sr, QString() ); // don't save default name
    }
}

void
document::finalClose( MainWindow * mainwindow )
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "dataproc::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }
    mainwindow->getControlMethod( *cm_ );
    boost::filesystem::path fname( dir / "default.cmth" );
    save( QString::fromStdWString( fname.wstring() ), *cm_ );

    mainwindow->getSampleRun( *sampleRun_ );
    boost::filesystem::path sname( dir / "samplerun.sequ" );
    save( QString::fromStdWString( sname.wstring() ), *sampleRun_ );
}

std::shared_ptr< adcontrols::ControlMethod::Method >
document::controlMethod() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return cm_;
}

std::shared_ptr< adcontrols::SampleRun >
document::sampleRun() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return sampleRun_;
}

void
document::setControlMethod( const adcontrols::ControlMethod::Method& m, const QString& filename )
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        cm_ = std::make_shared< adcontrols::ControlMethod::Method >( m );
        for ( auto& item : m ) {
            ADDEBUG() << item.modelname() << ", " << item.itemLabel() << " initial: " << item.isInitialCondition() << " time: " << item.time();
        }
    } while(0);

    if ( ! filename.isEmpty() ) {
        ctrlmethod_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( constants::GRP_METHOD_FILES, constants::KEY_FILES, filename );
    }

    emit onControlMethodChanged( filename );
}

void
document::setSampleRun( const adcontrols::SampleRun& m, const QString& filename )
{
    sampleRun_ = std::make_shared< adcontrols::SampleRun >( m );

    if ( ! filename.isEmpty() ) {
        samplerun_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( constants::GRP_SEQUENCE_FILES, constants::KEY_FILES, filename );
    }
    emit onSampleRunChanged( QString::fromWCharArray( sampleRun_->filePrefix() ), QString::fromWCharArray( sampleRun_->dataDirectory() ) );
}

QString
document::recentFile( const char * group, bool dir_on_fail )
{
    if ( group == 0 )
        group = constants::GRP_DATA_FILES;

    QString file = qtwrapper::settings( *settings_ ).recentFile( group, constants::KEY_FILES );
    if ( !file.isEmpty() )
        return file;

    if ( dir_on_fail ) {
        file = Core::DocumentManager::currentFile();
        if ( file.isEmpty() )
            file = qtwrapper::settings( *settings_ ).recentFile( constants::GRP_DATA_FILES, constants::KEY_FILES );
        
        if ( !file.isEmpty() ) {
            QFileInfo fi( file );
            return fi.path();
        }
        return QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() );
    }
    return QString();
}

bool
document::load( const QString& filename, adcontrols::ControlMethod::Method& m )
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
                    QMessageBox::information( 0, "acquire -- Open default control method"
                                              , (boost::format( "Failed to open file: %1% by reason of %2% @ %3% #%4%" )
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
document::save( const QString& filename, const adcontrols::ControlMethod::Method& m )
{
    adfs::filesystem file;

    if ( !file.create( filename.toStdWString().c_str() ) ) {
        ADTRACE() << "Error: \"" << filename.toStdString() << "\" can't be created";
        return false;
    }
    
    adfs::folder folder = file.addFolder( L"/ControlMethod" );
    adfs::file adfile = folder.addFile( filename.toStdWString(), filename.toStdWString() );
    try {
        adfile.dataClass( adcontrols::ControlMethod::Method::dataClass() );
        adfile.save( m );
    } catch ( std::exception& ex ) {
        ADTRACE() << "Exception: " << boost::diagnostic_information( ex );
        return false;
    }
    adfile.commit();

    return true;
}

bool
document::load( const QString& filename, adcontrols::SampleRun& m )
{
    QFileInfo fi( filename );

    if ( fi.exists() ) {
        adfs::filesystem fs;
        if ( fs.mount( filename.toStdWString().c_str() ) ) {
            adfs::folder folder = fs.findFolder( L"/SampleRun" );
        
            auto files = folder.files();
            if ( !files.empty() ) {
                auto file = files.back();
                try {
                    file.fetch( m );
                }
                catch ( std::exception& ex ) {
                    QMessageBox::information( 0, "acquire -- Open default sample run"
                                              , (boost::format( "Failed to open last used sample run file: %1% by reason of %2% @ %3% #%4%" )
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
document::save( const QString& filename, const adcontrols::SampleRun& m )
{
    adfs::filesystem file;

    if ( !file.create( filename.toStdWString().c_str() ) ) {
        ADTRACE() << "Error: \"" << filename.toStdString() << "\" can't be created";
        return false;
    }
    
    adfs::folder folder = file.addFolder( L"/SampleRun" );
    adfs::file adfile = folder.addFile( filename.toStdWString(), filename.toStdWString() );
    try {
        adfile.dataClass( adcontrols::ControlMethod::Method::dataClass() );
        adfile.save( m );
    } catch ( std::exception& ex ) {
        ADTRACE() << "Exception: " << boost::diagnostic_information( ex );
        return false;
    }
    adfile.commit();

    do {
        boost::filesystem::path xmlfile( filename.toStdWString() );
        xmlfile.replace_extension( ".xml" );
        std::wostringstream os;
        adcontrols::SampleRun::xml_archive( os, m );
        pugi::xml_document dom;
        dom.load( pugi::as_utf8( os.str() ).c_str() );
        dom.save_file( xmlfile.string().c_str() );
    } while(0);

    return true;
}

void
document::fsmStop()
{
    impl_->fsm().start();
}

void
document::fsmActPrepareForRun()
{
    adcontrols::SampleRun run;
    auto mainWindow = MainWindow::instance();

    if ( mainWindow && mainWindow->getSampleRun( run ) )
        setSampleRun( run, QString() );

    impl_->fsm().process_event( fsm::prepare() );
}

void
document::fsmActRun()
{
    impl_->fsm().process_event( fsm::start_run() );
}

void
document::fsmActInject()
{
    impl_->fsm().process_event( fsm::inject() );
}

void
document::fsmActStop()
{
    impl_->fsm().process_event( fsm::stop() );
}

// See also AcquirePlugin::handle_controller_message
void
document::notify_ready_for_run( const char * xml )
{
    if ( xml ) {
        std::wstring wxml( adportable::utf::to_wstring( xml ) );
        std::wistringstream is( wxml );

        adcontrols::SampleRun run;
        adcontrols::SampleRun::xml_restore( is, run );

        emit onSampleRunChanged( QString::fromWCharArray( run.filePrefix() ), QString::fromWCharArray( run.dataDirectory() ) );
        double length = run.methodTime();
        emit onSampleRunLength( QString( "%1 min" ).arg( QString::number( length / 60, 'f', 1 ) ) );
    }
}

MasterController *
document::masterController()
{
    return impl_->masterController();
}

MasterObserver *
document::masterObserver()
{
    return impl_->masterObserver();
}
