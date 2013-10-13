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

#ifndef MSCALIBSPECTRAWND_HPP
#define MSCALIBSPECTRAWND_HPP

#include <QWidget>
#include <portfolio/folium.hpp>
#include <boost/tuple/tuple.hpp>
#include <memory>
#include <tuple>
#include <map>

class QSplitter;
class QwtPlotMarker;
class QwtPlotCurve;

namespace adcontrols {
    class MassSpectrum;
    class MSCalibrateResult;
    class MSAssignedMasses;
    class ProcessMethod;
}
namespace adportable {  class Configuration; }
namespace adwplot { class SpectrumWidget; class Dataplot; }

namespace dataproc {

    class Dataprocessor;

    namespace internal { class SeriesData; }

    class MSCalibSpectraWnd : public QWidget {
        Q_OBJECT
    public:
        MSCalibSpectraWnd( QWidget * parent = 0 );
		~MSCalibSpectraWnd();
    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handleCheckStateChanged( Dataprocessor*, portfolio::Folium&, bool );

    signals:
        void fireSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& );

    private slots:
        void handleSelSummary( size_t idx, size_t fcn );
        void handleValueChanged();
        void handle_reassign_mass_requested();
        void handle_recalibration_requested();
        void handle_apply_calibration_to_dataset();
        void handle_apply_calibration_to_default();
        void handleAxisChanged( int );

    private:
        void init();
        void applyAssigned( const adcontrols::MSAssignedMasses&, const portfolio::Folium& );

        typedef std::pair< std::shared_ptr< adcontrols::MSCalibrateResult >, std::shared_ptr< adcontrols::MassSpectrum > > result_type;
        int populate( Dataprocessor *, portfolio::Folder& );
        void doCalibration( adcontrols::MassSpectrum& centroid, adcontrols::MSCalibrateResult&, const adcontrols::MSAssignedMasses& assigned );

        std::vector< std::shared_ptr< adwplot::SpectrumWidget > > wndSpectra_;
        std::vector< std::shared_ptr< QwtPlotMarker > > markers_;
        std::shared_ptr< QwtPlotMarker > time_length_marker_;

        std::vector< portfolio::Folium > folio_;

        portfolio::Folium folium_;
        QWidget * wndCalibSummary_;
        QSplitter * wndSplitter_;
        std::shared_ptr< adwplot::Dataplot > dplot_;
        int axis_;
		std::vector< std::tuple< std::wstring, std::shared_ptr<adcontrols::MassSpectrum>, std::shared_ptr<adcontrols::MSCalibrateResult> > > results_;
        std::map< std::wstring, std::shared_ptr< internal::SeriesData > > plotData_;
        std::map< std::wstring, std::shared_ptr< QwtPlotCurve > > plotCurves_;

        std::vector< std::shared_ptr< adcontrols::MSAssignedMasses > > assignedResults_;
        bool readCalibSummary( adcontrols::MSAssignedMasses& );
        void replotSpectra();
        void replotLengthTime();
        void plot( internal::SeriesData&, int id );
        void plotTimeMarker( double t );
    };

}

#endif // MSCALIBSPECTRAWND_HPP
