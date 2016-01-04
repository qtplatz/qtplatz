// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
	class MSPeakInfo;
    class Targeting;
}

namespace portfolio {
    class Folium;
}

namespace boost { namespace uuids { struct uuid; } }

namespace dataproc {

    class Dataprocessor;

    class MSProcessingWndImpl;

    class MSProcessingWnd : public QWidget {
        Q_OBJECT
        public:
        explicit MSProcessingWnd(QWidget *parent = 0);

        void draw_profile( const std::wstring& id, std::shared_ptr< adcontrols::MassSpectrum >& );
        void draw2( std::shared_ptr< adcontrols::MassSpectrum >& );
        void draw( std::shared_ptr< adcontrols::Chromatogram >&, int idx );
        void draw( std::shared_ptr< adcontrols::PeakResult >& );

        void idSpectrumFolium( const std::wstring& );
        void idChromatogramFolium( const std::wstring& );
        void handleCheckStateChanged( Dataprocessor* processor, portfolio::Folium& folium, bool isChecked );

    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleProcessed( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handlePrintCurrentView( const QString& outpdf );
        void handleAxisChanged( int );
        void handleFoliumDataChanged( const QString& );

        // slot for MSPeakTable
        void handleCurrentChanged( int idx, int fcn );
        void handleFormulaChanged( int idx, int fcn );
        void handleLockMass( const QVector< QPair<int, int> >& );
        void handleDataMayChanged();

    private slots:
        void handleCustomMenuOnProcessedSpectrum( const QPoint& );

        void selectedOnChromatogram( const QRectF& );
		void selectedOnProfile( const QRectF& );
		void selectedOnProcessed( const QRectF& );
		void selectedOnPowerPlot( const QRectF& );
        void handleZoomedOnSpectrum( const QRectF& );

    private:
        size_t drawIdx1_;
        size_t drawIdx2_;
        std::shared_ptr<MSProcessingWndImpl> pImpl_;
        std::pair< std::wstring, std::weak_ptr< adcontrols::MassSpectrum > > pProcessedSpectrum_;
        std::pair< std::wstring, std::weak_ptr< adcontrols::MassSpectrum > > pProfileSpectrum_;
        std::pair< std::wstring, std::weak_ptr< adcontrols::MSPeakInfo > > pkinfo_;
        std::pair< std::wstring, std::weak_ptr< adcontrols::Targeting > > targeting_;

        std::wstring idActiveFolium_;
        std::wstring idChromatogramFolium_;
        std::wstring idSpectrumFolium_;
        int axis_;
        bool assign_masses_to_profile( const std::pair< boost::uuids::uuid, std::string >& );
        bool assign_masses_to_profile( );
        double correct_baseline();
        void init();
        void draw1();
        double compute_rms( double, double );
        std::pair<double, double> compute_minmax( double, double );
        bool power_spectrum( const adcontrols::MassSpectrum&, std::vector<double>& x, std::vector<double>& y
                             , const std::pair<size_t, size_t>&, double& dc, double& nyquist );

        // from menu
        void frequency_analysis();
        void save_image_file();

    signals:
        void dataChanged( const QString& foliumGuid, const QString& attrGuid, int idx, int fcn );

    };

}


#endif // MSPROCESSINGWND_H
