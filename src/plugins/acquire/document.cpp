/**************************************************************************
** Copyright (C) 2010-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "constants.hpp"
#include "mainwindow.hpp"
#include "task.hpp"
#include "idgmodimpl.hpp"
#include "resultwriter.hpp"
#include <date/date.h>
#include <adacquire/masterobserver.hpp>
#include <adacquire/sampleprocessor.hpp>
#include <adacquire/simpleobserver.hpp>
#include <adacquire/task.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/controlmethod/tofchromatogrammethod.hpp>
#include <adcontrols/controlmethod/tofchromatogramsmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/samplerun.hpp>
#include <adcontrols/timedigitalhistogram.hpp>
#include <adcontrols/timedigitalmethod.hpp>
#include <adcontrols/trace.hpp>
#include <admethods/controlmethod/adtracemethod.hpp>
#include <adextension/icontrollerimpl.hpp>
#include <adextension/isequenceimpl.hpp>
#include <adextension/isnapshothandler.hpp>
#include <adfs/adfs.hpp>
#include <adfs/cpio.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adlog/logging_handler.hpp>
#include <admethods/controlmethod/adtracemethod.hpp>
#include <adplugins/adspectrometer/massspectrometer.hpp>
#include <adportable/binary_serializer.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/debug_core.hpp>
#include <adportable/profile.hpp>
#include <adportable/serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adurl/ajax.hpp>
#include <adurl/blob.hpp>
#include <adurl/sse.hpp>
#include <adutils/inifile.hpp>
#include <adwidgets/findslopeform.hpp>
#include <adwidgets/thresholdactionform.hpp>
#include <app/app_version.h>
#include <coreplugin/documentmanager.h>
#include <date/date.h>
#include <extensionsystem/pluginmanager.h>
#include <qtwrapper/settings.hpp>
#include <socfpga/constants.hpp>
#include <socfpga/session.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/at.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <QSettings>
#include <QFileInfo>
#include <QMessageBox>
#include <QMetaType>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QVector>
#include <algorithm>
#include <bitset>
#include <chrono>
#include <future>
#include <limits>
#include <string>
#include <fstream>

Q_DECLARE_METATYPE( boost::uuids::uuid );

using namespace acquire;

namespace acquire {

    struct user_preference {
        static boost::filesystem::path path( QSettings * settings ) {
            boost::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "acquire";
        }
    };

    template< typename T > struct xmlWriter {
        void operator()( const adcontrols::ControlMethod::MethodItem& mi, const boost::filesystem::path& dir ) const {
            T x;
            if ( mi.get<>( mi, x ) ) {
                boost::filesystem::path fname( dir / mi.modelname() );
                fname.replace_extension( ".cmth.xml" );
                std::wofstream outf( fname.string() );
                T::xml_archive( outf, x );
            }
        }
    };

    namespace so = adacquire::SignalObserver;

    struct ObserverData {
        const char * objtext;
        const boost::uuids::uuid objid;
        const char * dataInterpreterClsid;
        const so::Description desc;
    };

    static ObserverData observers [] = {
        // { acquire::waveform_observer_name      // "1.acquire.ms-cheminfo.com"
        //   , acquire::waveform_observer         // "ab4620f4-933f-4b44-9102-740caf8f791a"
        //   , acquire::waveform_datainterpreter  // "a33d0d5e-5ace-4d2c-9d46-ddffcd799b51"
        //   , { acquire::waveform_observer_name  // desc.traceId := data name on adfs file
        //       , so::eTRACE_SPECTRA
        //       , so::eMassSpectrometer
        //       , L"Time", L"Count", 3, 0
        //     }
        // }
        // , { acquire::histogram_observer_name     // "histogram.tdc.1.acquire.ms-cheminfo.com"
        //     , acquire::histogram_observer        // "b3600237-527b-4689-8b25-4ca1c30b99dd"
        //     , acquire::histogram_datainterpreter // "58fa8716-28fb-484f-bb89-49e843f70981"
        //     , { acquire::histogram_observer_name // desc.traceId := data name on adfs file
        //         , so::eTRACE_SPECTRA
        //         , so::eMassSpectrometer
        //         , L"Time", L"Count", 3, 0
        //     }
        // }
    };

    struct exec_fsm_stop {
        void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
            task::instance()->sample_stopped();
            for ( auto& iController : iControllers ) {
                if ( auto session = iController->getInstrumentSession() )
                    session->stop_run();
            }
        }
    };

    struct exec_fsm_start {
        void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
            for ( auto inst : iControllers ) {
                if ( auto session = inst->getInstrumentSession() )
                    session->start_run();
            }

            document::instance()->prepare_for_run();
            task::instance()->sample_started(); // workaround::method start
            // increment sample number
            if ( auto run = document::instance()->sampleRun() )
                ++( *run );
        }
    };

    struct exec_fsm_ready {
        void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
            // make immediate inject
            // adacquire::task::instance()->fsmInject();
        }
    };

    struct exec_fsm_inject {
        void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
            task::instance()->sample_injected();
            for ( auto& iController : iControllers ) {
                if ( auto session = iController->getInstrumentSession() )
                    session->event_out( adacquire::Instrument::instEventInjectOut ); // loopback to peripherals
            }
        }
    };

    struct exec_fsm_complete {
        void operator ()(  std::vector< std::shared_ptr< adextension::iController > >& iControllers ) const {
            adacquire::task::instance()->fsmStart();
            adacquire::task::instance()->fsmReady();
        }
    };

    // template<typename T> struct wrap {};

    //..........................................
    class document::impl {
    public:
        static document * instance_; // workaround
        static std::mutex mutex_;
        static const std::chrono::system_clock::time_point uptime_;
        static const uint64_t tp0_;

        std::mutex acquire_mutex_;

        std::shared_ptr< adcontrols::SampleRun > nextSampleRun_;
        std::shared_ptr< ::acquire::iDGMODImpl > iDGMODImpl_;
        std::shared_ptr< adextension::iSequenceImpl > iSequenceImpl_;
        std::vector< std::shared_ptr< adextension::iController > > iControllers_;
        std::vector< std::shared_ptr< adextension::iController > > activeControllers_;
        std::map< boost::uuids::uuid, std::shared_ptr< adacquire::SignalObserver::Observer > > observers_;
        bool isMethodDirty_;
        std::atomic< bool > autoZeroFlag_;

        std::shared_ptr< adcontrols::ControlMethod::Method > cm_;

        int32_t device_status_;

        std::shared_ptr< QSettings > settings_;  // user scope settings
        QString ctrlmethod_filename_;

        std::map< QString, bool > moduleLists_;
        std::set< QString > blockedModules_;
        std::ofstream console_;
        QString http_host_;
        QString http_port_;

        std::atomic_bool acquire_polling_stop_request_;
        std::atomic_bool acquire_polling_stopped_;
        bool  acquire_polling_enabled_;
        uint32_t acquire_polling_interval_;
        uint32_t acquire_polling_mtu_;

        bool     acquire_avgr_enabled_;
        double acquire_dg_adc_delay_;
        double acquire_dg_interval_;
        uint32_t acquire_avgr_samples_;
        uint32_t acquire_avgr_trig_counts_;
        uint32_t acquire_avgr_index_;
        uint32_t acquire_avgr_trig_number_;
        std::atomic< bool > acquire_avgr_dirty_;
        int32_t acquire_pkd_threshold_;
        bool  acquire_pkd_algo_;

        bool avrg_disable_dma_;
        bool avrg_ext_trigger_;
        bool avrg_invert_data_;

        uint32_t polling_counts_;

        ///---------------
        QJsonDocument threshold_method_;
        QJsonDocument threshold_action_;
        std::shared_ptr< adcontrols::threshold_method > method_;
        uint32_t avrg_count_;
        bool avrg_refresh_;
        std::map< uint32_t, std::pair< uint64_t, uint64_t > > event_tp_;

        // display data
        std::array< std::shared_ptr< adcontrols::Trace >, 8 > traces_;

        impl() : nextSampleRun_( std::make_shared< adcontrols::SampleRun >() )
               , iDGMODImpl_( std::make_shared< acquire::iDGMODImpl >() )
               , iSequenceImpl_( std::make_shared< adextension::iSequenceImpl >( "ACQUIRE" ) )
               , isMethodDirty_( true )
               , autoZeroFlag_( false )
               , cm_( std::make_shared< adcontrols::ControlMethod::Method >() )
               , device_status_( 0 )
               , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                           , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                           , QLatin1String( "acquire" ) ) )
               , blockedModules_( { "Acquire", "InfiTOF" } )
               , console_( "/dev/null" )
               , acquire_polling_enabled_( false )
               , acquire_polling_interval_( 1000 )
               , acquire_polling_mtu_( 4096 )
               , acquire_avgr_enabled_( false )
               , acquire_dg_adc_delay_( 0.0 )
               , acquire_dg_interval_( 1.0e-3 ) // := 1ms
               , acquire_avgr_samples_( 1024 )
               , acquire_avgr_trig_counts_( 0 )
               , acquire_avgr_index_( 0 )
               , acquire_avgr_trig_number_( 0 )
               , acquire_avgr_dirty_( true )
               , acquire_pkd_threshold_( 512 ) // 12bit singed int
               , acquire_pkd_algo_ ( false )
               , avrg_disable_dma_ ( false )
               , avrg_ext_trigger_ ( false )
               , avrg_invert_data_ ( false )
               , polling_counts_( 0 )
               , avrg_count_( 100 )
               , avrg_refresh_( false )
            {
                method_ = std::make_shared< adcontrols::threshold_method >();
                adcontrols::TofChromatogramsMethod tofm;
                tofm.setNumberOfTriggers( 1000 );

                for ( size_t i = 0; i < traces_.size(); ++i ) {
                    traces_[ i ] = std::make_shared< adcontrols::Trace >( i, 8192 - 512, 8192 );
                    traces_[ i ]->setEnable( true );
                }
        }

        void handle_fsm_state_changed( bool enter, int id_state, adacquire::Instrument::eInstStatus st ) {
            if ( enter )
                emit document::instance()->instStateChanged( st );
        }

        void handle_fsm_action( adacquire::Instrument::idFSMAction a ) {

            typedef boost::mpl::vector< exec_fsm_stop, exec_fsm_start, exec_fsm_ready, exec_fsm_inject, exec_fsm_complete > actions;

            switch( a ) {
            case adacquire::Instrument::fsmStop:
                boost::mpl::at_c<actions, adacquire::Instrument::fsmStop>::type()( iControllers_ );
                break;
            case adacquire::Instrument::fsmStart:
                boost::mpl::at_c<actions, adacquire::Instrument::fsmStart>::type()( iControllers_ );
                break;
            case adacquire::Instrument::fsmReady:
                boost::mpl::at_c<actions, adacquire::Instrument::fsmReady>::type()( iControllers_ );
                break;
            case adacquire::Instrument::fsmInject:
                boost::mpl::at_c<actions, adacquire::Instrument::fsmInject>::type()( iControllers_ );
                break;
            case adacquire::Instrument::fsmComplete:
                boost::mpl::at_c<actions, adacquire::Instrument::fsmComplete>::type()( iControllers_ );
                break;
            }
        }

        bool initStorage( const boost::uuids::uuid& uuid, adfs::sqlite& db ) const  {
            std::string objtext;

            if ( uuid == boost::uuids::uuid{{ 0 }} ) {
                objtext = "master.observer";
            } else {
                auto it = observers_.find( uuid );
                if ( it != observers_.end() )
                    objtext = it->second->objtext();
                else
                    return false;
            }

            ADDEBUG() << "## " << __FUNCTION__ << " " << uuid << ", " << objtext;

            do {
                adfs::stmt sql( db );

                static boost::uuids::uuid uuid_massspectrometer = boost::uuids::string_generator()( adspectrometer::MassSpectrometer::clsid_text );
                sql.prepare( "INSERT OR REPLACE INTO Spectrometer ( id, scanType, description, fLength ) VALUES ( ?,?,?,? )" );
                sql.bind( 1 ) = uuid_massspectrometer;
                sql.bind( 2 ) = 0;
                sql.bind( 3 ) = std::string( adspectrometer::MassSpectrometer::class_name );
                sql.bind( 4 ) = 1.0; // scanLaw->fLength( 0 ); // fLength at mode 0

                if ( sql.step() != adfs::sqlite_done )
                    ADDEBUG() << "sqlite error";
            } while ( 0 );

            // Save method
            if ( uuid == boost::uuids::uuid{{ 0 }} ) {
                // only if call for master observer

                adfs::stmt sql( db );
                sql.exec( "CREATE TABLE IF NOT EXISTS MetaData (clsid UUID, attrib TEXT, data BLOB )" ); // check adutils/AcquiredData::create_table_v3

                std::string ar;
                {
                    auto cm( cm_ );
                    boost::iostreams::back_insert_device< std::string > inserter( ar );
                    boost::iostreams::stream< boost::iostreams::back_insert_device< std::string > > device( inserter );
                    adcontrols::ControlMethod::Method::archive( device, *cm );
                }

                sql.prepare( "INSERT OR REPLACE INTO MetaData ( clsid, attrib, data ) VALUES ( ?,?,? )" );
                sql.bind( 1 ) = adcontrols::ControlMethod::Method::clsid();
                sql.bind( 2 ) = std::string( "ControlMethod::Method" );
                sql.bind( 3 ) = adfs::blob( ar.size(), reinterpret_cast< const int8_t * >( ar.data() ) );
                if ( sql.step() != adfs::sqlite_done )
                    ADDEBUG() << "sqlite error";
            }
            return true;
        }

        void apply() {
            // acquire_rx::instance<0>()->set_dg_adc_delay( acquire_dg_adc_delay_ );
            // acquire_rx::instance<0>()->set_dg_interval( acquire_dg_interval_ );
            // acquire_rx::instance<0>()->set_avgr_num_triggers( acquire_avgr_trig_counts_ );
            // acquire_rx::instance<0>()->set_avgr_disable_dma( avrg_disable_dma_ );
            // acquire_rx::instance<0>()->set_invert_data( avrg_invert_data_ );
            // acquire_rx::instance<0>()->set_pkd_threshold( acquire_pkd_threshold_ );
            // acquire_rx::instance<0>()->set_pkd_algo( acquire_pkd_algo_ );
            acquire_avgr_dirty_ = false;
        }

        void set_http_addr( const QString& host, const QString& port ) {
            http_host_ = host;
            http_port_ = port;
            settings_->setValue( Constants::THIS_GROUP + QString("/http_host"), host );
            settings_->setValue( Constants::THIS_GROUP + QString("/http_port"), port );
        }
        void config_http_addr( const QString& host, const QString& port ) {
            QJsonObject jobj {
                { "ip_address", host }
                ,{ "port",  port }
            };
            QJsonDocument jdoc( jobj );
            auto json = std::string( jdoc.toJson( QJsonDocument::Compact ).data() );
            if ( auto inst = iDGMODImpl_->getInstrumentSession() )
                inst->setConfiguration( json );
        }
    };

    //////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////

    std::mutex document::impl::mutex_;
    document * document::impl::instance_( 0 );
    const std::chrono::system_clock::time_point document::impl::uptime_ = std::chrono::system_clock::now();
    const uint64_t document::impl::tp0_ =
        std::chrono::duration_cast<std::chrono::nanoseconds>( document::impl::uptime_.time_since_epoch() ).count();
}

document::document() : impl_( new impl() )
{
    // diagnostic
    QVariant v;
    v.setValue( boost::uuids::uuid() );
}

document::~document()
{
    delete impl_;
    ADTRACE() << "=====> document dtor";
}

document *
document::instance()
{
    static std::once_flag flag;

    std::call_once( flag, [=] () { impl::instance_ = new document(); } );
    return impl::instance_;
}

void
document::actionConnect()
{
    using namespace std::literals::chrono_literals;

    if ( !impl_->iControllers_.empty() ) {

        std::vector< std::future<bool> > futures;

        std::vector< std::shared_ptr< adextension::iController > > activeControllers;

        for ( auto& iController : impl_->iControllers_ ) {

            if ( isControllerEnabled( iController->module_name() ) ) {

                ADTRACE() << "acquire actionConnect connecting to " << iController->module_name().toStdString();

                activeControllers.emplace_back( iController );

                futures.emplace_back( std::async( [iController] () { return iController->wait_for_connection_ready( 3s ); } ) );

                 if ( iController->connect() ) {
                     if ( iController->module_name() == iDGMODImpl::__module_name__ ) {
                         impl_->config_http_addr( impl_->http_host_, impl_->http_port_ );
                     }
                 }
            }
        }

        QStringList failed;
        size_t i = 0;
        for ( auto& future : futures ) {
            if ( future.get() )
                impl_->activeControllers_.push_back( activeControllers[ i ] );
            else
                failed << activeControllers[ i ]->module_name();
            ++i;
        }

        emit onModulesFailed( failed );

        auto cm = MainWindow::instance()->getControlMethod();
        //setControlMethod( *cm, QString() );

        futures.clear();
        for ( auto& iController : impl_->iControllers_ ) {
            if ( auto session = iController->getInstrumentSession() ) {
                session->initialize();
                session->prepare_for_run( cm );
            }
        }

        // setup observer hiralchey
        if ( auto masterObserver = adacquire::task::instance()->masterObserver() ) {

            masterObserver->setPrepareStorage( [&]( adacquire::SampleProcessor& sp ) {
                                                   return document::instance()->prepareStorage( boost::uuids::uuid{{ 0 }}, sp );
                                               } );

            masterObserver->setClosingStorage( [&]( adacquire::SampleProcessor& sp ) {
                                                   return document::instance()->closingStorage( boost::uuids::uuid{{ 0 }}, sp );
                                               } );

            for ( auto& iController : impl_->iControllers_ ) {
                if ( auto session = iController->getInstrumentSession() ) {
                    if ( auto observer = session->getObserver() ) {
                        masterObserver->addSibling( observer );
                    }
                }
            }

            // debug
            // ADDEBUG() << "=======================================================";
            // auto vec = masterObserver->siblings();
            // for ( auto& o: vec )
            //     ADDEBUG() << "============" << o->objtext() << ", " << o->objid();
            // ADDEBUG() << "=======================================================";
            // end debug
        } else {
            ADTRACE() << "##### No master observer found #####";
        }

        // FSM Action
        adacquire::task::instance()->connect_fsm_action( std::bind( &impl::handle_fsm_action, impl_, std::placeholders::_1 ) );

        // FSM State
        adacquire::task::instance()->connect_fsm_state(
            std::bind( &impl::handle_fsm_state_changed, impl_, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );

        adacquire::task::instance()->fsmStart();
        adacquire::task::instance()->fsmReady();
    }
}

void
document::actionInject()
{
    adacquire::task::instance()->fsmInject();
}

void
document::actionRec( bool onoff )
{
    task::instance()->setRecording( onoff );

    for ( auto inst : impl_->iControllers_ ) {
        if ( auto session = inst->getInstrumentSession() )
            session->recording( onoff );
    }
}

void
document::actionSyncTrig()
{
    //impl_->coadded_.clear();
}

void
document::actionRun()
{
    adacquire::task::instance()->fsmStart();
}

void
document::actionStop()
{
    adacquire::task::instance()->fsmStop();
}

void
document::addInstController( adextension::iController * p )
{
    try {
        if ( auto ptr = p->pThis() ) {

            using adextension::iController;
            using adacquire::SignalObserver::Observer;

            if ( document::instance()->isControllerEnabled( p->module_name() ) ) {

                impl_->console_ << "addInstController : " << p->module_name().toStdString() << std::endl;

                impl_->iControllers_.emplace_back( ptr );

                // switch to UI thread
                connect( p, &iController::message, document::instance()
                         , [] ( iController * p, unsigned int code, unsigned int value ) { document::instance()->handleMessage( p, code, value ); } );

                connect( p, &iController::notifyInfo, [](iController * p, const QByteArray& json){ document::instance()->handleInfo( p, json ); });

                // non UI thread
                connect( p, &iController::connected, [this] ( iController * p ) { handleConnected( p ); } );
                //connect( p, &iController::log, [this] ( iController * p, const QString& log ) { handleLog( p, log ); } );

                ptr->dataChangedHandler( [] ( Observer *o, unsigned int pos ) {
                                             task::instance()->onDataChanged( o, pos );
                                         } );
            }
        }

    } catch ( std::bad_weak_ptr& ) {

        QMessageBox::warning( MainWindow::instance(), "ACQUIRE plugin"
                              , QString( tr( "Instrument controller %1 has no shared_ptr; ignored." ) ).arg( p->module_name() ) );

    }
}

void
document::prepare_next_sample( std::shared_ptr< adcontrols::SampleRun > run, const adcontrols::ControlMethod::Method& cm )
{
    // make empty que
    while( auto sample = adacquire::task::instance()->deque() )
        ;

    // push new sample
    adacquire::task::instance()->prepare_next_sample( run, cm );

    // set INJECTION WAITING
    adacquire::task::instance()->fsmReady();

    emit sampleRunChanged();
}

void
document::prepare_for_run()
{
    using adcontrols::ControlMethod::MethodItem;

    auto cm = MainWindow::instance()->getControlMethod();

    ADTRACE() << "## prepare_for_run ##";
    prepare_next_sample( impl_->nextSampleRun_, *impl_->cm_ );

    std::vector< std::future< bool > > futures;
    for ( auto& iController : impl_->iControllers_ ) {
        if ( auto session = iController->getInstrumentSession() ) {
            futures.push_back( std::async( [=] () { return session->prepare_for_run( cm ); } ) );
        }
    }
    if ( !futures.empty() )
        task::instance()->post( futures );
}

void
document::start_run()
{
    ADTRACE() << "### start run ###";
    prepare_for_run();
}

void
document::stop()
{
    ADTRACE() << "### stop ###";

    std::vector< std::future< bool > > futures;

    for ( auto& iController : impl_->iControllers_ ) {
        if ( auto session = iController->getInstrumentSession() ) {
            futures.push_back( std::async( [=] () { return session->stop_run(); } ) );
        }
    }
    if ( !futures.empty() )
        task::instance()->post( futures );

}

int32_t
document::device_status() const
{
    return impl_->device_status_;
}


// static
bool
document::appendOnFile( const boost::filesystem::path& path
                        , const QString& title
                        , const adcontrols::MassSpectrum& ms
                        , QString& id )
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
		adfs::file file = folder.addFile( adfs::create_uuid(), title.toStdWString() );
        if ( file ) {
            file.dataClass( ms.dataClass() );
            id = QString::fromStdWString( file.id() );
            if ( file.save( ms ) )
				file.commit();
        }
	}
    return true;

}

void
document::initialSetup()
{
    adlog::logging_handler::instance()->register_handler(
        [&]( int pri, const std::string& text, const std::string& file, int line, const std::chrono::system_clock::time_point& tp ){
            impl_->console_ << text << std::endl;
        });
    adportable::core::debug_core::instance()->open( std::string() ); // disable log file
    adlog::logging_handler::instance()->setlogfile( std::string() );

    boost::filesystem::path dir = user_preference::path( impl_->settings_.get() );

    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "acquire::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    QString path = recentFile( Constants::GRP_DATA_FILES, false );
    if ( path.isEmpty() ) {
        path = QString::fromStdWString(
            ( boost::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data" ).generic_wstring() );
    } else {
        path = QFileInfo( path ).path();
    }

    if ( auto run = std::make_shared< adcontrols::SampleRun >() ) {

        boost::filesystem::path fname( dir / "samplerun.xml" );
        if ( boost::filesystem::exists( fname ) ) {
            std::wifstream inf( fname.string() );
            try {
                adcontrols::SampleRun::xml_restore( inf, *run );

                // replace directory name to 'today'
                run->setDataDirectory( impl_->nextSampleRun_->dataDirectory() ); // reset data directory to ctor default
                adacquire::SampleProcessor::prepare_sample_run( *run, false );

                MainWindow::instance()->setSampleRun( *run );
                document::setSampleRun( run );

            } catch ( std::exception& ex ) {
                ADTRACE() << ex.what();
            }
        }
    }

    // load last control method from ini-file
    if ( impl_->cm_ ) {
        boost::filesystem::path fname( dir / Constants::LAST_METHOD );
        if ( load( QString::fromStdString( fname.string() ), *impl_->cm_ ) )
            MainWindow::instance()->setControlMethod( impl_->cm_ ); // <-- update MainWindos
    };

    if ( auto settings = impl_->settings_ ) {
        impl_->http_host_ = settings->value( Constants::THIS_GROUP + QString("/http_host"),  "192.168.0.132" ).toString();
        impl_->http_port_ = settings->value( Constants::THIS_GROUP + QString("/http_port"),  "http" ).toString();
    }

    if ( auto settings = impl_->settings_ ) {
        using namespace adwidgets;
        auto m = settings->value( QString( Constants::THIS_GROUP ) + "/threshold_method"
                                  , findSlopeForm::toJson( adcontrols::threshold_method() ) );
        impl_->threshold_method_ = QJsonDocument::fromJson( m.toByteArray() );

        // convert to binary method
        if ( impl_->threshold_method_.isArray() ) {
            auto m = std::make_shared< adcontrols::threshold_method >();
            if ( adwidgets::findSlopeForm::fromJson(
                     QJsonDocument( impl_->threshold_method_.array().at(0).toObject() ).toJson( QJsonDocument::Compact ), *m ) )
                impl_->method_ = m;
        }

        auto a = settings->value( QString( Constants::THIS_GROUP ) + "/threshold_action"
                                  , ThresholdActionForm::toJson( adcontrols::threshold_action() ) );
        impl_->threshold_action_ = QJsonDocument::fromJson( a.toByteArray() );
    }

    // force update trace method to waveformwnd
    handleTraceMethodChanged();
}

void
document::finalClose()
{
    for ( auto iController : impl_->iControllers_ )
        iController->disconnect( true );

    // make empty que
    while( auto sample = adacquire::task::instance()->deque() )
        ;

    task::instance()->finalize();

    boost::filesystem::path dir = user_preference::path( impl_->settings_.get() );
    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "acquire::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }

    if ( auto cm = MainWindow::instance()->getControlMethod() ) {
        boost::filesystem::path fname( dir / Constants::LAST_METHOD );
        save( QString::fromStdString( fname.string() ), *cm );
    }


    if ( auto run = sampleRun() ) {
        boost::filesystem::path fname( dir / "samplerun.xml" );
        std::wofstream outf( fname.string() );
        adcontrols::SampleRun::xml_archive( outf, *run );
    }

    if ( auto settings = impl_->settings_ ) {
        settings->beginGroup( Constants::THIS_GROUP );

        settings->beginWriteArray( "ControlModule" );

        int i = 0;
        for ( auto& state : impl_->moduleLists_ ) {
            settings->setArrayIndex( i++ );
            settings->setValue( "module_name", state.first );
            settings->setValue( "enable", state.second );
        }

        settings->endArray();
        settings->endGroup();

        settings->sync();
    }

    if ( auto settings = impl_->settings_ )
        settings->setValue( QString( Constants::THIS_GROUP ) + "/threshold_method", impl_->threshold_method_.toJson( QJsonDocument::Compact ) );

    if ( auto settings = impl_->settings_ )
        settings->setValue( QString(Constants::THIS_GROUP) + "/threshold_action", impl_->threshold_action_.toJson( QJsonDocument::Compact ) );

    if ( auto settings = impl_->settings_ )
        settings->sync();
}

void
document::addToRecentFiles( const QString& filename )
{
    qtwrapper::settings( *impl_->settings_ ).addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, filename );
}

QString
document::recentFile( const char * group, bool dir_on_fail )
{
    if ( group == 0 )
        group = Constants::GRP_DATA_FILES;

    QString file = qtwrapper::settings( *impl_->settings_ ).recentFile( group, Constants::KEY_FILES );
    if ( !file.isEmpty() )
        return file;

    if ( dir_on_fail ) {
        file = Core::DocumentManager::currentFile();
        if ( file.isEmpty() )
            file = qtwrapper::settings( *impl_->settings_ ).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );

        if ( !file.isEmpty() ) {
            QFileInfo fi( file );
            return fi.path();
        }
        return QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() );
    }
    return QString();
}

std::shared_ptr< const adcontrols::SampleRun >
document::sampleRun() const
{
    return impl_->nextSampleRun_;
}

std::shared_ptr< adcontrols::SampleRun >
document::sampleRun()
{
    return impl_->nextSampleRun_;
}

void
document::setSampleRun( std::shared_ptr< adcontrols::SampleRun > sr )
{
    impl_->nextSampleRun_ = sr;
}

bool
document::isRecording() const
{
    return task::instance()->isRecording();
}

void
document::handleConnected( adextension::iController * controller )
{
    task::instance()->initialize();
}

void
document::handleMessage( adextension::iController * ic, uint32_t code, uint32_t value )
{
    if ( code == adacquire::Receiver::CLIENT_ATTACHED ) {

        // do nothing

    } else if ( code == adacquire::Receiver::STATE_CHANGED ) {

        if ( value & adacquire::Instrument::eErrorFlag ) {
            QMessageBox::warning( MainWindow::instance(), "ACQUIRE Error"
                                  , QString( "Module %1 error with code %2" ).arg( ic->module_name(), QString::number( value, 16 ) ) );
        }
    }
}

void
document::handleInfo( adextension::iController *, const QByteArray& json )
{
    static std::chrono::steady_clock::time_point __tp;

    auto jtick = QJsonDocument::fromJson( json ).object()[ "tick" ].toObject();
    if ( ! jtick.isEmpty() ) {
        emit onTick( json );
        if ( std::chrono::duration_cast< std::chrono::seconds >( std::chrono::steady_clock::now() - __tp ).count() > 6 ) {
            adurl::ajax ajax( impl_->http_host_.toStdString(), impl_->http_port_.toStdString() );
            if ( ajax( "GET", "/dg/ctl?status.json" ) ) {
                size_t sz;
                if ( auto resp = ajax.get_response( sz ) ) {
                    QByteArray data( resp, sz );
                    emit onDelayPulseData( data );
                }
            }
        }
    }
}

void
document::commitData()
{
    // save time data
    //impl_->resultWriter_->commitData();
}

QSettings *
document::settings()
{
    return impl_->settings_.get();
}

adextension::iSequenceImpl *
document::iSequence()
{
    return impl_->iSequenceImpl_.get();
}

bool
document::isControllerEnabled( const QString& module ) const
{
    if ( impl_->iDGMODImpl_->module_name() == module )
        return true;
    return false;
}

bool
document::isControllerBlocked( const QString& module ) const
{
    return impl_->blockedModules_.find( module ) != impl_->blockedModules_.end();
}

bool
document::prepareStorage( const boost::uuids::uuid& uuid, adacquire::SampleProcessor& sp ) const
{
    if ( uuid == boost::uuids::uuid{{ 0 }} )
        ADTRACE() << "## prepare storage '" << sp.storage_name().string() << "'";

    progress( 0.0, sp.sampleRun() ); // show data name on top of waveformwnd

    if ( impl_->initStorage( uuid, sp.filesystem().db() ) && uuid == boost::uuids::uuid{{ 0 }} ) {

        // // counting peaks
        // if ( uuid == boost::uuids::uuid{{ 0 }} ) {
        //     acquire::counting_data_writer::prepare_storage( sp.filesystem() );
        // };

        return true;
    }
    return false;
}

bool
document::closingStorage( const boost::uuids::uuid&, adacquire::SampleProcessor& ) const
{
    return true;
}

std::shared_ptr< const adcontrols::ControlMethod::Method >
document::controlMethod() const
{
    return nullptr;
}

void
document::acquire_ip_addr( const QString& host, const QString& port )
{
    ADTRACE() << __FUNCTION__ << host.toStdString() << ", " << port.toStdString();

}

QVector< QPair< QString, bool > >
document::controllerSettings() const
{
    QVector< QPair< QString, bool > > modules;
    for ( auto module: impl_->moduleLists_ )
        modules.append( { module.first, module.second } );
    return modules;
}

void
document::setControllerSettings( const QString& module, bool enable )
{
    ADTRACE() << module.toStdString() << ", " << enable;
    bool dirty( false );

    if ( impl_->moduleLists_[ module ] != enable )
        dirty = true;

    impl_->moduleLists_[ module ] = enable;

    // update settings
    impl_->settings_->beginGroup( Constants::THIS_GROUP );

    impl_->settings_->beginWriteArray( "Controllers" );

    int i = 0;
    for ( auto& state : impl_->moduleLists_ ) {
        impl_->settings_->setArrayIndex( i++ );
        impl_->settings_->setValue( "module_name", state.first );
        impl_->settings_->setValue( "enable", state.second );
    }

    impl_->settings_->endArray();
    impl_->settings_->endGroup();

    emit moduleConfigChanged();
}

void
document::addInstController( std::shared_ptr< adextension::iController > p )
{
    using adextension::iController;
    using adacquire::SignalObserver::Observer;

    if ( document::instance()->isControllerEnabled( p->module_name() ) ) {

        impl_->iControllers_.emplace_back( p );

        // switch to UI thread
        connect( p.get(), &iController::message, document::instance()
                 , [] ( iController * p, unsigned int code, unsigned int value ) { document::instance()->handleMessage( p, code, value ); } );

        // non UI thread
        connect( p.get(), &iController::connected, [this] ( iController * p ) { handleConnected( p ); } );
        //connect( p.get(), &iController::log, [this] ( iController * p, const QString& log ) { handleLog( p, log ); } );

        p->dataChangedHandler( [] ( Observer *o, unsigned int pos ) { task::instance()->onDataChanged( o, pos ); } );
    }

}

std::vector< adextension::iController * >
document::iControllers() const
{
    return { impl_->iDGMODImpl_.get() };
}

void
document::loadControllerSettings()
{
    auto& settings = impl_->settings_;
    auto& moduleLists = impl_->moduleLists_;

    settings->beginGroup( Constants::THIS_GROUP );

    int size = settings->beginReadArray( "Controllers" );

    for ( int i = 0; i < size; ++i ) {
        settings->setArrayIndex( i );
        QString module_name = settings->value("module_name").toString();
        bool enable = settings->value( "enable" ).toBool();
        moduleLists[ module_name ] = enable;
    }

    settings->endArray();
    settings->endGroup();

    for ( auto& list : impl_->blockedModules_ ) {
        auto it = moduleLists.find( list  );
        if ( it == moduleLists.end() ) {
            moduleLists[ list ] = false;
        }
    }
}

std::ostream&
document::console()
{
    return impl_->console_;
}

void
document::handleConsoleIn( const QString& line )
{
    ADTRACE() << line.toStdString();
}

bool
document::poll()
{
    return impl_->acquire_polling_enabled_;
}

void
document::acquire_apply( const QByteArray& json )
{
    impl_->console_ << json.data() << std::endl;
}

void
document::takeSnapshot()
{
    boost::filesystem::path dir( impl_->nextSampleRun_->dataDirectory() );

    if ( ! boost::filesystem::exists( dir ) ) {
        boost::system::error_code ec;
        boost::filesystem::create_directories( dir, ec );
    }
    auto path = dir / ( std::wstring( impl_->nextSampleRun_->filePrefix() ) + L"_snapshots.adfs" );

    std::chrono::system_clock::time_point tp = std::chrono::system_clock::now();
    std::string date = adportable::date_string::logformat( tp );

    // nothing to be done
}

const QJsonDocument&
document::threshold_method() const
{
    return impl_->threshold_method_;
}

const QJsonDocument&
document::threshold_action() const
{
    return impl_->threshold_action_;
}

QByteArray
document::acquire_method() const
{
    if ( auto settings = document::instance()->settings() )
        return settings->value( QString( Constants::THIS_GROUP ) + "/acquire", QByteArray() ).toByteArray();

    return QByteArray();
}

void
document::set_threshold_method( const QByteArray& json, int ch )
{
    auto obj = QJsonDocument::fromJson( json ).object();
    auto jslope = obj[ "findSlope" ];

    if ( jslope.isArray() ) {
        auto a = jslope.toArray().at( ch );
        emit on_threshold_method_changed( QJsonDocument( a.toObject() ) );
        impl_->threshold_method_ = QJsonDocument( jslope.toArray() );

        auto m = std::make_shared< adcontrols::threshold_method >();
        if ( impl_->threshold_method_.isArray() ) {
            auto ch = impl_->threshold_method_.array().at(0);
            if ( adwidgets::findSlopeForm::fromJson( QJsonDocument( ch.toObject() ).toJson( QJsonDocument::Compact ), *m ) )
                impl_->method_ = m;
        }
    }

    // debug
#if defined NDEBUG && 0
    {
        auto json = impl_->threshold_method_.toJson();
        ADDEBUG() << json.toStdString();
        auto doc = QJsonDocument::fromJson( json );
        if ( doc.isArray() ) {
            for ( auto ch: doc.array() ) {
                ADDEBUG() << QJsonDocument( ch.toObject() ).toJson().toStdString();
            }
        }
    }
#endif
}

void
document::set_threshold_action( const QByteArray& json )
{
    auto obj = QJsonDocument::fromJson( json ).object();
    auto jaction = obj[ "action" ];

    impl_->threshold_action_ = QJsonDocument( jaction.toObject() );

    emit on_threshold_action_changed( impl_->threshold_action_ );
}

void
document::set_tof_chromatograms_method( const QByteArray& json, bool commitSettings )
{
    auto doc = QJsonDocument::fromJson( json );
    auto jtop = doc.object()[ QString::fromStdString( adcontrols::TofChromatogramsMethod::modelClass() ) ].toObject();

    impl_->avrg_refresh_ = jtop[ "refreshHistogram" ].toBool();
    impl_->avrg_count_ = jtop[ "numberOfTriggers" ].toInt();

    task::instance()->setHistogramClearCycleEnabled( impl_->avrg_refresh_ );
    task::instance()->setHistogramClearCycle( impl_->avrg_count_ );

    auto jlist = jtop[ "list" ].toArray();
    size_t idx(0);
    for ( const auto& item: jlist ) {
        if ( idx < impl_->traces_.size() ) {
            const auto& obj = item.toObject();
            auto& trace = impl_->traces_.at( idx );
            bool enable = obj[ "enable" ].toBool();
            auto formula = obj[ "formula" ].toString().toStdString();
            auto legend = adcontrols::ChemicalFormula::formatFormula( formula );

            trace->setEnable( enable ); // id[0] is always true
            trace->setLegend( legend.empty() ? formula : legend );
        }
        ++idx;
    }

    while ( idx < impl_->traces_.size() )
        impl_->traces_.at( idx++ )->setEnable(false);

    // check if nothing enabled
    auto it = std::find_if( impl_->traces_.begin(), impl_->traces_.end(), [](const auto& t){ return t->enable(); });
    if ( it == impl_->traces_.end() ) {
        impl_->traces_.at( 0 )->setEnable( true );
    }
    if ( commitSettings ) {
        if ( auto settings = impl_->settings_ )
            settings->setValue( QString( Constants::THIS_GROUP ) + "/tofChromatogramsMethod", doc.toJson( QJsonDocument::Compact ) );
    }
}

QByteArray
document::tof_chromatograms_method() const
{
    if ( auto settings = impl_->settings_ )
        return settings->value( QString( Constants::THIS_GROUP ) + "/tofChromatogramsMethod", QByteArray() ).toByteArray();
    return QByteArray();
}

void
document::set_trace_method( const admethods::controlmethod::ADTraceMethod& m )
{
    for ( size_t idx = 0; idx < impl_->traces_.size() && idx < m.size(); ++idx ) {
        const auto& adtrace = m[ idx ];

        auto& trace = impl_->traces_.at( idx );
        trace->setEnable( adtrace.enable() ); // id[0] is always true
        trace->setLegend( adtrace.legend() );
        trace->setYOffset( adtrace.vOffset() );
    }

    // check if nothing enabled
    auto it = std::find_if( impl_->traces_.begin(), impl_->traces_.end(), [](const auto& t){ return t->enable(); });
    if ( it == impl_->traces_.end() ) {
        impl_->traces_.at( 0 )->setEnable( true );
    }
}

//////////////////
void
document::progress( double elapsed_time, std::shared_ptr< const adcontrols::SampleRun >&& sampleRun ) const
{
    double method_time = sampleRun->methodTime();
    QString runName = QString::fromStdWString( sampleRun->filePrefix() );

    emit sampleProgress( elapsed_time, method_time, runName, sampleRun->runCount() + 1, sampleRun->replicates() );
}

void
document::set_pkd_threshold( double d )
{
    // ACQUIRE GUI --> this
    emit on_threshold_level_changed( d );
}

std::pair< QString, QString >
document::http_addr() const
{
    return std::make_pair( impl_->http_host_, impl_->http_port_ );
}

void
document::set_http_addr( const QString& host, const QString& port )
{
    impl_->set_http_addr( host, port );
    impl_->config_http_addr( host, port );
}

//static
bool
document::write( const adacquire::SampleProcessor& sp, std::unique_ptr< map::trigger_data >&& data )
{
    adfs::stmt sql( sp.filesystem().db() );
#if 0
    sql.prepare( "INSERT INTO map_trigger (trig_count,fpga_clock,posix_clock,protocol,wellKnownEvents"
                 ",dac_clock,dac_x,dac_y,adc_clock,adc_x,adc_y)"
                 " VALUES (?,?,?,?,?,?,?,?,?,?,?)" );

    for ( auto& d: *data ) {
        int id(1);
        double adc_x = ( d.advalues()[0] - 1250 ) * 2;
        double adc_y = ( d.advalues()[1] - 1250 ) * 2;
        sql.bind( id++ ) = d.trig_count();
        sql.bind( id++ ) = d.fpga_clock();
        sql.bind( id++ ) = d.posix_clock();
        sql.bind( id++ ) = d.protocol();
        sql.bind( id++ ) = d.wellKnownEvents();
        sql.bind( id++ ) = d.dac_clock();
        sql.bind( id++ ) = d.dac_x();
        sql.bind( id++ ) = d.dac_y();
        sql.bind( id++ ) = d.adc_clock();
        sql.bind( id++ ) = adc_x;
        sql.bind( id++ ) = adc_y;

        if ( sql.step() != adfs::sqlite_done ) {
            ADDEBUG() << "sql error: " << sql.errmsg();
            break;
        }

        sql.reset();
    }
#endif
    return true;
}

//static
void
document::debug_write( const std::vector< std::pair< std::string, std::string > >& headers, const map::trigger_data& data )
{
#if 0
    constexpr const size_t llimit = 15;
    static uint64_t fpga_clock(0), posix_clock(0), trig_count(0), skip_count(0);
    static int64_t last_skip(0);
    int i = 0;

    for ( auto& datum: data ) {
        if ( i < llimit || i >= data.size() - 2 ) {
            std::chrono::nanoseconds dur( datum.posix_clock() );
            auto tp = std::chrono::time_point< std::chrono::system_clock >( dur );
            using namespace date;
            std::ostringstream o;
            o << tp;
            ADDEBUG() << boost::format( "[%4d] %d %s\t%.3lf s\t%8.3lf\t%8.3lf ms\t%d" )
                % i
                % datum.trig_count()
                % o.str()
                % (datum.fpga_clock() * 1.0e-9)
                % (( datum.fpga_clock() - fpga_clock )/1.0e6)
                % (( datum.posix_clock() - posix_clock )/1.0e6)
                % skip_count
                      << ", " << last_skip;
        } else if ( i == llimit + 1 ) {
            ADDEBUG() << "\t...snip...";
        }
        if ( trig_count && ( (trig_count + 1 ) != datum.trig_count() ) ) {
            ++skip_count;
            last_skip = datum.trig_count() - trig_count;
            ADDEBUG() << "skip detected: " << trig_count
                      << boost::format( " [%4d] %d\t%.3lf s\t%8.3lf\t%8.3lf ms\t%d" )
                % i
                % datum.trig_count()
                % (datum.fpga_clock() * 1.0e-9)
                % (( datum.fpga_clock() - fpga_clock )/1.0e6)
                % (( datum.posix_clock() - posix_clock )/1.0e6)
                % skip_count
                      << ", " << last_skip;
        }
        ++i;
        fpga_clock = datum.fpga_clock();
        posix_clock = datum.posix_clock();
        trig_count = datum.trig_count();
    }
#endif
}

void
document::debug_sse( const std::vector< std::pair< std::string, std::string> >& headers, const std::string& body )
{
    bool ok( false );
    {
        auto it = std::find_if( headers.begin(), headers.end()
                                , [](const auto& pair){ return pair.first == "event" && pair.second == "ad.values"; } );
        ok = it != headers.end();
    }

    if ( ok ) {
        auto it = std::find_if( headers.begin(), headers.end(), [&](auto& pair){ return pair.first == "data"; });
        if ( it != headers.end() )  {
            auto jdoc = QJsonDocument::fromJson( QByteArray( it->second.data(), it->second.size() ) );
            auto jarray = jdoc.object()[ "ad.values" ].toArray();
            for ( const auto& jadval: jarray ) {
                qDebug() << __FILE__ << ":" << __LINE__ << "\t" << jadval;
            }
        }
    }

}

void
document::debug_data( const std::vector< socfpga::dgmod::advalue >& vec )
{
    for ( const auto& item: vec ) {
        std::ostringstream o;
        for ( auto& ad: item.ad )
            o << boost::format("%.3f, ") % ad;
#if defined (Q_OS_MACOS)
        std::chrono::system_clock::time_point tp( std::chrono::microseconds( item.posix_time / 1000 ) );
#else
        std::chrono::system_clock::time_point tp( std::chrono::nanoseconds( item.posix_time ) );
#endif
        using namespace date;
        impl_->console_ << "tp: " << tp << " " << item.elapsed_time << "\tnacc: " << item.nacc << "\t" << o.str() << std::endl;
    }
}

void
document::setData( const std::vector< socfpga::dgmod::advalue >& data )
{
    bool injflag( false );
    for ( auto& item: data ) {
        double time = double(item.elapsed_time) / std::nano::den;

        if ( !injflag && ( item.flags & adacquire::SignalObserver::wkEvent_INJECT ) ) {
            injflag = true;
            for ( auto& trace: impl_->traces_ )
                trace->setInjectTime( double( item.flags_time ) / std::nano::den );
            std::lock_guard< std::mutex > lock( impl_->mutex_ );
            impl_->event_tp_[ adacquire::SignalObserver::wkEvent_INJECT ] = { item.posix_time, item.flags_time };
        }

        for ( size_t ch = 0; ch < item.ad.size() && ch < impl_->traces_.size(); ++ch ) {
            auto& trace = impl_->traces_.at( ch );
            trace->append( item.adc_counter, time, item.ad[ ch ], item.flags );
        }
    }

    do {
        std::lock_guard< std::mutex > lock( impl_->mutex_ );
        impl_->event_tp_[ 0 ] = { data.back().posix_time, data.back().elapsed_time };
    } while(0);

    if ( impl_->autoZeroFlag_ ) {
        impl_->autoZeroFlag_ = false;
        QVector<double> avrg( 8 );
        for ( auto& d: data )
            std::transform( avrg.begin(), avrg.end(), d.ad.begin(), avrg.begin(), std::plus< double >() );
        for ( auto& a: avrg )
            a /= data.size();
        emit on_auto_zero_changed( avrg );
    }

    emit document::instance()->dataChanged( socfpga::dgmod::trace_observer, -1 );
}

void
document::getTraces( std::vector< std::shared_ptr< adcontrols::Trace > >& traces )
{
    traces.clear();

    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    for ( auto trace: impl_->traces_ )
        traces.emplace_back( trace );
}

bool
document::applyTimedEvent( std::shared_ptr< const adcontrols::ControlMethod::TimedEvents > tt
                           , adcontrols::ControlMethod::const_time_event_iterator begin
                           , adcontrols::ControlMethod::const_time_event_iterator end )
{
    // // ---> infiTOF tofdll
    // hv::Session::instance()->time_event_trigger( tt, begin, end );

    // for ( auto& iController : impl_->iControllers_ ) {
    //     if ( auto session = iController->getInstrumentSession() )
    //         session->time_event_trigger( tt, begin, end );
    // }
    return true;
}

// posix_time, elapsed_time
std::pair< uint64_t, uint64_t >
document::find_event_time( uint32_t wellKnownEvent ) const
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    auto it = impl_->event_tp_.find( wellKnownEvent );
    if ( it != impl_->event_tp_.end() )
        return it->second;
    return { 0, 0 };
}

bool
document::save( const QString& filename, const adcontrols::ControlMethod::Method& m ) const
{
    adfs::filesystem file;

    if ( !file.create( filename.toStdWString().c_str() ) ) {
        ADTRACE() << "Error: \"" << filename.toStdString() << "\" can't be created";
        return false;
    }

    return adutils::inifile::save( file.db(), m );
}

bool
document::load( const QString& filename, adcontrols::ControlMethod::Method& m ) const
{
    QFileInfo fi( filename );

    if ( fi.exists() ) {
        adfs::filesystem file;
        if ( file.mount( filename.toStdWString().c_str() ) ) {
            if ( adutils::inifile::load( file.db(), m ) ) {
                return true;
            }
        }
    }
    return false;
}

void
document::handleTraceMethodChanged()
{
    if ( auto cm = MainWindow::instance()->getControlMethod() ) {
        auto it = cm->find( cm->begin(), cm->end(), admethods::controlmethod::ADTraceMethod::__clsid__ );
        if ( it != cm->end() ) {
            admethods::controlmethod::ADTraceMethod tm;
            it->get( *it, tm );
            set_trace_method( tm );
        }
    }
}

void
document::setAutoZero()
{
    impl_->autoZeroFlag_ = true;
}
