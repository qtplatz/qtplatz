/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "task.hpp"
#include "tdcdoc.hpp"
#include "u5303a_constants.hpp"
#include "icontrollerimpl.hpp"
#include <u5303a/digitizer.hpp>
#include <acqrscontrols/u5303a/method.hpp>
#include <acqrscontrols/u5303a/metadata.hpp>
#include <adlog/logger.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/samplerun.hpp>
#include <adinterface/controlserver.hpp>
#include <adextension/icontrollerimpl.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adportable/profile.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/serializer.hpp>
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
#include <future>
#include <string>

using namespace u5303a;

document * document::instance_ = 0;
std::mutex document::mutex_;

namespace u5303a {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "u5303a";
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

    namespace so = adicontroller::SignalObserver;

    class MasterObserver : public adicontroller::SignalObserver::Observer {
        const boost::uuids::uuid objid_;
    public:
        MasterObserver() : objid_( { 0 } ) {}
        bool connect( so::ObserverEvents * cb, so::eUpdateFrequency, const std::string& ) override { return false; }
        bool disconnect( so::ObserverEvents * cb ) override { return false; }
        const boost::uuids::uuid& objid() const override { return objid_; }
        const char * objtext() const override { return 0; }
        uint64_t uptime() const override { return 0; }
        std::shared_ptr< so::DataReadBuffer > readData( uint32_t pos ) override { return 0; }
        const char * dataInterpreterClsid() const override { return 0; }
    };

    class document::exec {
    public:
        std::chrono::system_clock::time_point tp_start_;
        uint64_t inject_time_point_;
        acqrscontrols::u5303a::method u5303a_;
        std::shared_ptr< adcontrols::ControlMethod::Method > ctrlm_;
        adcontrols::ControlMethod::Method::const_iterator nextIt_;

        exec() : tp_start_( std::chrono::system_clock::now() )
               , inject_time_point_(0) {
        }

        bool prepare_for_run( const adcontrols::ControlMethod::Method& m ) {
            using adcontrols::ControlMethod::MethodItem;
            ctrlm_ = std::make_shared< adcontrols::ControlMethod::Method >( m );
            ctrlm_->sort();
            nextIt_ = std::find_if( ctrlm_->begin(), ctrlm_->end(), [] ( const MethodItem& mi ){
                    return mi.modelname() == "u5303a";
                });
            if ( nextIt_ != ctrlm_->end() ) {
                adportable::serializer< acqrscontrols::u5303a::method >::deserialize( u5303a_, nextIt_->data(), nextIt_->size() );
                return true;
            }
            return false;
        }
    };

    class document::impl {
    public:
        std::shared_ptr< tdcdoc > tdcdoc_;
        std::shared_ptr< adcontrols::SampleRun > nextSampleRun_;
        std::shared_ptr< ::u5303a::iControllerImpl > iControllerImpl_;
        std::vector< std::shared_ptr< adextension::iController > > iControllers_;
        std::shared_ptr< MasterObserver > masterObserver_;
        bool isRecording_;
        std::mutex mutex_;

        impl() : tdcdoc_( std::make_shared< tdcdoc >() )
               , nextSampleRun_( std::make_shared< adcontrols::SampleRun >() )
               , iControllerImpl_( std::make_shared< u5303a::iControllerImpl >() )
               , isRecording_( false ) {
        }

        void addiController( std::shared_ptr< adextension::iController > p );
        void handleConnected( adextension::iController * controller );
        void handleMessage( adextension::iController * ic, unsigned long code, unsigned long value );
        void handleLog( adextension::iController *, const QString& );
        void handleDataEvent( adicontroller::SignalObserver::Observer *, unsigned int events, unsigned int pos );

    };

}
    
document::document() : digitizer_( new u5303a::digitizer )
                     , exec_( new exec() )
                     , device_status_( 0 )
                     , cm_( std::make_shared< adcontrols::ControlMethod::Method >() )
                     , method_( std::make_shared< acqrscontrols::u5303a::method >() )
                     , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                                 , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                 , QLatin1String( "u5303a" ) ) )
                     , impl_( new impl() )
{
}

