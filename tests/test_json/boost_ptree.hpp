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

#pragma once
#include <memory>
#include <boost/property_tree/ptree.hpp>

struct data;

class boost_ptree {
public:
    boost_ptree();
    ~boost_ptree();
    bool parse( const std::string& );
    std::string stringify( bool pritty = false ) const;
    static std::string stringify( const boost::property_tree::ptree&, bool pritty = false );
    bool map( data& );
    static std::string make_json( const data& );

    std::unique_ptr< boost::property_tree::ptree > ptree;
};
