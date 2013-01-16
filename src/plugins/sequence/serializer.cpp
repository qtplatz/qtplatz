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
#include <ace/CDR_Stream.h>
#include <ace/MEM_Connector.h>
#include <adinterface/controlmethodC.h>
#include <adsequence/sequence.hpp>
#include <adsequence/streambuf.hpp>
#include <adcontrols/processmethod.hpp>

// #include <boost/serialization/vector.hpp>
// #include <adportable/portable_binary_oarchive.hpp>
// #include <adportable/portable_binary_iarchive.hpp>

#if 0
namespace sequence {

    class vector_array {
        vector< std::vector<char> > vec;
    public:
        void append( ACE_Message_Block * mb ) {
            vec.push_back( std::vector<char> );
            std::vector<char>& buf = vec.back();
            buf.resize( mb->length() );
            std::copy( mb->rd_ptr(), mb->rd_ptr() + mb->length(), buf.begin() );
        }

        typedef vector< std::vector< char > > vector_type;
        
        vector< std::vector< char > >::const_iterator begin() const {
            return vec.begin();
        }

        vector< std::vector< char > >::const_iterator end() const {
            return vec.end();
        }

    private:
        firend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int /* version */ ) {
            ar & BOOST_SERIALIZATION_NVP( vec );
        }
        
    };

}
#endif

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

    ACE_MEM_Connector connector;
    ACE_MEM_Stream stream;
    ACE_MEM_Addr addr; //( ACE_DEFAUT_SERVER_PORT );
    
    if ( connector.connect( stream, addr.get_remote_addr() ) == -1 )
        return false;

    char buf[1024];
    for ( const ACE_Message_Block * mblk = cdr.begin(); mblk; mblk = mblk->cont() ) {
        stream.send( mblk->rd_ptr(), mblk->length() );
        size_t len;
        while ( ( len = stream.recv( buf, sizeof(buf) ) ) > 0 ) {
            for ( size_t i = 0; i < len; ++i )
                vec.push_back( buf[i] );
        }
    }

    // vector_array v;
    // std::vector< char >::iterator it = vec.begin();

    // for ( const ACE_Message_Block * mblk = cdr.begin(); mblk; mblk = mblk->cont() )
    //     v.append( mblk );

    // adsequence::streambuf obuf( vec );
    // std::ostream o( &buf );
    // portable_binary_oarchive ar( o );
    // ar << v;

    return true;
}

bool
serializer::restore( ControlMethod::Method& m, const std::vector<char>& vec )
{
    ACE_MEM_Connector connector;
    ACE_MEM_Stream stream;
    ACE_MEM_Addr addr; //( ACE_DEFAUT_SERVER_PORT );

    if ( connector.connect( stream, addr.get_remote_addr() ) == -1 )
        return false;

    // size_t size = stream.recv( &vec[0], vec.size() );
    // assert( size == vec.size() );

    ACE_Message_Block mb( ACE_CDR::MAX_ALIGNMENT + vec.size() );
    // ACE_CDR::mb_aligin( &mb );
    mb.copy( &vec[0], vec.size() );

    TAO_InputCDR cdr( &mb );
    return cdr >> m;

    // adsequence::streambuf ibuf( vec );
    // std::istream in( &ibuf );
    // portable_binary_iarchive ar( in );
    
    // vector_array v;
    // ar >> v;

    // ACE_Message_Block mb;
    // for ( vector_array::vector_type::const_iterator it = v.begin(); it != v.end(); ++it ) {
    //     mb.
    // }
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
