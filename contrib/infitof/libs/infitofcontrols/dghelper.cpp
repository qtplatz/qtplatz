/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "dghelper.hpp"
#include <adportable/debug.hpp>

using namespace infitofcontrols;

dghelper::dghelper() : file_( 0 ), line_( 0 )
{
}

dghelper::dghelper( const char * file, int line, std::string&& t ) : file_( file ), line_( line ), prefix_( t )
{
}

void
dghelper::operator()( const infitofcontrols::method& infm ) const
{
    adportable::debug dout( file_, line_ );

    dout << "\t============ " << prefix_ << " ==========================\n";
    size_t n(0);
    std::ostringstream o;
    for ( auto& p: infm.tof().protocols ) {
        dout << n++
             << std::make_pair( p.pulser.delay, p.pulser.width )
             << ", " << std::make_pair( p.inject.delay, p.inject.width )
             << ", " << std::make_pair( p.exit.delay, p.exit.width )
             << ", " << std::make_pair( p.gate[0].delay, p.gate[0].width )
             << ", " << std::make_pair( p.gate[1].delay, p.gate[1].width )
             << "\t:" << std::make_pair( p.external_adc_delay.delay, p.external_adc_delay.width )
             << ", " << std::make_pair( p.exit2.delay, p.exit2.width );
    }
    dout << "\n============ " << prefix_ << " ==========================";
}

void
dghelper::operator()( const adcontrols::ControlMethod::Method& m ) const
{
    infitofcontrols::method infm;
    auto it = m.find( m.begin(), m.end(), infitofcontrols::method::clsid() );
    if ( it != m.end() && it->get<>( *it, infm ) ) {
        (*this)( infm );
    } else {
        adportable::debug( file_, line_ )  << "\tMethod does not contains 'infitofcontrols::method::clsid'";
    }
}
