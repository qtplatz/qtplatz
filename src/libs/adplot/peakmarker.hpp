/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef PEAKMARKER_HPP
#define PEAKMARKER_HPP

#include "adplot_global.hpp"
#include "constants.hpp"
#include <memory>
#include <array>
#include <adcontrols/metric/prefix.hpp>

class QwtPlotMarker;
class QwtPlot;

namespace adcontrols {
    class MSPeakInfoItem; class MassSpectrum;
    class Chromatogram; class Peak;
}

namespace adplot {

    class ADPLOTSHARED_EXPORT PeakMarker : public std::enable_shared_from_this< PeakMarker > {
    public:
        enum idAxis { idPeakLeft, idPeakCenter, idPeakRight, idPeakBase, idPeakThreshold, idPeakTop
                      , idWidthLeft, idWidthRight
                      , numMarkers };

        virtual ~PeakMarker();
        PeakMarker();

        void attach( QwtPlot * );
        void detach();

        void setYAxis( int );
        void setValue( idAxis, double x, double y );

        // MS peak
        void setPeak( const adcontrols::MSPeakInfoItem&
                      , bool isTime = false, adcontrols::metric::prefix pfx = adcontrols::metric::micro );
        void setPeak( const adcontrols::MassSpectrum&, int idx, bool isTime = false, adcontrols::metric::prefix pfx = adcontrols::metric::micro );

        // Chromatograpic peak
        void setPeak( const adcontrols::Peak&
                      , adplot::constants::chromatogram_time_spec spec = adplot::constants::default_chromatogram_time );

        void visible( bool );

        QwtPlotMarker * marker( idAxis );

    private:
        std::array< QwtPlotMarker *, numMarkers > markers_;
    };

}

#endif // PEAKMARKER_HPP
