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
#include <adcontrols/countingmethod.hpp>
#include <adcontrols/elementalcompositionmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/mssimulatormethod.hpp>
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
        
        typedef boost::variant< CentroidMethod                  // 0
                                , ElementalCompositionMethod    // 1
                                , IsotopeMethod                 // 2
                                , MSCalibrateMethod             // 3
                                , MSChromatogramMethod          // 4
                                , MSLockMethod                  // 5
                                , PeakMethod                    // 6
                                , QuanCompounds                 // 7
                                , QuanMethod                    // 8
                                , TargetingMethod               // 9
                                , MSSimulatorMethod             // 10
                                , CountingMethod                // 11
                                > value_type;
        
        typedef std::vector< value_type > vector_type;
        typedef vector_type::iterator iterator;
        typedef vector_type::const_iterator const_iterator;
        
        template<class T> void appendMethod( const T& t ) { vec_.push_back( t ); }
        template<class T> ProcessMethod& operator << ( const T& t ) { vec_.push_back( t ); return *this; }
        template<class T> ProcessMethod& operator *= ( const T& t ) { remove<T>(); (*this) << t; return *this; } // remove if duplicate, and add
        ProcessMethod& operator *= ( const ProcessMethod& ); // remove duplicate and merge

        template<class T> inline bool is_type( const value_type& t ) const {
#if defined __clang__
            return std::strcmp( t.type().name(), typeid( T ).name() ) == 0;
#else
            return t.type() == typeid( T );
#endif
        }

        template<class T> const T* find() const {
            auto it = std::find_if( begin(), end(), [=] ( const value_type& t ){ return is_type<T>( t ); });
            if ( it != end() )
                return &boost::get<T>(*it);
            return 0;
        }

        template<class T> T* find() {
            auto it = std::find_if( begin(), end(), [=] ( const value_type& t ){ return is_type<T>( t ); });
            if ( it != end() )
                return &boost::get<T>(*it);
            return 0;
        }

        template<class T> void remove() {
            auto it = std::remove_if( begin(), end(), [=] ( const value_type& t ){ return is_type<T>( t ); });
            if ( it != end() )
                erase( it, end() );
        }
        
        const value_type& operator [] ( int ) const;
        value_type& operator [] ( int );
        void clear();
        void erase( iterator, iterator );
        size_t size() const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

    public:
        static bool archive( std::ostream&, const ProcessMethod& );
        static bool restore( std::istream&, ProcessMethod& );
        static bool xml_archive( std::wostream&, const ProcessMethod& );
        static bool xml_restore( std::wistream&, ProcessMethod& );

        const idAudit& ident() const { return ident_; }

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

