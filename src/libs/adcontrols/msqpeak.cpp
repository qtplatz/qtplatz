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

#include "msqpeak.hpp"
#include "msqpeaks.hpp"

using namespace adcontrols;

static std::wstring empty_wstring;

MSQPeak::MSQPeak( const std::wstring& dataGuid
                  , uint32_t idx
                  , uint32_t fcn
                  , MSQPeaks * pks )
    : dataGuid_( dataGuid )
    , idx_(idx)
    , fcn_(fcn)
    , time_(0)
    , mass_(0)
    , intensity_(0)
    , mode_(0)
    , amount_(0)
    , istd_(0)
    , isSTD_(false)
    , isIS_(false)
    , container_( pks->shared_from_this() )
{
}

MSQPeak::~MSQPeak()
{
}

MSQPeak::MSQPeak( const MSQPeak& t )
    : dataGuid_( t.dataGuid_ )
    , idx_( t.idx_ )
    , fcn_( t.fcn_ )
    , time_( t.time_ )
    , mass_( t.mass_ )
    , intensity_( t.intensity_ )
    , mode_( t.mode_ )
    , amount_( t.amount_ )
    , istd_( t.istd_ )
    , isSTD_( t.isSTD_ )
    , isIS_( t.isIS_ )
    , compId_( t.compId_ )
    , description_( t.description_ )
    , formula_( t.formula_)
    , proto_( t.proto_ )
{
}

void
MSQPeak::time( double v )
{
    time_ = v;
}

double
MSQPeak::time() const
{
    return time_;
}

void
MSQPeak::mass( double v )
{
    mass_ = v;
}

double
MSQPeak::mass() const
{
    return mass_;
}

void
MSQPeak::intensity( double v )
{
    intensity_ = v;
}

double
MSQPeak::intensity() const
{
    return intensity_;
}

void
MSQPeak::amount( double v )
{
    amount_ = v;
}

double
MSQPeak::amount() const
{
    return amount_;
}

void
MSQPeak::istd( uint32_t v )
{
    istd_ = v;
}

uint32_t
MSQPeak::istd() const
{
    return istd_;
}

void
MSQPeak::isSTD( bool v )
{
    isSTD_ = v;
}

bool
MSQPeak::isSTD() const
{
    return isSTD_;
}

void
MSQPeak::isIS( bool v )
{
    isIS_ = v;
}

bool
MSQPeak::isIS() const
{
    return isIS_;
}

void
MSQPeak::mode( int32_t v )
{
    mode_ = v;
}

int32_t 
MSQPeak::mode() const
{
    return mode_;
}

void
MSQPeak::idx( uint32_t idx )
{
    idx_ = idx;
}

void
MSQPeak::fcn( uint32_t fcn )
{
    fcn_ = fcn;
}

uint32_t
MSQPeak::idx() const
{
    return idx_;
}

uint32_t
MSQPeak::fcn() const
{
    return fcn_;
}

const std::wstring&
MSQPeak::dataGuid() const
{
    return dataGuid_;
}

void
MSQPeak::formula( const std::string& v )
{
    formula_ = v;
}

const std::string&
MSQPeak::formula() const
{
    return formula_;
}

void
MSQPeak::componentId( const std::wstring& v )
{
    compId_ = v;
}

const std::wstring&
MSQPeak::componentId() const
{
    return compId_;
}

void
MSQPeak::description( const std::string& v )
{
    description_ = v;
}

const std::string&
MSQPeak::description() const
{
    return description_;
}

void
MSQPeak::protocol( const std::string& v )
{
    proto_ = v;
}

const std::string&
MSQPeak::protocol() const
{
    return proto_;
}
