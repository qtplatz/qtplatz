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

QuanMethod::~QuanMethod()
{
}

QuanMethod::QuanMethod() : eq_(idCalibLinear)
                         , polynomialOrder_(2)
                         , isChromatogram_( false )
                         , isISTD_( false )
                         , use_weighting_( false )
                         , weighting_( idWeight_C1 )
                         , levels_(1)
                         , replicates_(1)
                         , use_bracketing_( true )
                         , bracketing_( idBracketStandard )
{
}

QuanMethod::QuanMethod( const QuanMethod& t ) : isChromatogram_( t.isChromatogram_ )
                                              , isISTD_( t.isISTD_ )
                                              , use_weighting_( t.use_weighting_) 
                                              , use_bracketing_( t.use_bracketing_)
                                              , eq_(t.eq_)
                                              , weighting_( t.weighting_ )
                                              , bracketing_( t.bracketing_ )
                                              , levels_( t.levels_)
                                              , replicates_( t.replicates_ )
                                              , polynomialOrder_( t.polynomialOrder_ )
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

uint32_t
QuanMethod::polynomialOrder() const
{
    return polynomialOrder_;
}

void
QuanMethod::polynomialOrder( uint32_t v )
{
    polynomialOrder_ = v;
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
QuanMethod::isWeighting() const
{
    return use_weighting_;
}

void
QuanMethod::isWeighting( bool v )
{
    use_weighting_ = v;
}
        
QuanMethod::CalibWeighting
QuanMethod::weighting() const
{
    return weighting_;
}

void
QuanMethod::weighting( CalibWeighting v )
{
    weighting_ = v;
}

bool
QuanMethod::isBracketing() const
{
    return use_bracketing_;
}

void
QuanMethod::isBracketing( bool v )
{
    use_bracketing_ = v;
}

QuanMethod::Bracketing
QuanMethod::bracketing() const
{
    return bracketing_;
}

void
QuanMethod::bracketing( QuanMethod::Bracketing v )
{
    bracketing_ = v;
}

bool
QuanMethod::isInternalStandard() const
{
    return isISTD_;
}

void
QuanMethod::isInternalStandard( bool v )
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

