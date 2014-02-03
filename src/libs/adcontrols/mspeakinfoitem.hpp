// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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
        void mass( double ); // re-calibration will update only mass
        double area() const;
        double height() const;
        double time( bool time = false ) const;
        unsigned int peak_index() const;
        void peak_index( int );
        unsigned int peak_start_index() const;
        void peak_start_index( unsigned int );
        unsigned int peak_end_index() const;
        void peak_end_index( unsigned int );
        double base_height() const;
        void base_height( double );

        double centroid_left( bool time = false ) const;
        double centroid_right( bool time = false ) const;
        double centroid_threshold() const;
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

        // re-assign mass (usually call from calibration process)
        void assign_mass( double mass, double left, double right, double hhLeft, double hhRight );

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
            ar  & peak_index_
                & peak_start_index_
                & peak_end_index_
                & base_height_
                & mass_
                & area_
                & height_
                & time_from_mass_
                & time_from_time_
                & HH_left_mass_
                & HH_right_mass_
                & HH_left_time_
                & HH_right_time_
                & centroid_left_mass_
                & centroid_right_mass_
                & centroid_left_time_
                & centroid_right_time_
                & centroid_threshold_
                ;
            if ( version >= 2 ) {
                ar & is_visible_
                    & is_reference_
                    & formula_
                    & annotation_
                    ;
            }
        }

    };

}

BOOST_CLASS_VERSION( adcontrols::MSPeakInfoItem, 2 )
