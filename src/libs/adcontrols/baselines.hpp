// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#pragma once

#include "adcontrols_global.h"

#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
//#include <boost/serialization/string.hpp>

namespace adcontrols {

    class Baseline;

    class ADCONTROLSSHARED_EXPORT Baselines {
    public:
        virtual ~Baselines();
        Baselines();
        Baselines( const Baselines& );

        typedef Baseline value_type;
        typedef std::vector< value_type > vector_type;

        inline operator const vector_type& () const      {  return baselines_;    }
        inline operator vector_type& ()                  {  return baselines_;    }
        int add(const Baseline& );
        int nextId( bool increment = false );
        inline vector_type::const_iterator begin() const { return baselines_.begin();  }
        inline vector_type::iterator begin()             { return baselines_.begin(); }
        inline vector_type::const_iterator end() const   { return baselines_.end(); }
        inline vector_type::iterator end()               { return baselines_.end(); }
        inline size_t size() const                       { return baselines_.size(); }
        inline vector_type::iterator erase( vector_type::iterator it )   { return baselines_.erase( it ); }
        inline vector_type::iterator erase( vector_type::iterator beg
            , vector_type::iterator end )  { return baselines_.erase( beg, end ); }

    private:
        int nextId_;
	int baseId_;
        vector_type baselines_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version ) {
	    (void)version;
	    ar & BOOST_SERIALIZATION_NVP( baseId_ );
	    ar & BOOST_SERIALIZATION_NVP( baselines_ );
        }
    };

}


