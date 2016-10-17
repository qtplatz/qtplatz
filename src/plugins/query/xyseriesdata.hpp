/**************************************************************************
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#pragma once
#include <qwt_series_data.h>
#include <QPointF>
#include <memory>

namespace acqrscontrols { namespace aqdrv4 { class waveform; } }

class XYSeriesData : public QwtSeriesData< QPointF > {

    XYSeriesData( const XYSeriesData& ) = delete;
    XYSeriesData& operator = ( const XYSeriesData& ) = delete;

public:
    XYSeriesData( std::shared_ptr< const acqrscontrols::aqdrv4::waveform > );

    size_t size() const override;
    QPointF sample( size_t idx ) const override;
    QRectF boundingRect() const override;
    void setRawVertical( bool );
    
private:
    bool rawVertical_;
    std::shared_ptr< const acqrscontrols::aqdrv4::waveform > d_;
    QRectF boundingRect_;
};
