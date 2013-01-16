/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include "serializer.hpp"
#include <tao/CDR.h>
#include <ace/FILE_Connector.h>
#include <adinterface/controlmethodC.h>
#include <adsequence/sequence.hpp>
#include <adsequence/streambuf.hpp>
#include <adcontrols/processmethod.hpp>

using namespace sequence;

serializer::serializer()
{
}

bool
serializer::archive( std::vector<char>& vec, const ControlMethod::Method& m )
{
    vec.clear();

    TAO_OutputCDR cdr;
    cdr << m;

    // ACE_FILE_Connector connector;
    // ACE_FILE_IO file;
    // ACE_FILE_Addr addr;
    ACE_MEM_Connector connector;
    ACE_MEM_Stream stream;
    ACE_MEM_Addr addr;

    connector.connect( stream );

    size_t len = cdr.begin()->total_length();
    vec.resize( len );

    std::vector< char >::iterator it = vec.begin();
    for ( const ACE_Message_Block * mblk = cdr.begin(); mblk; mblk = mblk->cont() ) {

        stream.send( mblk->rd_ptr(), mblk->length() );

        std::copy( mblk->base(), mblk->base() + mblk->length(), it );
        it += mblk->length();
    }


    do {
    TAO_InputCDR in( static_cast< const char *>( &vec[0] ), vec.size() );
    ControlMethod::Method mm;
    in >> mm;
    std::cout << "serializer::archive store " << mm.lines.length() << std::endl;
    } while(0);

    return true;
}

bool
serializer::archive( std::vector<char>& vec, const adcontrols::ProcessMethod& m )
{
    vec.clear();
    adsequence::streambuf obuf( vec );
    std::ostream o( &obuf );
    return adcontrols::ProcessMethod::archive( o, m );
}

bool
serializer::restore( ControlMethod::Method& m, const std::vector<char>& vec )
{
    TAO_InputCDR cdr( &vec[0], vec.size() );
    return cdr >> m;
}

bool
serializer::restore( adcontrols::ProcessMethod& m, const std::vector<char>& vec )
{
    adsequence::streambuf ibuf( const_cast<std::vector<char>&>(vec) );
    std::istream is( &ibuf );
    return adcontrols::ProcessMethod::restore( is, m );
}

bool
serializer::restore( boost::shared_ptr<ControlMethod::Method>& ptr, const std::vector<char>& vec )
{
    if ( ! ptr )
        ptr.reset( new ControlMethod::Method() );
    return restore( *ptr, vec );
}

bool
serializer::restore( boost::shared_ptr< adcontrols::ProcessMethod>& ptr, const std::vector<char>& vec )
{
    if ( ! ptr )
        ptr.reset( new adcontrols::ProcessMethod() );
    return restore( *ptr, vec );
}
