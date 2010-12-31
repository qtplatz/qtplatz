// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#pragma once

#include <atlbase.h>
#include <atlcom.h>

#include <QAxWidget>

#include "import_sagraphics.h"

namespace adwidgets {
    namespace ui {

        class Dataplot;
        enum ColorIndices;

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
                    short getColorIndex( ::adwidgets::ui::ColorIndices ) const;

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

