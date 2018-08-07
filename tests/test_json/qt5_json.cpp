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
#include <QJsonObject>
#include <QJsonArray>
#include <QByteArray>
#include <iostream>


qt5_json::qt5_json() : doc( std::make_unique< QJsonDocument >() )
{
}

qt5_json::~qt5_json()
{
}

bool
qt5_json::parse( const std::string& json_string )
{
    QByteArray data( json_string.data(), json_string.size() );
    *doc = QJsonDocument::fromJson( data );
}

std::string
qt5_json::stringify( bool pritty ) const
{
    QByteArray xdata( doc->toJson( pritty ? QJsonDocument::Indented : QJsonDocument::Compact ) );
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
    const auto& jobj = doc->object();
    const auto& top = jobj[ "tick" ].toObject();
    
    d.tick = top[ "tick" ].toInt();
    d.time = top[ "time" ].toString().toULongLong();
    d.nsec = top[ "nsec" ].toInt();

    const auto& hv = top[ "hv" ].toObject();
    {
        const auto& values = hv[ "values" ].toArray();
        for ( const auto& value: values ) {
            auto obj = value.toObject();
            tick::hv::value x;
            x.id = obj[ "id" ].toInt();
            x.name = obj[ "name" ].toString().toStdString();
            x.sn   = obj[ "sn" ].toInt();
            x.set  = obj[ "set" ].toDouble();
            x.act  = obj[ "act" ].toDouble();
            x.unit = obj[ "unit" ].toString().toStdString();
            d.values.emplace_back( x );
        }
    }

    d.alarm = top["alarms"].toObject()["alarm"].toObject()["text"].toString().toStdString();
    
    const auto& adc = top["adc"].toObject();
    d.adc.tp = adc["tp"].toString().toULongLong();
    d.adc.nacc = adc["nacc"].toString().toInt();
    {
        const auto& values = adc["values"].toArray();
        for ( const auto& value: values ) {
            d.adc.values.emplace_back( value.toDouble() );
        }
    }
}

std::string
qt5_json::make_json( const data& d )
{
    QJsonObject jobj, top;
 
    top["tick"] = qint32( d.tick );
    top["time"] = std::to_string( d.time ).c_str();
    top["nsec"] = qint32( d.nsec );

    { // hv
        QJsonArray values;
        for ( const auto& value: d.values ) {
            QJsonObject child;
            child[ "id" ] = qint32( value.id );
            child[ "name" ] = QString::fromStdString( value.name );
            child[ "sn" ] = qint32( value.sn );
            child[ "set" ] = value.set;
            child[ "act" ] = value.act;
            child[ "unit" ] =  QString::fromStdString( value.unit );
            values.append( child );
        }
        top[ "values" ] = values;
    }

    QJsonObject alarms, alarm;
    alarm["text"] = QString::fromStdString( d.alarm );
    alarms["alarm"] = alarm;
    top["alarms"] = alarms;

    {
        QJsonObject adc;
        adc["tp"] = qint64( d.adc.tp );
        adc["nacc"] = qint32( d.adc.nacc );
        QJsonArray values;
        for ( const auto& value: d.adc.values ) {
            values.append( value );
        }
        adc["values"] = values;
        top[ "adc" ] = adc;
    }

    jobj[ "tick" ] = top;

    return stringify( jobj );
}


