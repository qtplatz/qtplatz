/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanresponse.hpp"
#include <boost/uuid/string_generator.hpp>
#include <boost/json.hpp>

using namespace adcontrols;

QuanResponse::~QuanResponse()
{
}

QuanResponse::QuanResponse() : idx_( 0 )
                             , fcn_( 0 )
                             , intensity_( 0 )
                             , amounts_( 0 )
                             , mass_( 0 )
                             , tR_( 0 )
                             , countTimeCounts_( 0 )
                             , countTriggers_( 0 )
                             , pkarea_( 0 )
                             , pkheight_( 0 )
                             , pkwidth_( 0 )
                             , theoreticalPlate_( 0 )
                             , capacityFactor_( 0 )
                             , asymmetry_( 0 )
                             , resolution_( 0 )
{
}

QuanResponse::QuanResponse( const QuanResponse& t ) : idx_( t.idx_ )
                                                    , idTable_( t.idTable_ )
                                                    , idCompound_( t.idCompound_ )
                                                    , dataGuid_( t.dataGuid_ )
                                                    , fcn_( t.fcn_ )
                                                    , intensity_( t.intensity_ )
                                                    , amounts_( t.amounts_ )
                                                    , mass_( t.mass_ )
                                                    , tR_( t.tR_ )
                                                    , formula_( t.formula_ )
                                                    , countTimeCounts_( t.countTimeCounts_ )
                                                    , countTriggers_( t.countTriggers_ )
                                                    , pkarea_( t.pkarea_ )
                                                    , pkheight_( t.pkheight_ )
                                                    , pkwidth_( t.pkwidth_ )
                                                    , theoreticalPlate_( t.theoreticalPlate_ )
                                                    , capacityFactor_( t.capacityFactor_ )
                                                    , asymmetry_( t.asymmetry_ )
                                                    , resolution_( t.resolution_ )
{
}

int32_t
QuanResponse::peakIndex() const
{
    return idx_;
}

const boost::uuids::uuid&
QuanResponse::idTable() const
{
    return idTable_;
}

const boost::uuids::uuid&
QuanResponse::idCompound() const
{
    return idCompound_;
}

const boost::uuids::uuid&
QuanResponse::dataGuid() const
{
    return dataGuid_;
}

int32_t
QuanResponse::fcn() const
{
    return fcn_;
}

double
QuanResponse::intensity() const
{
    return intensity_;
}

double
QuanResponse::amounts() const
{
    return amounts_;
}

double
QuanResponse::mass() const
{
    return mass_;
}

double
QuanResponse::tR() const
{
    return tR_;
}

uint64_t
QuanResponse::countTimeCounts() const
{
    return countTimeCounts_;
}

uint64_t
QuanResponse::countTriggers() const
{
    return countTriggers_;
}


void
QuanResponse::setPeakIndex( int32_t d )
{
    idx_ = d;
}

boost::uuids::uuid&
QuanResponse::idTable()
{
    return idTable_;
}

boost::uuids::uuid&
QuanResponse::idCompound()
{
    return idCompound_;
}

template<> ADCONTROLSSHARED_EXPORT
void
QuanResponse::setDataGuid( const std::wstring& d )
{
    dataGuid_ = boost::uuids::string_generator()( d );
}

template<> ADCONTROLSSHARED_EXPORT
void
QuanResponse::setDataGuid( const std::string& d )
{
    dataGuid_ = boost::uuids::string_generator()( d );
}

void
QuanResponse::setDataGuid( const boost::uuids::uuid& d )
{
    dataGuid_ = d;
}

void
QuanResponse::setFcn( int32_t d )
{
    fcn_ = d;
}

void
QuanResponse::setIntensity( double d )
{
    intensity_ = d;
}

void
QuanResponse::setAmounts( double d )
{
    amounts_ = d;
}

void
QuanResponse::setMass( double d )
{
    mass_ = d;
}

void
QuanResponse::set_tR( double d )
{
    tR_ = d;
}

void
QuanResponse::setCountTimeCounts( uint64_t d )
{
    countTimeCounts_ = d;
}

void
QuanResponse::setCountTriggers( uint64_t d )
{
    countTriggers_ = d;
}

double
QuanResponse::pkarea() const
{
    return pkarea_;
}

double
QuanResponse::pkheight() const
{
    return pkheight_;
}

double
QuanResponse::pkwidth() const
{
    return pkwidth_;
}

double
QuanResponse::theoreticalPlate() const
{
    return theoreticalPlate_;
}

double
QuanResponse::capacityFactor() const
{
    return capacityFactor_;
}

double
QuanResponse::asymmetry() const
{
    return asymmetry_;
}

double
QuanResponse::resolution() const
{
    return resolution_;
}

void
QuanResponse::setPkArea( double t )
{
    pkarea_ = t;
}

void
QuanResponse::setPkHeight( double t )
{
    pkheight_ = t;
}

void
QuanResponse::setPkWidth( double t )
{
    pkwidth_ = t;
}

void
QuanResponse::setTheoreticalPlate( double t )
{
    theoreticalPlate_ = t;
}

void
QuanResponse::setCapacityFactor( double t )
{
    capacityFactor_ = t;
}

void
QuanResponse::setAsymmetry( double t )
{
    asymmetry_ = t;
}

void
QuanResponse::setResolution( double t )
{
    resolution_ = t;
}

QuanResponse::operator boost::json::object () const
{
    return boost::json::object{
        { "idx",            idx_ }
        , { "idTable_",     idTable_ }
        , { "idCompound",   idCompound_ }
        , { "dataGuid",     dataGuid_ }
        , { "fcn",          fcn_ }
        , { "intensity",	intensity_ }
        , { "amounts",      amounts_ }
        , { "mass",         mass_ }
        , { "tR",           tR_ }
        , { "formula",      formula_ }
        , { "countTimeCounts",	 countTimeCounts_ }
        , { "countTriggers",	 countTriggers_ }
        , { "pkarea",       pkarea_ }
        , { "pkheight",     pkheight_ }
        , { "pkwidth",      pkwidth_ }
        , { "theoreticalPlate",	 theoreticalPlate_ }
        , { "capacityFactor",	 capacityFactor_ }
        , { "asymmetry",	asymmetry_ }
        , { "resolution",	resolution_ }
    };

}
