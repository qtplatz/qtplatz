// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include "peak.hpp"
#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/string.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>

namespace adcontrols {

    class Baseline;

    class ADCONTROLSSHARED_EXPORT Peaks;

    class Peaks {
    public:
        ~Peaks();
        Peaks();
        Peaks( const Peaks& );
        Peaks& operator = ( const Peaks& );

        typedef Peak value_type;
        typedef std::vector< Peak > vector_type;

        void add( const Peak& );
        value_type& emplace_back( Peak&& t );

        inline operator const vector_type& () const      { return peaks_;	      }
        inline operator vector_type& ()                  { return peaks_;	      }
        inline vector_type::const_iterator begin() const { return peaks_.begin(); }
        inline vector_type::iterator begin()             { return peaks_.begin(); }
        inline vector_type::const_iterator end() const   { return peaks_.end();   }
        inline vector_type::iterator end()               { return peaks_.end();   }
        inline vector_type::reverse_iterator rbegin()    { return peaks_.rbegin();}
        inline vector_type::reverse_iterator rend()      { return peaks_.rend();  }
        inline size_t size() const                       { return peaks_.size();  }
        inline vector_type::iterator erase( vector_type::iterator pos )   { return peaks_.erase( pos ); }
        inline vector_type::iterator erase( vector_type::iterator beg, vector_type::iterator end )   { return peaks_.erase( beg, end ); }

        double areaTotal() const;
		void areaTotal( double );
		double heightTotal() const;
		void heightTotal( double );
		double noiseLevel() const;
		void noiseLevel( double );

        // algorithms
        vector_type::iterator find_first_peak( const Baseline& );
        vector_type::const_iterator find_first_peak( const Baseline& ) const;

        vector_type::const_iterator find_peakId( int32_t peakid ) const;
        vector_type::iterator find_peakId( int32_t peakid );

    private:
#if defined _MSC_VER
# pragma warning( disable: 4251 )
#endif
        vector_type peaks_;

        double areaTotal_;
        double heightTotal_;
        double noiseLevel_;

        friend ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag, boost::json::value&, const Peaks& );
        friend ADCONTROLSSHARED_EXPORT Peaks tag_invoke( const boost::json::value_to_tag< Peaks >&, const boost::json::value& jv );

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            (void)version;
	    ar & BOOST_SERIALIZATION_NVP( peaks_ );
	    ar & BOOST_SERIALIZATION_NVP( areaTotal_ );
	    ar & BOOST_SERIALIZATION_NVP( heightTotal_ );
	    ar & BOOST_SERIALIZATION_NVP( noiseLevel_ );
        }

    };

}
