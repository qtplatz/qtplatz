//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "device_hvcontroller.h"
#include <ace/Message_Block.h>
#include <acewrapper/inputcdr.h>
#include <acewrapper/outputcdr.h>
#include "../tofcontroller/tofcontrollerC.h"
#include "../tofcontroller/constants.h"
#include "devicefacade.h"
#include "constants.h"
#include "./reactor_thread.h"
#include <ace/Reactor.h>

using namespace device_emulator;

device_hvcontroller::device_hvcontroller(void)
{
	// adConfig_.reset( new TOFInstrument::ADConfigurations() );
    data_.reset( new AnalyzerDeviceData() );
    data_->model = "device_htcontroller";
    data_->hardware_rev = "hardware 2010/08/21";
    data_->firmware_rev = "firmware 2010/08/21";
    data_->serailnumber = "20100821";
    data_->positive_polarity = 0;
    data_->ionguide_bias_voltage = 0;
    data_->ionguide_rf_voltage = 0;
    data_->orifice1_voltage = 0;
    data_->orifice2_voltage = 0;
    data_->orifice4_voltage = 0;
    data_->focus_lens_voltage = 0;
    data_->left_right_voltage = 0;
    data_->quad_lens_voltage = 0;
    data_->pusher_voltage = 0;
    data_->pulling_voltage = 0;
    data_->supress_voltage = 0;
    data_->pushbias_voltage = 0;
    data_->mcp_voltage = 0;
    data_->accel_voltage = 0;  // digital value
}

device_hvcontroller::~device_hvcontroller(void)
{
    if ( state_ > device_state::state_off )
        deactivate();
}

device_hvcontroller::device_hvcontroller( const device_hvcontroller& t ) : device_state( t )
{
	data_ = t.data_;
}

void
device_hvcontroller::activate()
{
    // state_ = device_state::state_initializing;
    doit( device_state::command_initialize );
    ACE_Reactor * reactor = acewrapper::singleton::ReactorThread::instance()->get_reactor();
    reactor->schedule_timer( this, 0, ACE_Time_Value(1), ACE_Time_Value(1) );
}

void
device_hvcontroller::deactivate()
{
    ACE_Reactor * reactor = acewrapper::singleton::ReactorThread::instance()->get_reactor();
    reactor->cancel_timer( this );
    doit( device_state::command_off );
}

bool
device_hvcontroller::instruct_handle_data( ACE_InputCDR& cdr, unsigned long cmdId )
{
	if ( cmdId == tofcontroller::constants::SESSION_SENDTO_DEVICE ) {
       unsigned long clsId;
	   acewrapper::InputCDR in( cdr );
       in >> clsId;
	   if ( clsId == TOFConstants::ClassID_AnalyzerDeviceData ) {
           AnalyzerDeviceData data;
           if ( copyIn( cdr, data ) ) {
               *data_ = data;
			   return true;
           }
	   }
    } else if ( cmdId == tofcontroller::constants::SESSION_QUERY_DEVICE ) {
        unsigned long clsId;
        acewrapper::InputCDR in( cdr );
        do {
            in >> clsId;
            if ( clsId == TOFConstants::ClassID_AnalyzerDeviceData ) {
                ACE_OutputCDR cdr;
                cdr.write_ulong( clsId );
                if ( copyOut( cdr, *data_ ) ) {
                    ACE_Message_Block * mb = cdr.begin()->duplicate();
                    mb->msg_type( constants::MB_CLASS_TO_CONTROLLER );
                    singleton::device_facade::instance()->putq( mb );
                }
            }
        } while (clsId != TOFConstants::EOR );
    }

	return false;
}

bool
device_hvcontroller::instruct_copy_data( ACE_OutputCDR& out, ACE_InputCDR& in, unsigned long clsid)
{
    acewrapper::OutputCDR cdr( out );
    cdr << clsid;
    if ( clsid == TOFConstants::ClassID_AnalyzerDeviceData ) {
        return copyOut( cdr, *data_ );
    }
    return false;
}

