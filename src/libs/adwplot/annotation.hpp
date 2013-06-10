// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <string>

#include <compiler/diagnostic_push.h>
#include <compiler/disable_unused_parameter.h>
#if ! defined Q_MOC_RUN
#include <boost/smart_ptr.hpp>
#endif
#include <compiler/diagnostic_pop.h>

#include <QtCore>

class QwtPlotMarker;

namespace adwplot {

    class Dataplot;

    class Annotation {
    public:
        explicit Annotation( Dataplot&, const std::wstring&, double x = 0.0, double y = 0.0
            , Qt::GlobalColor color = Qt::darkGreen );
        Annotation( const Annotation& );

        void setLabelAlighment( Qt::Alignment );

        inline operator QwtPlotMarker * () { return marker_.get(); }

    private:
        Dataplot * plot_;
        boost::shared_ptr< QwtPlotMarker > marker_;
    };

}



