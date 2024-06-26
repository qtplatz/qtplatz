/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef QUANSVGPLOT_HPP
#define QUANSVGPLOT_HPP

#include <QByteArray>
#include <quanpublisher.hpp>

namespace quan {

    class QuanPlotData;

    class QuanSvgPlot {
    public:
        QuanSvgPlot();
        bool plot( const QuanPlotData&, size_t idx, int fcn, const std::string& );
        bool plot( const QuanPlotData&, size_t idx, int fcn, const std::string&, const std::pair<double,double>& range );
        bool plot( const QuanPublisher::resp_data&, const QuanPublisher::calib_curve& );
        const char * data() const { return svg_.data(); }
        size_t size() const { return svg_.size(); }
    private:
        bool plot_chromatogram( const QuanPlotData&, size_t idx, int fcn, const std::string& );
        bool plot_spectrum( const QuanPlotData&, size_t idx, int fcn, const std::string&, const std::pair<double,double>& range );

        QByteArray svg_;
    };

}

#endif // QUANSVGPLOT_HPP
