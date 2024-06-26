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

#ifndef SPECTROGRAMDATA_HPP
#define SPECTROGRAMDATA_HPP

#pragma once

#include "adplot_global.hpp"
#include <qwt_raster_data.h>
#include <qwt_interval.h>
#include <tuple>

class QRectF;

namespace adplot {

    class ADPLOTSHARED_EXPORT SpectrogramData : public QwtRasterData {
    public:
        SpectrogramData();
        virtual ~SpectrogramData();
        double value( double x, double y ) const override;
        virtual QRectF boundingRect() const;
        virtual bool zoomed( const QRectF& ) { return false; }
        QwtInterval interval( Qt::Axis ) const override; // 6.2.0

        void setInterval( Qt::Axis, QwtInterval&& );
    private:
        std::tuple< QwtInterval, QwtInterval, QwtInterval > interval_;
    };

}

#endif // SPECTROGRAMDATA_HPP
