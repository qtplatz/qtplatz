//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "monitor_ui.h"
#include "ui_form.h"
#include "treemodel.h"
#include "../../../tofcontroller/tofcontrollerC.h"
#include <adportable/configuration.h>
#include <adplugin/orbmanager.h>
#include <acewrapper/nameservice.h>
#include <adinterface/receiverS.h>
#include <QtCore/qplugin.h>
#include <adinterface/eventlog_helper.h>
#include <qtwrapper/qstring.h>

using namespace adtofms;

namespace adtofms {
    namespace impl {
        
        class TOF : public POA_Receiver {
            monitor_ui& parent_;
        public:
            ~TOF();
            TOF( monitor_ui& t );
            TOFInstrument::TofSession_var tof;

            // POA_Receiver
            void message( Receiver::eINSTEVENT msg, CORBA::ULong value );
            void log( const EventLog::LogMessage& );
            void shutdown();
            void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text );
        };

    }
}

monitor_ui::monitor_ui(QWidget *parent) : IMonitor(parent)
                                        , ui(new Ui::Form)
                                        , pTof_(0)
                                        , treeModel_( new TreeModel )
{
    ui->setupUi(this);
    pTof_ = new impl::TOF(*this);
    connect( this, SIGNAL( signal_pushButton_clicked() ), this, SLOT( handle_clicked() ) );

    connect( this, SIGNAL( signal_log(QString, QString) ), this, SLOT( handle_log( QString, QString ) ) );

    ui->treeView->setModel( treeModel_ );
    for ( int column = 0; column < treeModel_->columnCount(); ++column )
        ui->treeView->resizeColumnToContents( column );
}

monitor_ui::~monitor_ui()
{
    delete ui;
    delete treeModel_;
}

void
monitor_ui::OnCreate( const adportable::Configuration& c )
{
    config_ = c;
}

void
monitor_ui::OnInitialUpdate()
{
    // now, it is safe to access CORBA servant
    if ( adplugin::ORBManager::instance()->init( 0, 0 ) >= 0 ) {

        CORBA::Object_var obj = adplugin::ORBManager::instance()->getObject( L"tofcontroller.manager" );

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
monitor_ui::OnUpdate( boost::any& )
{
}

void
monitor_ui::OnUpdate( unsigned long lHint )
{
    (void)lHint;
}

void
monitor_ui::OnFinalClose()
{
}

void
monitor_ui::handle_clicked()
{
    pTof_->tof->tof_debug( L"monitor_ui::handle_clicked()", L"monitor_ui" );
}

void
monitor_ui::handle_log( QString qtext, QString key )
{
    TreeModel& model = *treeModel_;

    int row = model.findParent( key );
    if ( row < 0 ) {
        row = model.rowCount();
        model.insertRow( row );
        QModelIndex parentIndex = model.index( row, 0 );
        model.setData( parentIndex, key );
        model.setData( model.index( row, 2 ), qtext );
    } else {
        QModelIndex index = model.index( row, 0 );
        int childRow = model.rowCount( index );
        model.insertRow( childRow, index );
        model.setData( model.index( childRow, 0, index ), key );
        model.setData( model.index( childRow, 2, index ), qtext );
    }
}

void
monitor_ui::slot_log( QString text, QString key )
{
    emit signal_log( text, key );
}

// call from ui
void
monitor_ui::on_pushButton_clicked()
{
    emit signal_pushButton_clicked(); 
}

// Q_EXPORT_PLUGIN( monitor_ui )

/////////////////////////////////

using namespace adtofms::impl;

TOF::~TOF()
{
}

TOF::TOF( monitor_ui& t ) : parent_(t)
{
}

void
TOF::message( Receiver::eINSTEVENT msg, CORBA::ULong value )
{
}

void
TOF::log( const EventLog::LogMessage& log )
{
    std::wstring text = adinterface::EventLog::LogMessageHelper::toString( log );
    if ( parent_.ui ) {

		QString key = qtwrapper::qstring::copy( log.srcId.in() );
        if ( key.isEmpty() )
            return;
		QString qtext = qtwrapper::qstring::copy( text );

        parent_.slot_log( qtext, key );
        /*
		TreeModel& model = *treeModel_;

		int row = model.findParent( key );
		if ( row < 0 ) {
			row = model.rowCount();
            model.insertRow( row );
            QModelIndex parentIndex = model.index( row, 0 );
            model.setData( parentIndex, key );
			model.setData( model.index( row, 2 ), qtext );
		} else {
			QModelIndex index = model.index( row, 0 );
			int childRow = model.rowCount( index );
			model.insertRow( childRow, index );
			model.setData( model.index( childRow, 0, index ), key );
			model.setData( model.index( childRow, 2, index ), qtext );
		}
        */
    }
}

void
TOF::shutdown()
{
}

void
TOF::debug_print( CORBA::Long pri, CORBA::Long cat, const char * text )
{
}

