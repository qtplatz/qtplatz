/**************************************************************************
** Copyright (C) 2019-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanresponsemethod.hpp"
#include "msfinder.hpp"
#include <adportable/json_helper.hpp>
#include <adportable/json/extract.hpp>

using namespace adcontrols;

QuanResponseMethod::~QuanResponseMethod()
{
}

QuanResponseMethod::QuanResponseMethod() : intensityMethod_( idArea )
                                         , findAlgo_( idFindLargest )
                                         , widthMethod_( idWidthDaltons )
                                         , widthPpm_( 100 )
                                         , widthDaltons_( 0.010 )
                                         , dataSelect_( idAverage )
                                         , enableAutoTargeting_(true)
                                         , peakWidthForChromatogram_( 1.0 )
{
}

QuanResponseMethod::QuanResponseMethod( const QuanResponseMethod& t ) : intensityMethod_( t.intensityMethod_ )
                                                                      , findAlgo_( t.findAlgo_ )
                                                                      , widthMethod_( t.widthMethod_ )
                                                                      , widthPpm_( t.widthPpm_ )
                                                                      , widthDaltons_( t.widthDaltons_ )
                                                                      , dataSelect_( t.dataSelect_ )
                                                                      , enableAutoTargeting_( t.enableAutoTargeting_ )
                                                                      , peakWidthForChromatogram_( t.peakWidthForChromatogram_ )
{
}

QuanResponseMethod::idWidthMethod
QuanResponseMethod::widthMethod() const
{
    return widthMethod_;
}

void
QuanResponseMethod::setWidthMethod( idWidthMethod id )
{
    widthMethod_ = id;
}

double
QuanResponseMethod::width( idWidthMethod id ) const
{
    if ( id == idWidthPpm )
        return widthPpm_;
    else
        return widthDaltons_;
}

void
QuanResponseMethod::setWidth( double w, idWidthMethod id )
{
    if ( id == idWidthPpm )
        widthPpm_ = w;
    else
        widthDaltons_ = w;
}

QuanResponseMethod::idDataSelect
QuanResponseMethod::dataSelectionMethod() const
{
    return dataSelect_;
}

void
QuanResponseMethod::setDataSelectionMethod( idDataSelect id )
{
    dataSelect_ = id;
}

void
QuanResponseMethod::setFindAlgorithm( idFindAlgorithm id )
{
    findAlgo_ = id;
}

idFindAlgorithm
QuanResponseMethod::findAlgorithm() const
{
    return findAlgo_;
}

void
QuanResponseMethod::setIntensityMethod( idIntensity id )
{
    intensityMethod_ = id;
}

QuanResponseMethod::idIntensity
QuanResponseMethod::intensityMethod() const
{
    return intensityMethod_;
}

bool
QuanResponseMethod::enableAutoTargeting() const
{
    return enableAutoTargeting_;
}

void
QuanResponseMethod::setEnableAutoTargeting( bool enable )
{
    enableAutoTargeting_ = enable;
}

double
QuanResponseMethod::peakWidthForChromatogram() const
{
    return peakWidthForChromatogram_;
}

void
QuanResponseMethod::setPeakWidthForChromatogram( double value )
{
    peakWidthForChromatogram_ = value;
}

namespace adcontrols {

    void
    tag_invoke( boost::json::value_from_tag, boost::json::value& jv, const QuanResponseMethod& t )
    {
        jv = {
            { "clsid",                     "adcontrols::QuanResponseMethod" }
            , { "width.widthMethod",        uint32_t( t.widthMethod_ ) } // enum
            , { "width.widthPpm",           t.widthPpm_ }
            , { "width.widthDaltons",       t.widthDaltons_ }
            , { "data.dataSelectionMethod", uint32_t( t.dataSelect_ ) } // enum
            , { "data.intensity",           uint32_t( t.intensityMethod_ ) } // enum  // doCentroid or Area
            , { "data.findAlgo",            uint32_t( t.findAlgo_ ) }     // enum     // Most intens peak/Closest
            , { "autoTargeting.enable",     t.enableAutoTargeting_ }      // bool
            , { "autoTargeting.peakwidth",  t.peakWidthForChromatogram_ } // double
        };
    }

    QuanResponseMethod
    tag_invoke( boost::json::value_to_tag< QuanResponseMethod >&, const boost::json::value& jv )
    {
        using namespace adportable::json;
        QuanResponseMethod t;
        if ( jv.is_object() ) {
            const auto& obj = jv.as_object();
            uint32_t x;
            extract( obj, x, "width.widthMethod" );  t.widthMethod_ = QuanResponseMethod::idWidthMethod( x );
            extract( obj, t.widthPpm_, "width.widthPpm" );
            extract( obj, t.widthDaltons_, "width.widthDaltons" );
            extract( obj, x, "data.dataSelectionMethod" ); t.dataSelect_ = QuanResponseMethod::idDataSelect( x );
            extract( obj, x, "data.intensity" ); t.intensityMethod_ = QuanResponseMethod::idIntensity(x);
            extract( obj, x, "data.findAlgo" ); t.findAlgo_ = idFindAlgorithm( x );
            extract( obj, t.enableAutoTargeting_, "autoTargeting.enable" );
            extract( obj, t.peakWidthForChromatogram_, "autoTargeting.peakwidth" );
        }
        return t;
    }
}
