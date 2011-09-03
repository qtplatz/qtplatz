// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "adcontrols_global.h"

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSPeakInfoItem {
    public:
        ~MSPeakInfoItem(void);
        MSPeakInfoItem(void);
        MSPeakInfoItem( const MSPeakInfoItem& );
        MSPeakInfoItem( unsigned int peak_index, double mass, double area, double height, double hh, double time );
        double mass() const;
        double area() const;
        double height() const;
        double widthHH() const;
        double time() const;
        unsigned int peak_index() const;
        void peak_index( int );
        unsigned int peak_start_index() const;
        void peak_start_index( unsigned int );
        unsigned int peak_end_index() const;
        void peak_end_index( unsigned int );
        double base_height() const;
        void base_height( double );

    private:
        unsigned int peak_index_;
        unsigned int peak_start_index_;
        unsigned int peak_end_index_;
        double base_;
        double mass_;
        double area_;
        double height_;
        double hh_;
        double time_;
    };

}
