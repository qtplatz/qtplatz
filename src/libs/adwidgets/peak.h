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

namespace SAGRAPHICSLib {
    struct ISADPPeak;
}

namespace adwidgets {
    namespace ui {
        
        class Peak  {
        public:
            ~Peak();
            Peak( SAGRAPHICSLib::ISADPPeak * pi = 0 );
            Peak( const Peak& );

            bool visible() const;
            void visible( bool );

            double startX() const;
            void startX( double );

            double startY() const;
            void startY( double );
            
            double endX() const;
            void endX( double );
            
            double endY() const;
            void endY( double );
            
            double centreX() const;
            void centreX( double );
            
            double centreY() const;
            void centreY( double );
            
            double baselineStartY() const;
            void baselineStartY( double );
            
            double baselineEndY() const;
            void baselineEndY( double );
            
            double baselineCentreY() const;
            void baselineCentreY( double );
            
            bool drawBaseline() const;
            void drawBaseline( bool );

            bool drawBaselineCentre() const;
            void drawBaselineCentre( bool );

            bool peakFill() const;
            void peakFill( bool );

            short colorIndex() const;
            void colorIndex( short );

            bool marked() const;
            void marked( bool );

            short fillStyle();
            void fillStyle( short );

            short fillColorIndex();
            void fillColorIndex( short );

            enum PeakMarkerStyle startMarkerStyle() const;
            void startMarkerStyel( PeakMarkerStyle );

            enum PeakMarkerStyle endMarkerStyle() const;
            void endMarkerStyle( PeakMarkerStyle );

            enum PeakMarkerStyle centreMarkerStyle() const;
            void centreMarkerStyle( PeakMarkerStyle );

            short startMarkerColorIndex() const;
            void startMarkerColorIndex( short );

            short endMarkerColorIndex() const;
            void endMarkerColorIndex( short );

            short centreMarkerColorIndex() const;
            void centreMarkerColroIndex( short );
            
        private:
            SAGRAPHICSLib::ISADPPeak * pi_;
        };
        
    }
}

