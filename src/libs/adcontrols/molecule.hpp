/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "element.hpp"
#include <cstdlib>
#include <string>
#include <vector>

// interface for isotope cluster pattern

namespace adcontrols {

	namespace mol {

        struct ADCONTROLSSHARED_EXPORT isotope {
            double mass;
            double abundance;
            isotope( double m = 0, double a = 1.0 ) : mass(m), abundance(a) {}
        };

#if defined _MSC_VER
        template class ADCONTROLSSHARED_EXPORT std::vector < isotope > ;
        template class ADCONTROLSSHARED_EXPORT std::vector < element > ;
#endif

        class ADCONTROLSSHARED_EXPORT molecule {
        public:
            molecule();
            molecule( const molecule& t );

            inline operator bool () const { return !elements_.empty(); }

            molecule& operator << (const element&);
            molecule& operator << (element&& e);
            molecule& operator << (const isotope&);
            molecule& operator << (isotope&& i);
            int charge() const;
            void setCharge( int );
            void clear();
            typedef std::vector< isotope >::iterator       cluster_iterator ;
            typedef std::vector< isotope >::const_iterator const_cluster_iterator ;
            typedef std::vector< element >::iterator       elements_iterator ;
            typedef std::vector< element >::const_iterator const_elements_iterator ;
            cluster_iterator       cluster_begin()        { return cluster_.begin(); }
            const_cluster_iterator cluster_begin() const  { return cluster_.begin(); }
            cluster_iterator       cluster_end()          { return cluster_.end(); }
            const_cluster_iterator cluster_end() const    { return cluster_.end(); }
            elements_iterator       elements_begin()       { return elements_.begin(); }
            const_elements_iterator elements_begin() const { return elements_.begin(); }
            elements_iterator       elements_end()         { return elements_.end(); }
            const_elements_iterator elements_end() const   { return elements_.end(); }

            const std::vector< isotope >& cluster() const { return cluster_; }
            const std::vector< element >& elements() const { return elements_; }
            std::vector< isotope >::const_iterator max_abundant_isotope() const;

            /* return standard formula
             */
            std::string formula() const;

            /* display formula (usually, 'formula + adduct')
             */
            void set_display_formula( const std::string& );
            const std::string& display_formula() const;

            void setMass( double );
            double mass( bool handleCharge = true ) const;

        private:
            std::vector< isotope > cluster_;
            std::vector< element > elements_;
            int charge_;
            std::string display_formula_;
            double mass_;
        };

    }
}
