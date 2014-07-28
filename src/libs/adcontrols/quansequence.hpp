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
#include "quansample.hpp"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/uuid/uuid_serialize.hpp>
#include <cstdint>
#include <memory>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace adcontrols {

#if defined _MSC_VER
    template class ADCONTROLSSHARED_EXPORT std::vector < QuanSample > ;
#endif

    class ADCONTROLSSHARED_EXPORT QuanSequence  {
    public:
        ~QuanSequence();
        QuanSequence();
        QuanSequence( const QuanSequence& );

        static const wchar_t * dataClass() { return L"adcontrols::QuanSequence"; }

        typedef QuanSample value_type;
        typedef std::vector< value_type > vector_type;
        typedef std::vector< QuanSample >::iterator iterator_type;
        
        std::vector< QuanSample >::iterator begin() { return samples_.begin(); }
        std::vector< QuanSample >::iterator end() { return samples_.end(); }
        std::vector< QuanSample >::const_iterator begin() const { return samples_.begin(); }
        std::vector< QuanSample >::const_iterator end() const { return samples_.end(); }
        size_t size() const { return samples_.size(); }
        QuanSequence& operator << ( const QuanSample& t );

        void uuid( const boost::uuids::uuid& );
        const boost::uuids::uuid& uuid() const;

        const wchar_t * outfile() const { return outfile_.c_str(); }
        void outfile( const wchar_t * file ) { outfile_ = file; }

        static bool archive( std::ostream&, const QuanSequence& );
        static bool restore( std::istream&, QuanSequence& );

    private:
        boost::uuids::uuid uuid_;
        std::vector< QuanSample > samples_;
        std::wstring outfile_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;
            ar & BOOST_SERIALIZATION_NVP( uuid_ )
                & BOOST_SERIALIZATION_NVP( samples_ )
                & BOOST_SERIALIZATION_NVP( outfile_ )
                ;
        }

    };
    typedef std::shared_ptr<QuanSequence> QuanSequencePtr;   

}

#endif // QUANSEQUENCE_HPP
