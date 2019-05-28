/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "msfinder.hpp"

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
{
}

QuanResponseMethod::QuanResponseMethod( const QuanResponseMethod& t ) : intensityMethod_( t.intensityMethod_ )
                                                                      , findAlgo_( t.findAlgo_ )
                                                                      , widthMethod_( t.widthMethod_ )
                                                                      , widthPpm_( t.widthPpm_ )
                                                                      , widthDaltons_( t.widthDaltons_ )
                                                                      , dataSelect_( t.dataSelect_ )
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


std::string
QuanResponseMethod::toJson() const
{
    boost::property_tree::ptree pt;
    pt.put( "clsid", "adcontrols::QuanResponseMethod" );
    pt.put( "width.widthMethod", widthMethod_ );
    pt.put( "width.widthPpm", widthPpm_ );
    pt.put( "width.widthDaltons", widthDaltons_ );
    pt.put( "data.dataSelectionMethod", dataSelect_ );
    pt.put( "data.intensity", intensityMethod_ );  // doCentroid or Area
    pt.put( "data.findAlgo", findAlgo_ );          // Most intens peak/Closest

    std::ostringstream o;
    boost::property_tree::write_json( o, pt );

    return o.str();
}

void
QuanResponseMethod::fromJson( const std::string& json )
{
    boost::property_tree::ptree pt;

    std::istringstream in( json );
    boost::property_tree::read_json( in, pt );
    // if ( auto widthMethod = pt.get_optional< int >( "width.widthMethod" ) )
    //     widthMethod_ = static_cast< idWidthMethod >( widthMethod.get() );
    // if ( auto widthPpm = pt.get_optional< double >( "width.widthPpm" ) )
    //     widthPpm_ = widthPpm.get();
    // if ( auto widthDaltons = pt.get_optional< double >( "width.widthDaltons" ) )
    //     widthDaltons_ = widthDaltons.get();
    // if ( auto dataMethod = pt.get_optional< int >( "data.dataMethod" ) )
    //     dataMethod_ = static_cast< idData >( dataMethod.get() );
}
