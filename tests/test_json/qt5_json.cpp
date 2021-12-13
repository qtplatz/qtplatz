// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2018 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "qt5_json.hpp"
#include "data.hpp"
#include <QDebug>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <boost/exception/all.hpp>

namespace {
    template<class T> void extract( const QJsonObject& obj, T& t, const QString& key );

    template<> void extract( const QJsonObject& obj, std::string& t, const QString& key )  {
        t = obj[ key ].toString().toStdString();
    }
    template<> void extract( const QJsonObject& obj, QString& t, const QString& key )  {
        t = obj[ key ].toString();
    }
    template<> void extract( const QJsonObject& obj, double& t, const QString& key )  {
        t = obj[ key ].toDouble();
    }
    template<> void extract( const QJsonObject& obj, int32_t& t, const QString& key )  {
        t = obj[ key ].toInt();
    }
    template<> void extract( const QJsonObject& obj, uint32_t& t, const QString& key )  {
        t = obj[ key ].toInt();
    }
    template<> void extract( const QJsonObject& obj, uint64_t& t, const QString& key )  {
        t = obj[ key ].toDouble();
    }
}


struct qt5_json::impl {
    QJsonDocument doc;
};


qt5_json::qt5_json() : impl_( std::make_unique< impl >() )
{
}

qt5_json::~qt5_json()
{
}

bool
qt5_json::parse( const std::string& json_string )
{
    QByteArray data( json_string.data(), json_string.size() );
    impl_->doc = QJsonDocument::fromJson( data );
    return true;
}

std::string
qt5_json::stringify( bool pritty ) const
{
    QByteArray xdata( impl_->doc.toJson( pritty ? QJsonDocument::Indented : QJsonDocument::Compact ) );
    return std::string( xdata.data() );
}

std::string
qt5_json::stringify( const QJsonObject& obj, bool pritty )
{
    QJsonDocument doc( obj );
    QByteArray xdata( doc.toJson( pritty ? QJsonDocument::Indented : QJsonDocument::Compact ) );
    return std::string( xdata.data() );
}

bool
qt5_json::map( data& d )
{
    const auto& jobj = impl_->doc.object();
    const auto& top = jobj[ "tick" ].toObject();

    extract( top, d.tick, "tick" );
    extract( top, d.time, "time" );
    extract( top, d.nsec, "nsec" );

    // d.tick = top[ "tick" ].toInt();
    // d.time = top[ "time" ].toString().toULongLong();
    // d.nsec = top[ "nsec" ].toInt();

    const auto& hv = top[ "hv" ].toObject();
    {
        const auto& values = hv[ "values" ].toArray();
        for ( const auto& value: values ) {
            auto obj = value.toObject();
            tick::hv::value x;
            extract( obj, x.id, "id" );
            extract( obj, x.name,  "name" );
            extract( obj, x.sn, "sn" );
            extract( obj, x.set, "set" );
            extract( obj, x.act, "act" );
            extract( obj, x.unit, "unit" );
            // x.id = obj[ "id" ].toInt();
            // x.name = obj[ "name" ].toString().toStdString();
            // x.sn   = obj[ "sn" ].toInt();
            // x.set  = obj[ "set" ].toDouble();
            // x.act  = obj[ "act" ].toDouble();
            // x.unit = obj[ "unit" ].toString().toStdString();
            d.values.emplace_back( x );
        }
    }

    d.alarm = top["alarms"].toObject()["alarm"].toObject()["text"].toString().toStdString();

    const auto& adc = top["adc"].toObject();
    extract( adc, d.adc.tp, "tp" );
    extract( adc, d.adc.nacc, "nacc" );
    // d.adc.tp = adc["tp"].toString().toULongLong();
    // d.adc.nacc = adc["nacc"].toString().toInt();
    {
        const auto& values = adc["values"].toArray();
        for ( const auto& value: values ) {
            d.adc.values.emplace_back( value.toDouble() );
        }
    }
    return true;
}

std::string
qt5_json::make_json( const data& d )
{
    QJsonArray values;
    for ( const auto& value: d.values ) {
        QJsonObject child{
            { "id", qint32( value.id ) }
            , { "name", QString::fromStdString( value.name ) }
            , { "sn", qint32( value.sn ) }
            , { "set", value.set }
            , { "act", value.act }
            , { "unit", QString::fromStdString( value.unit ) }
        };
        values.append( child );
    }

    QJsonArray adc_values;
    for ( const auto& adc_value: d.adc.values )
        adc_values.push_back( adc_value );

    QJsonObject jobj{
        { "tick", QJsonObject{
                { "tick", qint32( d.tick ) }
                , { "time", qint64( d.time ) }
                , { "nsec", qint32( d.nsec ) }
                , { "hv", QJsonObject{
                        { "values", values }
                        , { "alarms", QJsonObject{
                                { "alarm", QJsonObject{
                                        { "text", QString::fromStdString( d.alarm ) }
                                    }
                                }
                            }
                        }
                    }
                }
                , { "adc", QJsonObject{
                        { "tp", qint64( d.adc.tp ) }
                        , { "nacc", qint32( d.adc.nacc ) }
                        , { "values", adc_values }
                    }
                }
            }
        }
    };
    return stringify( jobj );
}
