// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPLOTIMPL_H
#define DATAPLOTIMPL_H

#include <atlbase.h>
#include <atlcom.h>

#include <QAxWidget>

#include "import_sagraphics.h"

namespace adil {
  namespace ui {

    class Dataplot;

    namespace internal {
      namespace win32 {
	__declspec(selectany) _ATL_FUNC_INFO SADP_MouseDown = { CC_STDCALL, VT_EMPTY, 3, { VT_R8, VT_R8, VT_I2 } };
	__declspec(selectany) _ATL_FUNC_INFO SADP_MouseUp   = { CC_STDCALL, VT_EMPTY, 3, { VT_R8, VT_R8, VT_I2 } };
	__declspec(selectany) _ATL_FUNC_INFO SADP_MouseMove = { CC_STDCALL, VT_EMPTY, 3, { VT_R8, VT_R8, VT_I2 } };
	__declspec(selectany) _ATL_FUNC_INFO SADP_Character = { CC_STDCALL, VT_EMPTY, 1, { VT_I4 } };
	__declspec(selectany) _ATL_FUNC_INFO SADP_KeyDown   = { CC_STDCALL, VT_EMPTY, 1, { VT_I4 } };
	__declspec(selectany) _ATL_FUNC_INFO SADP_SetFocus  = { CC_STDCALL, VT_EMPTY, 1, { VT_I4 } };
	__declspec(selectany) _ATL_FUNC_INFO SADP_KillFocus = { CC_STDCALL, VT_EMPTY, 1, { VT_I4 } };
	__declspec(selectany) _ATL_FUNC_INFO SADP_MouseDblClk = { CC_STDCALL, VT_EMPTY, 3, { VT_R8, VT_R8, VT_I2 } };
	
	/******************************************************
	 */
	class DataplotImpl
	  : public QAxWidget
	  , public IDispEventSimpleImpl<100, DataplotImpl, &SAGRAPHICSLib::DIID__ISADataplotEvents>
	  , public IDispEventSimpleImpl<100, DataplotImpl, &SAGRAPHICSLib::DIID__ISADataplotEvents2> { 
	  //Q_OBJECT
	public:
	  ~DataplotImpl();
	  DataplotImpl( Dataplot& parent );
	  bool createControl();
	  SAGRAPHICSLib::ISADataplot* operator -> () { return pi_.p; };
	private:
	  Dataplot& dataplot_;
	  CComPtr<SAGRAPHICSLib::ISADataplot> pi_;
	public:
	  STDMETHOD(OnMouseDown)(double x, double y, short button );
	  STDMETHOD(OnMouseUp)( double x, double y, short Button );
	  STDMETHOD(OnMouseMove)( double x, double y, short Button );
	  STDMETHOD(OnCharacter)( long KeyCode );
	  STDMETHOD(OnKeyDown)( long KeyCode );
	  STDMETHOD(OnSetFocus)( long hWnd );
	  STDMETHOD(OnKillFocus)( long hWnd );
	  STDMETHOD(OnMouseDblClk)(double x, double y, short button );
	  
	  BEGIN_SINK_MAP( DataplotImpl )
	    SINK_ENTRY_INFO(100, SAGRAPHICSLib::DIID__ISADataplotEvents,  1, OnMouseDown,   &SADP_MouseDown)
	    SINK_ENTRY_INFO(100, SAGRAPHICSLib::DIID__ISADataplotEvents,  2, OnMouseUp,     &SADP_MouseUp)
	    SINK_ENTRY_INFO(100, SAGRAPHICSLib::DIID__ISADataplotEvents,  3, OnMouseMove,   &SADP_MouseMove)
	    SINK_ENTRY_INFO(100, SAGRAPHICSLib::DIID__ISADataplotEvents,  4, OnCharacter,   &SADP_Character)
	    SINK_ENTRY_INFO(100, SAGRAPHICSLib::DIID__ISADataplotEvents,  5, OnSetFocus,    &SADP_SetFocus)
	    SINK_ENTRY_INFO(100, SAGRAPHICSLib::DIID__ISADataplotEvents,  6, OnKillFocus,   &SADP_KillFocus)
	    SINK_ENTRY_INFO(100, SAGRAPHICSLib::DIID__ISADataplotEvents2, 1, OnMouseDblClk, &SADP_MouseDblClk)
	    END_SINK_MAP()
	    };
	/******************************************************/
      }
    }
  }
}


#endif // DATAPLOTIMPL_H
