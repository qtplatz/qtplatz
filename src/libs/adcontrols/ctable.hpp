/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#ifndef CTABLE_HPP
#define CTABLE_HPP

#include <vector>
#include <string>
#include "adcontrols_global.h"

#if defined _MSC_VER
# pragma warning( push )
# pragma warning( disable: 4251 )
#endif

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT CTable {
    public:
        CTable();
        CTable( const CTable& );

        struct ADCONTROLSSHARED_EXPORT Atom {
            Atom();
            Atom( const Atom& );
            double x, y, z;

            std::string symbol;
            int mass_difference;
            int charge;
            int atom_stereo_parity;
            int hydrogen_count;
            int stereo_care_box;
            int valence;
            int H0;
            int atom_atom_mapping_number;
            int inversion_retention_flag;
            int exact_change_flag;
        };

        struct ADCONTROLSSHARED_EXPORT Bond {
            Bond();
            Bond( const Bond& );
            int first_atom_number;
            int second_atom_number;
            int bond_type;
            int bond_stereo;
            int bond_topology;
            int reacting_center_status;
        };
        typedef std::vector< Atom > atom_vector;
        typedef std::vector< Bond > bond_vector;

    public:
        const atom_vector& atoms() const;
        const bond_vector& bonds() const;
        void operator << ( const Atom& );
        void operator << ( const Bond& );
        const Atom& atom( int ) const;
        const Bond& bond( int ) const;
        void clear();
        bool empty() const { return atoms_.empty(); }

    private:

        atom_vector atoms_;
        bond_vector bonds_;
    };
}

#if defined _MSC_VER
# pragma warning( pop )
#endif

#endif // CTABLE_HPP
