/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef QUANSEQUENCE_HPP
#define QUANSEQUENCE_HPP

#include "adcontrols_global.h"
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <memory>
#include <vector>

namespace boost { 
    namespace uuids { struct uuid; }
    namespace serialization { class access; }
}


namespace adcontrols {

    class QuanSample;
    class idAudit;

    class ADCONTROLSSHARED_EXPORT QuanSequence  {
    public:
        ~QuanSequence();
        QuanSequence();
        QuanSequence( const QuanSequence& );

        static const wchar_t * dataClass() { return L"adcontrols::QuanSequence"; }

        typedef QuanSample value_type;
        typedef std::vector< value_type > vector_type;
        typedef std::vector< QuanSample >::iterator iterator;
        typedef std::vector< QuanSample >::const_iterator const_iterator;
        
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;
        size_t size() const;
        QuanSequence& operator << ( const QuanSample& t );

        const boost::uuids::uuid& uuid() const;

        const idAudit& ident() const;

        const wchar_t * outfile() const;
        void outfile( const wchar_t * file );

        const wchar_t * filename() const;
        void filename( const wchar_t * file );

        static bool archive( std::ostream&, const QuanSequence& );
        static bool restore( std::istream&, QuanSequence& );
        static bool xml_archive( std::wostream&, const QuanSequence& );
        static bool xml_restore( std::wistream&, QuanSequence& );

    private:

#   if  defined _MSC_VER
#   pragma warning(disable:4251)
#   endif
        class impl;
        std::unique_ptr< impl > impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

    typedef std::shared_ptr<QuanSequence> QuanSequencePtr;   

}

BOOST_CLASS_VERSION( adcontrols::QuanSequence, 2 )

#endif // QUANSEQUENCE_HPP
