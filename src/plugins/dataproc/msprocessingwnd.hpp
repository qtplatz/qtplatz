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

#ifndef MSPROCESSINGWND_H
#define MSPROCESSINGWND_H

#include <QWidget>
#include <boost/smart_ptr.hpp>
#include <map>
#include <boost/variant.hpp>

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
	class PeakResult;
}

namespace portfolio {
    class Folium;
}

namespace dataproc {

    class Dataprocessor;

    namespace internal {

        class MSProcessingWndImpl;

        class MSProcessingWnd : public QWidget {
            Q_OBJECT
        public:
            explicit MSProcessingWnd(QWidget *parent = 0);

            void init();

            void draw1( boost::shared_ptr< adcontrols::MassSpectrum >& );
            void draw2( boost::shared_ptr< adcontrols::MassSpectrum >& );
            void draw( boost::shared_ptr< adcontrols::Chromatogram >& );
			void draw( boost::shared_ptr< adcontrols::PeakResult >& );
      
        signals:
      
        public slots:
            void handleSessionAdded( Dataprocessor* );
            void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );

        private slots:
			void onCustomMenuOnProcessedSpectrum( const QPoint& );

			void selectedOnChromatogram( const QPointF& );
			void selectedOnChromatogram( const QRectF& );
			void selectedOnProfile( const QPointF& );
			void selectedOnProcessed( const QPointF& );

        private:
            size_t drawIdx1_;
            size_t drawIdx2_;
            boost::shared_ptr<MSProcessingWndImpl> pImpl_;
            std::wstring idActiveFolium_;
        };

    }
}


#endif // MSPROCESSINGWND_H
