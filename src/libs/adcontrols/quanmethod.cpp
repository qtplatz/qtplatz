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

QuanMethod::QuanMethod() : isCounting_( true )
                         , isChromatogram_( false )
                         , isISTD_( false )
                         , use_weighting_( false )
                         , use_bracketing_( true )
                         , eq_(idCalibLinear)
                         , weighting_( idWeight_C1 )
                         , bracketing_( idBracketStandard )                           
                         , polynomialOrder_( 1 )
                         , debug_level_( 0 )                           
                         , levels_( 1 )
                         , replicates_( 1 )
                         , save_on_datasource_( false )
{
}

QuanMethod::QuanMethod( const QuanMethod& t ) : ident_( t.ident_ )
                                              , isCounting_( t.isCounting_ )
                                              , isChromatogram_( t.isChromatogram_ )
                                              , isISTD_( t.isISTD_ )
                                              , use_weighting_( t.use_weighting_) 
                                              , use_bracketing_( t.use_bracketing_)
                                              , eq_(t.eq_)
                                              , weighting_( t.weighting_ )
                                              , bracketing_( t.bracketing_ )
                                              , levels_( t.levels_)
                                              , replicates_( t.replicates_ )
                                              , polynomialOrder_( t.polynomialOrder_ )
                                              , debug_level_( t.debug_level_ )
                                              , save_on_datasource_( t.save_on_datasource_ )
                                              , quanMethodFilename_( t.quanMethodFilename_ )
                                              , quanCompoundsFilename_( t.quanCompoundsFilename_ )
                                              , quanSequenceFilename_( t.quanSequenceFilename_ )
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
QuanMethod::isCounting() const
{
    return isCounting_;
}

void
QuanMethod::setIsCounting( bool v )
{
    isCounting_ = v;
}

bool
QuanMethod::isChromatogram() const
{
    return isChromatogram_;
}

void
QuanMethod::setIsChromatogram( bool v )
{
    isChromatogram_ = v;
}

bool
QuanMethod::isWeighting() const
{
    return use_weighting_;
}

void
QuanMethod::setIsWeighting( bool v )
{
    use_weighting_ = v;
}
        
QuanMethod::CalibWeighting
QuanMethod::weighting() const
{
    return weighting_;
}

void
QuanMethod::setWeighting( CalibWeighting v )
{
    weighting_ = v;
}

bool
QuanMethod::isBracketing() const
{
    return use_bracketing_;
}

void
QuanMethod::setIsBracketing( bool v )
{
    use_bracketing_ = v;
}

QuanMethod::Bracketing
QuanMethod::bracketing() const
{
    return bracketing_;
}

void
QuanMethod::setBracketing( QuanMethod::Bracketing v )
{
    bracketing_ = v;
}

bool
QuanMethod::isInternalStandard() const
{
    return isISTD_;
}

void
QuanMethod::setIsInternalStandard( bool v )
{
    isISTD_ = v;
}

uint32_t
QuanMethod::levels() const
{
    return levels_;
}

void
QuanMethod::setLevels( uint32_t v )
{
    levels_ = v;
}

uint32_t
QuanMethod::replicates() const
{
    return replicates_;
}

void
QuanMethod::setReplicates( uint32_t v )
{
    replicates_ = v;
}

uint32_t
QuanMethod::debug_level() const
{
    return debug_level_;
}

void
QuanMethod::set_debug_level( uint32_t v )
{
    debug_level_ = v;
}

bool
QuanMethod::save_on_datasource() const
{
    return save_on_datasource_;
}

void
QuanMethod::set_save_on_datasource( bool v )
{
    save_on_datasource_ = v;
}