///////////
bool
device_hvcontroller::copyIn( ACE_InputCDR& cdr, AnalyzerDeviceData& data )
{
	acewrapper::InputCDR in(cdr);
    in >> data.model;
    in >> data.hardware_rev;
    in >> data.firmware_rev;
    in >> data.serailnumber;
	in >> data.positive_polarity;
    in >> data.ionguide_bias_voltage;

    in >> data.ionguide_rf_voltage;
    in >> data.orifice1_voltage;
    in >> data.orifice2_voltage;
    in >> data.orifice4_voltage;
    in >> data.focus_lens_voltage;
    in >> data.left_right_voltage;
    in >> data.quad_lens_voltage;
    in >> data.pusher_voltage;
    in >> data.pulling_voltage;
    in >> data.supress_voltage;
    in >> data.pushbias_voltage;
    in >> data.mcp_voltage;
    in >> data.accel_voltage;  // digital value
	return true;
}

///////////
bool
device_hvcontroller::copyOut( ACE_OutputCDR& cdr, const AnalyzerDeviceData& data )
{
    acewrapper::OutputCDR out(cdr);
    out << data.model;
    out << data.hardware_rev;
    out << data.firmware_rev;
    out << data.serailnumber;
    out << data.positive_polarity;
    out << data.ionguide_bias_voltage;

    out << data.ionguide_rf_voltage;
    out << data.orifice1_voltage;
    out << data.orifice2_voltage;
    out << data.orifice4_voltage;
    out << data.focus_lens_voltage;
    out << data.left_right_voltage;
    out << data.quad_lens_voltage;
    out << data.pusher_voltage;
    out << data.pulling_voltage;
    out << data.supress_voltage;
    out << data.pushbias_voltage;
    out << data.mcp_voltage;
    out << data.accel_voltage;  // digital value
	return true;
}

int
device_hvcontroller::handle_timeout( const ACE_Time_Value&, const void * )
{
    if ( state() <= device_state::state_initializing )
        doit( device_state::command_stop );

    if ( data_ ) {
        data_->positive_polarity++;
        data_->ionguide_bias_voltage++;
        data_->ionguide_rf_voltage++;
        data_->orifice1_voltage++;
        data_->orifice2_voltage++;
        data_->orifice4_voltage++;
        data_->focus_lens_voltage++;
        data_->left_right_voltage++;
        data_->quad_lens_voltage++;
        data_->pusher_voltage++;
        data_->pulling_voltage++;
        data_->supress_voltage++;
        data_->pushbias_voltage++;
        data_->mcp_voltage++;
        data_->accel_voltage++;  // digital value
    }
    return 0;
}


AnalyzerDeviceData::AnalyzerDeviceData( const AnalyzerDeviceData& t )
{
    operator=(t);
}

void
AnalyzerDeviceData::operator = ( const AnalyzerDeviceData& t )
{
    //std::string model;
    //std::string hardware_rev;
    //std::string firmware_rev;
    //std::string serailnumber;

    positive_polarity   = t.positive_polarity;
    ionguide_bias_voltage = t.ionguide_bias_voltage;
    ionguide_rf_voltage = t.ionguide_rf_voltage;
    orifice1_voltage = t.orifice1_voltage;
    orifice2_voltage = t.orifice2_voltage;
    orifice4_voltage = t.orifice4_voltage;
    focus_lens_voltage = t.focus_lens_voltage;
    left_right_voltage = t.left_right_voltage;
    quad_lens_voltage = t.quad_lens_voltage;
    pusher_voltage = t.pusher_voltage;
    pulling_voltage = t.pulling_voltage;
    supress_voltage = t.supress_voltage;
    pushbias_voltage = t.pushbias_voltage;
    mcp_voltage = t.mcp_voltage;
    accel_voltage = t.accel_voltage;
}