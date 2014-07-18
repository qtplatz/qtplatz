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

#include "quanmethod.hpp"

using namespace adcontrols;

QuanMethod::QuanMethod() : eq_(idCalibLinear)
                         , isChromatogram_( false )
                         , isISTD_( false )
                         , use_waiting_( false )
                         , waiting_method_( idWait_C1 )
                         , levels_(1)
                         , replicates_(1)
{
}

QuanMethod::CalibEq
QuanMethod::equation() const
{
    return eq_;
}

void
QuanMethod::equation( CalibEq v )
{
    eq_ = v;
}

bool
QuanMethod::isChromatogram() const
{
    return isChromatogram_;
}

void
QuanMethod::isChromatogram( bool v )
{
    isChromatogram_ = v;
}

bool
QuanMethod::isWaiting() const
{
    return use_waiting_;
}

void
QuanMethod::isWaiting( bool v )
{
    use_waiting_ = v;
}
        
QuanMethod::CalibWaiting
QuanMethod::waiting() const
{
    return waiting_method_;
}

void
QuanMethod::waiting( CalibWaiting v )
{
    waiting_method_ = v;
}

bool
QuanMethod::ISTD() const
{
    return isISTD_;
}

void
QuanMethod::ISTD( bool v )
{
    isISTD_ = v;
}

uint32_t
QuanMethod::levels() const
{
    return levels_;
}

void
QuanMethod::levels( uint32_t v )
{
    levels_ = v;
}

uint32_t
QuanMethod::replicates() const
{
    return replicates_;
}

void
QuanMethod::replicates( uint32_t v )
{
    replicates_ = v;
}

