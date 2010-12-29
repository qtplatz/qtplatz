// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "adcontrols_global.h"

#include "timeutil.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
//#include <boost/serialization/string.hpp>
//#include <boost/serialization/vector.hpp>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT Baseline {
    public:
        virtual ~Baseline();
        Baseline();
        Baseline( const Baseline& );
        
        long baseId() const;
        void baseId( long );

        long startPos() const;
        void StartPos( long );
       
        long StopPos() const;
        void StopPos( long );

        bool isManuallyModified() const;
        void manuallyModified( bool );

        double startHeight() const;
        double stopHeight() const;
        seconds_t   startTime() const;
        seconds_t   stopTime() const;
    
        void startHeight( double );
        void stopHeight( double );
        void startTime( const seconds_t& );
        void stopTime( const seconds_t& );

        double height(int pos) const;
    private:
        bool manuallyModified_;
        long baseId_;
        long startPos_;
        long stopPos_;
        double startHeight_;
        double stopHeight_;
        seconds_t startTime_;
        seconds_t stopTime_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            if ( version >= 0 ) {
                ar & BOOST_SERIALIZATION_NVP( manuallyModified_ );
                ar & BOOST_SERIALIZATION_NVP( baseId_ );
                ar & BOOST_SERIALIZATION_NVP( startPos_ );
                ar & BOOST_SERIALIZATION_NVP( stopPos_ );
                ar & BOOST_SERIALIZATION_NVP( startHeight_ );
                ar & BOOST_SERIALIZATION_NVP( stopHeight_ );
                ar & BOOST_SERIALIZATION_NVP( startTime_ );
                ar & BOOST_SERIALIZATION_NVP( stopTime_ );
            }
        }

    };

}


