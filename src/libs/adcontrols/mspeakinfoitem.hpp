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
#include <string>
#include <cstdint>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    namespace internal { class CentroidProcessImpl; }

    class ADCONTROLSSHARED_EXPORT MSPeakInfoItem {
    public:
        ~MSPeakInfoItem(void);
        MSPeakInfoItem(void);
        MSPeakInfoItem( const MSPeakInfoItem& );

        double mass() const;

        void assign_mass( double );     // his will change centroid left/right values (for lock mass)
        void set_mass( double mass, double left, double right );

        double area() const;
        void set_area( double );
        double height() const;
        void set_height( double );
        double time( bool from_time = false ) const;
        void set_time( double, double left, double right, bool from_time = false );
        unsigned int peak_index() const;
        void set_peak_index( unsigned int );
        unsigned int peak_start_index() const;
        void set_peak_start_index( unsigned int );
        unsigned int peak_end_index() const;
        void set_peak_end_index( unsigned int );
        double base_height() const;
        void set_base_height( double );

        double centroid_left( bool time = false ) const;
        void set_centroid_left( double t, bool time );
        
        double centroid_right( bool time = false ) const;
        void set_centroid_right( double t, bool time );

        double centroid_threshold() const;
        void set_centroid_threshold( double );

        void set_width_hh_lr( double left, double right, bool time = false );

        double widthHH( bool time = false ) const;
        double hh_left_time() const;
        double hh_right_time() const;

        const std::string& formula() const;
        void formula( const std::string& );
        const std::wstring& annotation() const;
        void annotation( const std::wstring& );
        bool visible() const;
        void visible( bool );
        bool is_reference() const;
        void is_reference( bool );


        static bool xml_archive( std::wostream&, const MSPeakInfoItem& );
        static bool xml_restore( std::wistream&, MSPeakInfoItem& );

    private:
        uint32_t peak_index_;
        uint32_t peak_start_index_;
        uint32_t peak_end_index_;
        double base_height_;
        double mass_;
        double area_;
        double height_;

        double time_from_mass_;
        double time_from_time_;
        double HH_left_mass_;
        double HH_right_mass_;
        double HH_left_time_;
        double HH_right_time_;

        double centroid_left_mass_;
        double centroid_right_mass_;
        double centroid_left_time_;
        double centroid_right_time_;
        double centroid_threshold_; // absolute hight (for graphical rep)

        bool is_visible_;
        bool is_reference_;
        std::string formula_;
        std::wstring annotation_;

        friend class internal::CentroidProcessImpl;
        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version ) {
            ar  & BOOST_SERIALIZATION_NVP( peak_index_ )
                & BOOST_SERIALIZATION_NVP( peak_start_index_ )
                & BOOST_SERIALIZATION_NVP( peak_end_index_ )
                & BOOST_SERIALIZATION_NVP( base_height_ )
                & BOOST_SERIALIZATION_NVP( mass_ )
                & BOOST_SERIALIZATION_NVP( area_ )
                & BOOST_SERIALIZATION_NVP( height_ )
                & BOOST_SERIALIZATION_NVP( time_from_mass_ )
                & BOOST_SERIALIZATION_NVP( time_from_time_ )
                & BOOST_SERIALIZATION_NVP( HH_left_mass_ )
                & BOOST_SERIALIZATION_NVP( HH_right_mass_ )
                & BOOST_SERIALIZATION_NVP( HH_left_time_ )
                & BOOST_SERIALIZATION_NVP( HH_right_time_ )
                & BOOST_SERIALIZATION_NVP( centroid_left_mass_ )
                & BOOST_SERIALIZATION_NVP( centroid_right_mass_ )
                & BOOST_SERIALIZATION_NVP( centroid_left_time_ )
                & BOOST_SERIALIZATION_NVP( centroid_right_time_ )
                & BOOST_SERIALIZATION_NVP( centroid_threshold_ )
                ;
            if ( version >= 2 ) {
                ar & BOOST_SERIALIZATION_NVP( is_visible_ )
                    & BOOST_SERIALIZATION_NVP( is_reference_ )
                    & BOOST_SERIALIZATION_NVP( formula_ )
                    & BOOST_SERIALIZATION_NVP( annotation_ )
                    ;
            }
        }

    };

}

BOOST_CLASS_VERSION( adcontrols::MSPeakInfoItem, 2 )
