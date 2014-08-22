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

#ifndef MSPEAKINFO_HPP
#define MSPEAKINFO_HPP

#pragma once


#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>
#include <memory>
#include <vector>
#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class MSPeakInfoItem;

    class ADCONTROLSSHARED_EXPORT MSPeakInfo {
    public:
        MSPeakInfo( int mode = 0 );
        MSPeakInfo( const MSPeakInfo& );

        static const wchar_t * dataClass() { return L"adcontrols::MSPeakInfo"; }

        typedef std::vector< MSPeakInfoItem >::iterator iterator;
        typedef std::vector< MSPeakInfoItem >::const_iterator const_iterator;
        
        inline iterator begin() { return vec_.begin(); }
        inline iterator end() { return vec_.end(); }
        inline const_iterator begin() const { return vec_.begin(); }
        inline const_iterator end() const { return vec_.end(); }
        size_t size() const;
        size_t total_size() const;
        void clear();

        MSPeakInfo& operator << ( const MSPeakInfoItem& );

        void addSegment( const MSPeakInfo& );
        MSPeakInfo& getSegment( size_t fcn );
        const MSPeakInfo& getSegment( size_t fcn ) const;
        size_t numSegments() const;
        int mode() const;
        void mode( int );
        void protocol( int32_t protId, int32_t nProtocols );
        int32_t protocolId() const;
        int32_t nProtocols() const;
        MSPeakInfo * findProtocol( int32_t );
        const MSPeakInfo * findProtocol( int32_t ) const;
        void clearSegments();

        bool trim( MSPeakInfo&, const std::pair<double, double>& range ) const;

        static bool archive( std::ostream&, const MSPeakInfo& );
        static bool restore( std::istream&, MSPeakInfo& );

    private:
        std::vector< MSPeakInfoItem > vec_;
        std::vector< MSPeakInfo > siblings_;
        int32_t mode_;
        int32_t protocolId_;
        int32_t nProtocols_;

        friend class boost::serialization::access;
        template<class Archive> void serialize(Archive& ar, const unsigned int version ) {
            if ( version >= 2 )
                ar & BOOST_SERIALIZATION_NVP( mode_ );
            if ( version >= 3 )
                ar & BOOST_SERIALIZATION_NVP( protocolId_ ) & BOOST_SERIALIZATION_NVP( nProtocols_ );
            ar  & BOOST_SERIALIZATION_NVP( vec_ )
                & BOOST_SERIALIZATION_NVP( siblings_ )
                ;
        }

    };

	typedef std::shared_ptr< MSPeakInfo > MSPeakInfoPtr;
}

BOOST_CLASS_VERSION( adcontrols::MSPeakInfo, 3 )

#endif // MSPEAKINFO_HPP
