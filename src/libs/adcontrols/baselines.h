// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

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
        long add(const Baseline& );
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
# pragma warning( disable: 4251 )
        vector_type baselines_;
# pragma warning( default: 4251 )


        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP( baseId_ );
                ar & BOOST_SERIALIZATION_NVP( baselines_ );
            }
        }
    };

}


