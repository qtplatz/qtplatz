/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <compiler/disable_dll_interface.h>
#include <memory>

namespace adcontrols {

    class MSQPeaks;

    class ADCONTROLSSHARED_EXPORT MSQPeak {
    public:
        ~MSQPeak();
        MSQPeak( const std::wstring& dataGuid = L"", uint32_t idx = 0, uint32_t fcn = 0, MSQPeaks * pks = 0 );
        MSQPeak( const MSQPeak& );
        void time( double t );
        double time() const;
        void mass( double );
        double mass() const;
        void intensity( double );
        double intensity() const;
        void mode( int32_t );
        int32_t mode() const;
        void amount( double );
        double amount() const;
        void istd( uint32_t );
        uint32_t istd() const;
        void isSTD( bool );
        bool isSTD() const;
        void isIS( bool );
        bool isIS() const;
        void idx( uint32_t );
        void fcn( uint32_t );
        uint32_t idx() const;
        uint32_t fcn() const;
        void protocol( const std::string& );
        const std::string& protocol() const;
        void componentId( const std::wstring& );
        const std::wstring& componentId() const;
        void formula( const std::string& );
        const std::string& formula() const;
        void description( const std::string& );
        const std::string& description() const;

        const std::wstring& dataGuid() const;
        
    private:
        uint32_t idx_;  // peak index within a 'segment' spectrum
        uint32_t fcn_;  // protocol (function) id in a spectrum, which may contain segments
        double time_;
        double mass_;
        double intensity_;
        uint32_t mode_;
        double amount_;
        uint32_t istd_;  // zero := not using istd | not an ISTD
        bool isSTD_;
        bool isIS_;
        std::wstring dataGuid_;
        std::wstring compId_;  // component name as primary-key
        std::string description_;
        std::string formula_;
        std::string proto_;
        std::weak_ptr< MSQPeaks > container_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version) {
            (void)(version);
            ar & dataGuid_
                & idx_
                & fcn_
                & time_
                & mass_
                & intensity_
                & mode_
                & amount_
                & istd_
                & isSTD_
                & isIS_
                & idx_
                & fcn_
                & proto_
                & compId_
                & formula_
                & description_
                ;
        }

    };
}


