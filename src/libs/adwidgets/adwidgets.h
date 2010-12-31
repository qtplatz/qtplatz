// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

namespace adwidgets {
    namespace ui {
        
        enum CursorStyle {
            CS_None = 0,
            CS_HorizontalLine = 1,
            CS_VerticalLine = 2,
            CS_Box = 3,
            CS_Plus = 4,
            CS_Crosshair = 5,
            CS_Line = 6,
            CS_TrackTrace = 7,
            CS_TrackLine = 8,
            CS_TrackMarker = 9,
            CS_TrackCrosshair = 10,
            CS_VerticalBar = 11
        };
        
        enum YAxis {
            Y1 = 0,
            Y2 = 1
        };

        enum LineStyle {
            LS_Solid = 0,
            LS_Dash = 1,
            LS_Dot = 2,
            LS_DashDot = 3
        };
        
        enum MarkerStyle {
            MS_None = 0,
            MS_Point = 1,
            MS_Circle = 2,
            MS_Dot = 3,
            MS_Box = 4,
            MS_Square = 5,
            MS_X = 6,
            MS_Plus = 7,
            MS_Diamond = 8,
            MS_FilledDiamond = 9
        };
        
        enum FractionStyle {
            FS_Tick = 0,
            FS_Bar = 1,
            FS_Line = 2,
            FS_Fill = 3,
            FS_LineFill = 4
        };
        
        enum AnnotationStyle {
            AS_None = 0,
            AS_Above = 1,
            AS_Below = 2
        };
        
        enum TraceStyle {
            TS_Connected = 0,
            TS_Stick = 1,
            TS_Point = 2
        };
        
        enum TickComputeStyle {
            TCS_Auto = 0,
            TCS_Incremental = 1,
            TCS_Manual = 2
        };
        
        enum TickStyle {
            TS_None = 0,
            TS_Outside = 1,
            TS_Inside = 2,
            TS_Both = 3
        };
        
        enum TickLabelFormat {
            TLF_Decimal = 0,
            TLF_Scientific = 1
        };
        
        enum Orientation {
            O_Horizontal = 0,
            O_Vertical = 1
        };

        enum ScaleStyle {
            SS_Linear = 0,
            SS_Log10 = 1,
            SS_LogE = 2,
            SS_GeometricProgression = 3,
            SS_UserSpecified = 4
        };

        enum PlotStyle {
            PS_Line = 0,
            PS_Filled = 1
        };
        
        enum ColorTable {
            CT_Display = 0,
            CT_Printer = 1
        };
        
        enum PeakMarkerStyle {
            PM_None = 0,
            PM_UpStick = 1,
            PM_DownStick = 2,
            PM_CrossStick = 3,
            PM_UpTriangle = 4,
            PM_DownTriangle = 5,
            PM_UpTriangleFilled = 6,
            PM_DownTriangleFilled = 7,
            PM_UpWedgeFilled = 8,
            PM_DownWedgeFilled = 9
        };
        
        enum LegendPosition {
            LP_TopLeft = 0,
            LP_TopRight = 1,
            LP_BottomRight = 2,
            LP_BottomLeft = 3
        };
        
    }
}
