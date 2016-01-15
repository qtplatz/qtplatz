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

#ifndef MSCALIBSPECTRAWND_HPP
#define MSCALIBSPECTRAWND_HPP

#include <QWidget>
#if ! defined Q_MOC_RUN
#include <adportfolio/folium.hpp>
#endif
#include <boost/tuple/tuple.hpp>
#include <memory>
#include <tuple>
#include <map>
#include <deque>
#include <array>

class QSplitter;
class QwtPlotMarker;
class QwtPlotCurve;
class QwtPlot;
class QwtText;

namespace adcontrols {
    class MassSpectrum;
    class MSCalibrateResult;
    class MSAssignedMasses;
    class ProcessMethod;
    class MSPeaks;
    enum hor_axis;
}

namespace adportable {  class Configuration; }
namespace adplot { class SpectrumWidget; class plot; }
class QPrinter;

namespace dataproc {

    class Dataprocessor;
    template<int> struct Fitter;

    namespace internal { class SeriesData; }

    class MSCalibSpectraWnd : public QWidget {
        Q_OBJECT
    public:
        MSCalibSpectraWnd( QWidget * parent = 0 );
		~MSCalibSpectraWnd();

    public slots:
        void handleSessionAdded( Dataprocessor* );
        void handleProcessed( Dataprocessor*, portfolio::Folium& );
        void handleSelectionChanged( Dataprocessor*, portfolio::Folium& );
        void handleApplyMethod( const adcontrols::ProcessMethod& );
        void handleCheckStateChanged( Dataprocessor*, portfolio::Folium&, bool );
        void handlePrintCurrentView( const QString& outpdf );
        void handleAxisChanged( adcontrols::hor_axis );

    signals:
        void onSetData( const adcontrols::MSCalibrateResult&, const adcontrols::MassSpectrum& );
        void onPrint( QPrinter&, QPainter& );

    private slots:
        void handleSelSummary( size_t idx, size_t fcn );
        void handleValueChanged();
        void handle_reassign_mass_requested();
        void handle_recalibration_requested();
        void handle_apply_calibration_to_dataset();
        void handle_apply_calibration_to_all();
        void handle_apply_calibration_to_default();

        //---
        void handle_add_selection_to_peak_table( const adcontrols::MSPeaks& );

    private:
        void init();
        void applyAssigned( const adcontrols::MSAssignedMasses&, const portfolio::Folium& );

        int populate( Dataprocessor *, portfolio::Folder& );
        void doCalibration( adcontrols::MassSpectrum& centroid, adcontrols::MSCalibrateResult&, const adcontrols::MSAssignedMasses& assigned );

        QString fullpath_;

        // selSpectra member holds last selected spectra regardless of isChecked state (for spectral comperison on display)
        std::deque< std::pair< std::wstring, std::weak_ptr< adcontrols::MassSpectrum > > > selSpectra_;
        std::wstring selFormula_;

        // results member holds all isChecked flagged data
        typedef std::tuple< std::wstring, std::weak_ptr<adcontrols::MassSpectrum>, std::weak_ptr<adcontrols::MSCalibrateResult> > result_type;
		std::vector< result_type > results_;
        //std::vector< size_t > number_of_segments_;

        // marged is convenient marged spectra for easy review all identified peaks
        std::shared_ptr< adcontrols::MassSpectrum > margedSpectrum_;
        std::shared_ptr< adcontrols::MSCalibrateResult > margedCalibResult_;

        QWidget * wndCalibSummary_;
        QSplitter * wndSplitter_;
        int axis_;

        std::vector< std::shared_ptr< adplot::SpectrumWidget > > wndSpectra_;
        std::vector< std::shared_ptr< QwtPlotMarker > > markers_;
        
        enum { idPlotSqrtMassTime, idPlotLengthTime, idPlotOrbit, idPlotInjection };  // plot (left & right)

        std::vector< std::shared_ptr< adplot::plot > > plots_;
        std::vector< std::shared_ptr< QwtPlotCurve > > curves_;

        bool readCalibSummary( adcontrols::MSAssignedMasses& );
        void replotSpectra();
        void flight_length_regression();

        void plot_time_corrected_calibrant( int id, int mode, const Fitter<2>&, adplot::plot& );
        void plot_length_time( int id, const std::wstring& formula, const Fitter<2>&, adplot::plot& );
        void plot_orbital_sector_calibration( int id, const Fitter<2>&, adplot::plot& plot );
        void plot_injection_sector_calibration( int id, const Fitter<2>&, adplot::plot& plot );
        void plot_fitter( int id, const QwtText&, const Fitter<2>&, adplot::plot&, int xAxis = 0, int yAxis = 0);

        void generate_marged_result( Dataprocessor * );
    };

}

#endif // MSCALIBSPECTRAWND_HPP
