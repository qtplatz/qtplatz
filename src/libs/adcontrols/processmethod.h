// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include <boost/variant.hpp>
#include <boost/smart_ptr.hpp>
#include <vector>

namespace adcontrols {

    class CentroidMethod;
    class IsotopeMethod;
    class ElementalCompositionMethod;
    class MSCalibrateMethod;
    class TargetingMethod;

    class ADCONTROLSSHARED_EXPORT ProcessMethod {
    public:
        ~ProcessMethod();
        ProcessMethod();
        ProcessMethod( const ProcessMethod& );

        typedef boost::variant< CentroidMethod
                              , IsotopeMethod
                              , ElementalCompositionMethod
                              , MSCalibrateMethod
                              , TargetingMethod 
                              > value_type;

        typedef std::vector< value_type > vector_type;

        //void appendMethod( const CentroidMethod& );
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

    private:

# pragma warning(disable:4251)
        vector_type vec_;
//# pragma warning(default:4251)
    };

}


