// -*- C++ -*-
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

#include "msreferencedefns.h"

using namespace adcontrols;

/////////////////////// Formula /////////////////////////////

MSRefFormula::~MSRefFormula()
{
}

MSRefFormula::MSRefFormula() : enable_(true)
                             , formula_( L"" )
                             , adduct_( L"H" )
                             , loss_( L"" )
                             , polarityPositive_(true)
                             , chargeCount_(1)
                             , comments_( L"" )
{
}

MSRefFormula::MSRefFormula( const MSRefFormula& t ) : enable_( t.enable_ )
                                                    , formula_( t.formula_ )
                                                    , adduct_( t.adduct_ )
                                                    , loss_( t.loss_ )
                                                    , polarityPositive_( t.polarityPositive_ )
                                                    , chargeCount_( t.chargeCount_ )
                                                    , comments_( t.comments_ )
{
}

MSRefFormula::MSRefFormula( const std::wstring& formula
                           , bool enable
                           , bool positive
                           , const std::wstring& adduct
                           , const std::wstring& loss
                           , size_t chargeCount ) : enable_( enable )
                                                  , formula_( formula )
                                                  , adduct_( adduct )
                                                  , loss_( loss )  
                                                  , polarityPositive_( positive )
                                                  , chargeCount_( chargeCount )
{
} 

MSRefFormula::operator bool () const
{
    return enable_;
}

void 
MSRefFormula::enable( bool value )
{
    enable_ = value;
}

const std::wstring&
MSRefFormula::formula() const
{
    return formula_;
}

void 
MSRefFormula::formula( const std::wstring&  value )
{
    formula_ = value;
}

const std::wstring&
MSRefFormula::adduct() const
{
    return adduct_;
}

void 
MSRefFormula::adduct( const std::wstring&  value )
{
    adduct_ = value;
}

const std::wstring&
MSRefFormula::loss() const
{
    return loss_;
}

void 
MSRefFormula::loss( const std::wstring&  value )
{
    loss_ = value;
}

bool 
MSRefFormula::polarityPositive() const
{
    return polarityPositive_;
}

void 
MSRefFormula::polarityPositive( bool  value )
{
    polarityPositive_ = value;
}

size_t
MSRefFormula::chargeCount() const
{
    return chargeCount_;
}

void 
MSRefFormula::chargeCount( size_t  value )
{
    chargeCount_ = value;
}

const std::wstring&
MSRefFormula::comments() const
{
    return comments_;
}

void 
MSRefFormula::comments( const std::wstring&  value )
{
    comments_ = value;
}


//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

MSRefSeries::~MSRefSeries()
{
}

MSRefSeries::MSRefSeries() : enable_( true )
                           , lowMass_( 100 )
                           , highMass_( 1000 )
                           , polarityPositive_( true )
                           , chargeCount_( 1 )
{
}

MSRefSeries::MSRefSeries( const MSRefSeries& t ) : enable_( t.enable_ )
                                                , repeat_( t.repeat_ )
                                                , endGroup_( t.endGroup_ )
                                                , adduct_( t.adduct_ )
                                                , loss_( t.loss_ )
                                                , lowMass_( t.lowMass_ )
                                                , highMass_( t.highMass_ )
                                                , polarityPositive_( t.polarityPositive_ )
                                                , chargeCount_( t.chargeCount_ )
                                                , comments_( t.comments_ )
{
}


const std::wstring&
MSRefSeries::repeat() const
{
    return repeat_;
}

const std::wstring&
MSRefSeries::endGroup() const
{
    return endGroup_;
}

const std::wstring&
MSRefSeries::adduct() const
{
    return adduct_;
}

const std::wstring&
MSRefSeries::loss() const
{
    return loss_;
}

double 
MSRefSeries::lowMass()
{
    return lowMass_;
}

double 
MSRefSeries::highMass()
{
    return highMass_;
}

bool 
MSRefSeries::polarityPositive() const
{
    return polarityPositive_;
}

size_t
MSRefSeries::chargeCount() const
{
    return chargeCount_;
}

const std::wstring&
MSRefSeries::comments() const
{
    return comments_;
}

void 
MSRefSeries::enable( bool value )
{
    enable_ = value;
}

void 
MSRefSeries::endGroup( const std::wstring&  value )
{
    endGroup_ = value;
}

void 
MSRefSeries::adduct( const std::wstring&  value )
{
    adduct_ = value;
}

void 
MSRefSeries::loss( const std::wstring&  value )
{
    loss_ = value;
}

void 
MSRefSeries::lowMass( double  value )
{
    lowMass_ = value;
}

void 
MSRefSeries::highMass( double  value )
{
    highMass_ = value;
}

void 
MSRefSeries::polarityPositive( bool  value )
{
    polarityPositive_ = value;
}

void 
MSRefSeries::chargeCount( size_t  value )
{
    chargeCount_ = value;
}

void 
MSRefSeries::comments( const std::wstring&  value )
{
    comments_ = value;
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

MSReferenceDefns::~MSReferenceDefns()
{
}

MSReferenceDefns::MSReferenceDefns()
{
}

MSReferenceDefns::MSReferenceDefns( const MSReferenceDefns & t ) : refFormula_( t.refFormula_ )
                                                                 , refSeries_( t.refSeries_ ) 
{
}

void
MSReferenceDefns::addFormula( const MSRefFormula& v )
{
    refFormula_.push_back( v );
}

void
MSReferenceDefns::addSeries( const MSRefSeries& v )
{
    refSeries_.push_back( v );
}

const std::vector< MSRefFormula >&
MSReferenceDefns::formulae() const
{
    return refFormula_;
}

const std::vector< MSRefSeries >&
MSReferenceDefns::series() const
{
    return refSeries_;
}