document::~document()
{
    delete digitizer_;
    delete impl_;
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
document::actionConnect()
{
    if ( !impl_->iControllers_.empty() ) {

        std::vector< std::future<bool> > futures;
        for ( auto& iController : impl_->iControllers_ ) {
            futures.push_back( std::async( [iController] () {
                        return iController->connect() && iController->wait_for_connection_ready(); } ) );
        }

        std::vector< std::shared_ptr< adextension::iController > > activeControllers;

        size_t i = 0;
        for ( auto& future : futures ) {
            if ( future.get() )
                activeControllers.push_back( impl_->iControllers_[ i ] );
            ++i;
        }
        impl_->iControllers_ = activeControllers;

        auto cm = MainWindow::instance()->getControlMethod();
        
        futures.clear();
        for ( auto& iController : impl_->iControllers_ ) {
            if ( auto session = iController->getInstrumentSession() )
                futures.push_back( std::async( std::launch::async, [session,cm](){
                            return session->initialize() && session->prepare_for_run( cm );  } ) );
        }

        task::instance()->post( futures );

        // setup observer hiralchey
        impl_->masterObserver_ = std::make_shared< MasterObserver >();
        for ( auto& iController : impl_->iControllers_ ) {
            if ( auto session = iController->getInstrumentSession() ) {
                if ( auto observer = session->getObserver() )
                    impl_->masterObserver_->addSibling( observer );
            }
        }
        
        task::instance()->prepare_next_sample( impl_->masterObserver_.get(), impl_->nextSampleRun_, *cm );

    }
}

void
document::u5303a_connect()
{
    digitizer_->connect_reply( boost::bind( &document::reply_handler, this, _1, _2 ) );
    digitizer_->connect_waveform( boost::bind( &document::waveform_handler, this, _1, _2, _3 ) );
    digitizer_->peripheral_initialize();
}

void
document::prepare_for_run()
{
    using adcontrols::ControlMethod::MethodItem;

    cm_ = MainWindow::instance()->getControlMethod();

    if ( exec_->prepare_for_run( *cm_ ) ) {
        digitizer_->peripheral_prepare_for_run( *exec_->ctrlm_ );
        // while .. if other item on initial condition exists.
    }
    else
        QMessageBox::information( 0, "u5303a::document", QString( "Preparing for run withouth method " ) );
}

void
document::u5303a_start_run()
{
	digitizer_->peripheral_run();
}

void
document::u5303a_stop()
{
	digitizer_->peripheral_stop();
}

void
document::u5303a_trigger_inject()
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
document::waveform_handler( const acqrscontrols::u5303a::waveform * ch1, const acqrscontrols::u5303a::waveform * ch2, acqrscontrols::u5303a::method& )
{
    auto ptr = ch1->shared_from_this();
    std::lock_guard< std::mutex > lock( mutex_ );
    while ( que_.size() >= 32 )
        que_.pop_front();
	que_.push_back( ptr );
    emit on_waveform_received();
    return false;
}

std::shared_ptr< const acqrscontrols::u5303a::waveform >
document::findWaveform( uint32_t serialnumber )
{
    (void)serialnumber;
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( que_.empty() )
        return 0;
	std::shared_ptr< const acqrscontrols::u5303a::waveform > ptr = que_.back();
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

std::shared_ptr< const acqrscontrols::u5303a::method >
document::method() const
{
    return method_;
}

std::shared_ptr< adcontrols::MassSpectrum >
document::getHistogram( double resolution ) const
{
    acqrscontrols::u5303a::metadata meta;
    std::vector< std::pair< double, uint32_t > > hist;

    auto sp = std::make_shared< adcontrols::MassSpectrum >();    
    std::pair<uint32_t,uint32_t> serialnumber;
    std::pair<uint64_t,uint64_t> timeSinceEpoch;
#if 0
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
        prop.setTimeSinceEpoch( timeSinceEpoch.second );
        prop.setDataInterpreterClsid( "u5303a" );
        
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
            ap240x::histogram::average( hist, resolution, times, intens );
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
#endif
    return sp;
}

u5303a::iControllerImpl *
document::iController()
{
    return impl_->iControllerImpl_.get();
}


// static
bool
document::toMassSpectrum( adcontrols::MassSpectrum& sp, const acqrscontrols::u5303a::waveform& waveform )
{
    using namespace adcontrols::metric;

    sp.setCentroid( adcontrols::CentroidNone );

    adcontrols::MSProperty prop = sp.getMSProperty();
    adcontrols::MSProperty::SamplingInfo info( 0
                                               , uint32_t( waveform.meta_.initialXOffset / waveform.meta_.xIncrement + 0.5 )
                                               , uint32_t( waveform.data_size() )
                                               , waveform.method_.method_.nbr_of_averages + 1
                                               , 0 );
    info.fSampInterval( 1.0 / waveform.method_.method_.samp_rate );
    prop.acceleratorVoltage( 3000 );
    prop.setSamplingInfo( info );
    
    prop.setTimeSinceInjection( waveform.meta_.initialXTimeSeconds );
    prop.setTimeSinceEpoch( waveform.timeSinceEpoch_ );
    prop.setDataInterpreterClsid( "u5303a" );

    u5303a::device_data data;
    data.ident = *waveform.ident();
    data.meta = waveform.meta_;
    std::string ar;
    adportable::binary::serialize<>()( data, ar );
    prop.setDeviceData( ar.data(), ar.size() );

    // prop.setDeviceData(); TBA
    sp.setMSProperty( prop );
    sp.resize( waveform.data_size() );

    auto dp = waveform.data();
    for ( size_t idx = 0; idx < waveform.data_size(); ++idx )
        sp.setIntensity( idx, *dp++ );

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
            QMessageBox::information( 0, "u5303a::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    QString path = recentFile( Constants::GRP_DATA_FILES, false );
    if ( path.isEmpty() ) {
        path = QString::fromStdWString( ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data" ).generic_wstring() );
    } else {
        path = QFileInfo( path ).path();
    }

    do {
        boost::filesystem::path mfile( dir / "u5303a.cmth.xml" );
        acqrscontrols::u5303a::method m;
        if ( load( QString::fromStdWString( mfile.wstring() ), m ) )
            *method_ = m;

        if ( cm_ ) {
            cm_->append<>( m );
        }

    } while ( 0 );
}

void
document::finalClose()
{
    boost::filesystem::path dir = user_preference::path( settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "u5303a::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }

    cm_ = MainWindow::instance()->getControlMethod();
    auto it = cm_->find( cm_->begin(), cm_->end(), acqrscontrols::u5303a::method::modelClass() );
    if ( it != cm_->end() ) {
        acqrscontrols::u5303a::method x;
        if ( it->get<>( *it, x ) ) {
            boost::filesystem::path fname( dir / "u5303a.cmth.xml" );
            save( QString::fromStdWString( fname.wstring() ), x );
        }
    }
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
document::load( const QString& filename, acqrscontrols::u5303a::method& m )
{
    try {

        std::wifstream inf( filename.toStdString() );
        boost::archive::xml_wiarchive ar( inf );
        
        ar >> boost::serialization::make_nvp( "u5303a_method", m );

        return true;

    } catch( ... ) {
        std::cout << "############# acqrscontrols::u5303a::method load failed" << std::endl;
    }
    return false;
}

bool
document::save( const QString& filename, const acqrscontrols::u5303a::method& m )
{
    std::wofstream outf( filename.toStdString() );

    boost::archive::xml_woarchive ar( outf );
    ar << boost::serialization::make_nvp( "u5303a_method", m );
    return true;
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

std::shared_ptr< adcontrols::ControlMethod::Method >
document::controlMethod() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return cm_;
}

void
document::setControlMethod( const adcontrols::ControlMethod::Method& m, const QString& filename )
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        cm_ = std::make_shared< adcontrols::ControlMethod::Method >( m );
        auto it = cm_->find( cm_->begin(), cm_->end(), acqrscontrols::u5303a::method::modelClass() );
        if ( it != cm_->end() ) {
            acqrscontrols::u5303a::method m;
            if ( it->get( *it, m ) )
                * method_ = m;
        }
    } while(0);

    if ( ! filename.isEmpty() ) {
        ctrlmethod_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, filename );
    }

    emit onControlMethodChanged( filename );
}

double
document::triggers_per_second() const
{
    return 0; //impl_->triggers_per_sec();
}

size_t
document::unprocessed_trigger_counts() const
{
    return 0; //impl_->unprocessed_trigger_counts();
}

void
document::save_histogram( size_t tickCount, const adcontrols::MassSpectrum& hist )
{
#if 0
    std::ofstream of( impl_->hist_datafile(), std::ios_base::out | std::ios_base::app );

    const double * times = hist.getTimeArray();
    const double * intens = hist.getIntensityArray();
    const adcontrols::MSProperty& prop = hist.getMSProperty();

    of << boost::format("\n%d, %.8lf, %.14le") % tickCount % prop.timeSinceInjection() % prop.instTimeRange().first;
    for ( size_t i = 0; i < hist.size(); ++i )
        of << boost::format(", %.14le, %d" ) % times[ i ] % uint32_t( intens[ i ] );
#endif
}

void
document::set_threshold_method( int ch, const adcontrols::threshold_method& m )
{
    if ( ch == 0 ) {
        auto ptr = std::make_shared< acqrscontrols::u5303a::method >( *method_ );
        ptr->threshold_ = m;
        method_ = ptr;
        emit on_threshold_method_changed( ch );
    }
}

void
document::set_method( const acqrscontrols::u5303a::method& m )
{
    auto ptr = std::make_shared< acqrscontrols::u5303a::method >( m );
    ptr->threshold_ = method_->threshold_;

    method_ = ptr;
}

const adcontrols::SampleRun *
document::sampleRun() const
{
    return impl_->nextSampleRun_.get();
}

void
document::setSampleRun( std::shared_ptr< adcontrols::SampleRun > sr )
{
    impl_->nextSampleRun_ = sr;
}

bool
document::isRecording() const
{
    return impl_->isRecording_;
}

void
document::actionRec( bool onoff )
{
    if ( impl_->isRecording_ != onoff ) {
        impl_->isRecording_ = onoff;
        //emit recStateChanged( onoff );
    }
}

void
document::actionSyncTrig()
{
    //task::instance()->clear_histogram();
}

void
document::actionRun( bool onoff )
{
}

void
document::addiController( adextension::iController * p )
{
    try {

        if ( auto ptr = p->pThis() )
            impl_->addiController( p->pThis() );

    } catch ( std::bad_weak_ptr& ) {
        
        QMessageBox::warning( MainWindow::instance(), "MALPIX Acquire"
                              , QString( tr( "Instrument controller %1 is not compatible; ignored." ) ).arg( p->module_name() ) );

    }
}

void
document::impl::addiController( std::shared_ptr< adextension::iController > p )
{

    using adextension::iController;
    using adicontroller::SignalObserver::Observer;

    iControllers_.push_back( p );

    // switch to UI thread
    connect( p.get(), &iController::connected, [this] ( iController * p ) { handleConnected( p ); } );
    connect( p.get(), &iController::message, [this] ( iController * p, unsigned int code, unsigned int value ) { handleMessage( p, code, value ); } );
    connect( p.get(), &iController::log, [this] ( iController * p, const QString& log ) { handleLog( p, log ); } );

    // non UI thread
    //p->dataChangedHandler( [] ( Observer *o, unsigned int pos ) { task::instance()->onDataChanged( o, pos ); } );

}

void
document::impl::handleConnected( adextension::iController * controller )
{
    task::instance()->initialize();
}

void
document::impl::handleMessage( adextension::iController * ic, unsigned long code, unsigned long value )
{
#if 0
    if ( code == adicontroller::Receiver::CLIENT_ATTACHED ) {

        emit document::instance()->instStateChanged( int( adicontroller::Instrument::eStandBy ) ); // --> enable FSM UI

    } else if ( code == adicontroller::Receiver::STATE_CHANGED ) {

        emit document::instance()->instStateChanged( int( value ) );

        if ( value == adi::Instrument::eStandBy && ctrlMethod_ && ic ) {
            if ( auto session = ic->getInstrumentSession() )
                session->prepare_for_run( ctrlMethod_ );
        }
    }
#endif
}

void
document::impl::handleLog( adextension::iController *, const QString& )
{
}

void
document::impl::handleDataEvent( adicontroller::SignalObserver::Observer *, unsigned int events, unsigned int pos )
{
}

tdcdoc *
document::tdc()
{
    return impl_->tdcdoc_.get();
}

void
document::setData( const boost::uuids::uuid& objid, std::shared_ptr< adcontrols::MassSpectrum > ms, unsigned idx )
{
    assert( idx < acqrscontrols::u5303a::nchannels );
#if 0        
    do {
        std::lock_guard< std::mutex > lock( impl_->mutex_ );    
        impl_->spectra_[ objid ][ idx ] = ms;
    } while( 0 );

    emit dataChanged2( objid, idx );

    if ( objid == ap240_observer ) {
        double resolution = 0;
        if ( auto tm = tdc()->threshold_method( idx ) )
            resolution = tm->time_resolution;

        size_t trigCount;
        std::pair< uint64_t, uint64_t > timeSinceEpoch;

        auto histogram = tdc()->getHistogram( resolution, idx, trigCount, timeSinceEpoch );
        tdc()->update_rate( trigCount, timeSinceEpoch );
        do {
            std::lock_guard< std::mutex > lock( impl_->mutex_ );
            impl_->spectra_[ histogram_observer ][ idx ] = histogram;
        } while ( 0 );

        emit dataChanged2( histogram_observer, idx );
    }
#endif
}
