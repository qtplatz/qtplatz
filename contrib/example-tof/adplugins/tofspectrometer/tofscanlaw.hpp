/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef tofSCANLAW_HPP
#define tofSCANLAW_HPP

#include <adcontrols/massspectrometer.hpp>

namespace tofspectrometer {


    class tofScanLaw : public adcontrols::MassSpectrometer::ScanLaw {
    public:
        tofScanLaw();
		// implements ScanLaw
		virtual double getMass( double secs, int type ) const;
		virtual double getTime( double mass, int type ) const;
		virtual double getMass( double secs, double fLength ) const;
		virtual double getTime( double mass, double fLength ) const;
    };

}
#endif // tofSCANLAW_HPP
