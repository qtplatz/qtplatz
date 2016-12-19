/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#pragma once

#include <qwt_plot.h>
#include <memory>

namespace adcontrols { class MappedImage; }

namespace counting2d {

    class SpectrogramPlot : public QwtPlot {
        Q_OBJECT
    public:
        ~SpectrogramPlot();
        explicit SpectrogramPlot(QWidget *parent = 0);

        void setData( std::shared_ptr< adcontrols::MappedImage >&& );
        void setCellSelection( const QRect& );
        void setCellSelectionEnabled( bool );
        void setAxisZMax( double max );
        double z() const;
        
    signals:
        void cellSelected( const QRect& );
        
    private:
        class impl;
        impl * impl_;
    };

}

