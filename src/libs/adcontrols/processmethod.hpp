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
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/elementalcompositionmethod.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/idaudit.hpp>
#include <boost/variant.hpp>
#include <vector>


#include <compiler/disable_dll_interface.h>

namespace boost { namespace serialization { class access; } }


namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT ProcessMethod {
    public:
        ~ProcessMethod();
        ProcessMethod();
        ProcessMethod( const ProcessMethod& );
        template< class T > ProcessMethod( const T& v ) {  vec_.push_back( v );   }
        
        static const wchar_t * dataClass() { return L"adcontrols::ProcessMethod"; }
        
        typedef boost::variant< CentroidMethod
                                , IsotopeMethod
                                , ElementalCompositionMethod
                                , MSCalibrateMethod
                                , TargetingMethod 
                                , PeakMethod 
                                , MSChromatogramMethod
                                , QuanCompounds
                                , QuanMethod
                                > value_type;
        
        typedef std::vector< value_type > vector_type;
        
        template<class T> void appendMethod( const T& );
        template<class T> const T* find() const;
        
        const value_type& operator [] ( int ) const;
        value_type& operator [] ( int );
        void clear();

        size_t size() const;
        vector_type::iterator begin();
        vector_type::iterator end();
        vector_type::const_iterator begin() const;
        vector_type::const_iterator end() const;

    public:
        static bool archive( std::ostream&, const ProcessMethod& );
        static bool restore( std::istream&, ProcessMethod& );

    private:
        vector_type vec_;
        idAudit ident_;

        friend class boost::serialization::access;
        template<class Archiver> void serialize(Archiver& ar, const unsigned int version) {
            ar & BOOST_SERIALIZATION_NVP( vec_ );
            if ( version >= 1 )
                ar & BOOST_SERIALIZATION_NVP( ident_ );
        }

    };

    typedef std::shared_ptr<ProcessMethod> ProcessMethodPtr;

}

BOOST_CLASS_VERSION( adcontrols::ProcessMethod, 1 )

