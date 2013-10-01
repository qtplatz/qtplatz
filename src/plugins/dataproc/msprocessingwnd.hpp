// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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
#if ! defined Q_MOC_RUN
#include <boost/variant.hpp>
#endif
#include <map>
#include <memory>

namespace adcontrols {
    class MassSpectrum;
    class Chromatogram;
	class PeakResult;
    class ProcessMethod;
}

namespace portfolio {
    class Folium;
}

namespace dataproc {

    class Dataprocessor;

    class MSProcessingWndImpl;

    class MSProcessingWnd : public QWidget {
        Q_OBJECT
        public:
        explicit MSProcessingWnd(QWidget *parent = 0);

        void init();

        void draw1( std::shared_ptr< adcontrols::MassSpectrum >& );
        void draw2( std::shared_ptr< adcontrols::MassSpectrum >& );
        void draw( std::shared_ptr< adcontrols::Chromatogram >& );
        void draw( std::shared_ptr< adcontrols::PeakResult >& );

        void idSpectrumFolium( const std::wstring& );
        void idChromatogramFolium( const std::wstring& );

    signals:
      
    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handlePrintCurrentView( const QString& outpdf );

    private slots:
        void handleCustomMenuOnProcessedSpectrum( const QPoint& );

        void selectedOnChromatogram( const QPointF& );
        void selectedOnChromatogram( const QRectF& );
        void selectedOnProfile( const QPointF& );
		void selectedOnProfile( const QRectF& );
        void selectedOnProcessed( const QPointF& );
		void selectedOnProcessed( const QRectF& );
        void handleAxisChanged( int );

    private:
        size_t drawIdx1_;
        size_t drawIdx2_;
        std::shared_ptr<MSProcessingWndImpl> pImpl_;
        // std::shared_ptr< adcontrols::MassSpectrum > pProcessedSpectrum_;
        std::weak_ptr< adcontrols::MassSpectrum > pProcessedSpectrum_;
        std::weak_ptr< adcontrols::MassSpectrum > pProfileSpectrum_;

        std::wstring idActiveFolium_;
        std::wstring idChromatogramFolium_;
        std::wstring idSpectrumFolium_;
        int axis_;
    };

}


#endif // MSPROCESSINGWND_H
