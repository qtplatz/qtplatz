/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "rawdata.hpp"
#include <adfs/sqlite.hpp>

rawdata::rawdata() : polarity_( negative_polarity )
                   , threshold_( -10 )
{
}

bool
rawdata::open( const boost::filesystem::path& path )
{
    path_ = path;
    return fs_.mount( path );
}


void
rawdata::setThreshold( double t )
{
    threshold_ = t;
}

void
rawdata::setPolairty( enum polarity t )
{
    polarity_ = t;
}


enum rawdata::polarity
rawdata::polarity() const
{
    return polarity_;
}

bool
rawdata::processIt( std::function< void( size_t, size_t ) > progress )
{
    auto& db = fs_.db();
}
