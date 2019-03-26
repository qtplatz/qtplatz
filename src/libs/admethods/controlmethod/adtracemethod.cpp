/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "adtracemethod.hpp"
#include "serializer.hpp"
#include <adportable/debug.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <string>
#include <map>
#include <vector>


namespace admethods {
    namespace controlmethod {

        constexpr const boost::uuids::uuid ADTraceMethod::__clsid__;

        using namespace boost::serialization;
        template<typename Archive> ADMETHODSSHARED_EXPORT void ADTraceMethod::serialize( Archive& ar, const unsigned int version ) {
            if ( Archive::is_saving::value ) {
                auto json = toJson();
                ar & BOOST_SERIALIZATION_NVP( json );
            } else {
                std::string json;
                ar & BOOST_SERIALIZATION_NVP( json );
                fromJson( json );
            }
        }
    }
}

using namespace admethods::controlmethod;

ADTraceMethod::~ADTraceMethod()
{
}

ADTraceMethod::ADTraceMethod() : data_{ { ADTrace() } }
{
}

ADTraceMethod::ADTraceMethod( const ADTraceMethod& t ) : data_( t.data_ )
{
}

bool
ADTraceMethod::archive( std::ostream& os, const ADTraceMethod& t )
{
    portable_binary_oarchive ar( os );
    try {
        ar & t;
        return true;
    } catch ( std::exception& ) {
        assert(0);
    }
    return false;
}

//static
bool
ADTraceMethod::restore( std::istream& is, ADTraceMethod& t )
{
    portable_binary_iarchive ar( is );
    try {
        ar & t;
        return true;
    } catch ( std::exception& ) {
        assert(0);
    }
    return false;
}

std::string
ADTraceMethod::toJson( bool pritty ) const
{
    QJsonArray a;

    for ( const auto& t: data_ ) {
        QJsonObject obj{ {"enable", t.enable() }, {"legend", QString::fromStdString( t.legend() )}, { "vOffset", t.vOffset()} };
        a.push_back( obj );
    }
    QJsonObject jobj{ {"clsid", __clsid_str}, { "modelClass", __modelClass__ }, { "data", a } };

    return QJsonDocument( jobj ).toJson( pritty ? QJsonDocument::Indented : QJsonDocument::Compact ).toStdString();
}

void
ADTraceMethod::fromJson( const std::string& json )
{
    auto jobj = QJsonDocument::fromJson( QByteArray( json.data(), json.size() ) ).object();
    if ( jobj[ "clsid" ].toString() != __clsid_str )
        assert(0);

    auto ja = jobj[ "data" ].toArray();

    auto it = data_.begin();
    for ( const auto& jt: ja ) {
        if ( it != data_.end() ) {
            const auto& jitem = jt.toObject();
            it->setEnable( jitem[ "enable" ].toBool() );
            it->setLegend( jitem[ "legend" ].toString().toStdString() );
            it->setVOffset( jitem[ "vOffset" ].toDouble() );
        }
        ++it;
    }
}

///////////////////////////
ADTrace::ADTrace() : d_( false, "", 0.0 )
{
}

ADTrace::ADTrace( std::tuple< bool, std::string, double >&& t ) : d_( t )
{
}

ADTrace::ADTrace( const ADTrace& t ) : d_( t.d_ )
{
}

bool
ADTrace::enable() const
{
    return std::get<0>(d_);
}

void
ADTrace::setEnable( bool enable )
{
    std::get<0>(d_) = enable;
}

double
ADTrace::vOffset() const
{
    return std::get<2>(d_);
}

void
ADTrace::setVOffset( double t )
{
    std::get<2>(d_) = t;
}

std::string
ADTrace::legend() const
{
    return std::get<1>(d_);
}

void
ADTrace::setLegend( const std::string& t )
{
    std::get<1>(d_) = t;
}
