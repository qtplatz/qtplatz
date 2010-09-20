#include "debug_ui.h"
#include "ui_debug_ui.h"

#include <adportable/configuration.h>
#include <adplugin/orbmanager.h>
#include <acewrapper/nameservice.h>
#include <QtCore/qplugin.h>
#include <adinterface/eventlog_helper.h>
#include <qtwrapper/qstring.h>

#pragma warning(disable:4996)
# include "../../../tofcontroller/tofcontrollerC.h"
# include <adinterface/receiverS.h>
#pragma warning(default:4996)

using namespace adtofms;

namespace adtofms {
    namespace impl {
        
        class TOF2 : public POA_Receiver {
            debug_ui& parent_;
        public:
            ~TOF2();
            TOF2( debug_ui& t );
            TOFInstrument::TofSession_var tof;

            // POA_Receiver
            void message( Receiver::eINSTEVENT msg, CORBA::ULong value );
            void log( const EventLog::LogMessage& );
            void shutdown();
            void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text );
        };

    }
}

debug_ui::debug_ui(QWidget *parent) : IMonitor(parent)
                                    , ui(new Ui::debug_ui)
                                    , pTof_(0)
{
    ui->setupUi(this);
    pTof_ = new impl::TOF2(*this);
    // connect( this, SIGNAL( signal_pushButton_clicked() ), this, SLOT( handle_clicked() ) );
    connect( this, SIGNAL( signal_log_out( const QString ) ), this, SLOT( handle_log_out( const QString ) ) );
}

debug_ui::~debug_ui()
{
    delete ui;
}

void
debug_ui::OnCreate( const adportable::Configuration& c )
{
    config_ = c;
}

void
debug_ui::OnInitialUpdate()
{
    // now, it is safe to access CORBA servant
    if ( adplugin::ORBManager::instance()->init( 0, 0 ) >= 0 ) {

        CORBA::Object_var obj = adplugin::ORBManager::instance()->getObject( L"tofcontroller.session" );

        if ( ! CORBA::is_nil( obj.in() )  ) {

            pTof_->tof = TOFInstrument::TofSession::_narrow( obj );

            if ( ! CORBA::is_nil( pTof_->tof.in() ) ) {
                CORBA::WString_var version_ = pTof_->tof->tof_software_revision();
                pTof_->tof->connect( pTof_->_this(), L"adtofms.monitor_ui" );
            }

        }

    }
}

void
debug_ui::OnUpdate( boost::any& )
{
}

void
debug_ui::OnUpdate( unsigned long lHint )
{
    (void)lHint;
}

void
debug_ui::OnFinalClose()
{
}

void
debug_ui::slot_log_out( const QString text )
{
    emit signal_log_out( text );
}

void
debug_ui::handle_log_out( const QString text )
{
    ui->plainTextEdit->appendPlainText( text );
}

// Q_EXPORT_PLUGIN( monitor_ui )

/////////////////////////////////

using namespace adtofms::impl;

TOF2::~TOF2()
{
}

TOF2::TOF2( debug_ui& t ) : parent_(t)
{
}

void
TOF2::message( Receiver::eINSTEVENT msg, CORBA::ULong value )
{
}

void
TOF2::log( const EventLog::LogMessage& log )
{
    std::wstring text = adinterface::EventLog::LogMessageHelper::toString( log );
    if ( parent_.ui ) {

		QString key = qtwrapper::qstring::copy( log.srcId.in() );
		QString qtext = qtwrapper::qstring::copy( text );
        QString text = key + '\t' + qtext;
        parent_.slot_log_out( text );
    }
}

void
TOF2::shutdown()
{
}

void
TOF2::debug_print( CORBA::Long pri, CORBA::Long cat, const char * text )
{
}

