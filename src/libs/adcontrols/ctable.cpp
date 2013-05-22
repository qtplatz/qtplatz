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

#include "ctable.hpp"

using namespace adcontrols;

CTable::CTable()
{
}

CTable::CTable( const CTable& t ) : atoms_( t.atoms_ )
                                  , bonds_( t.bonds_ ) 
{
}

const CTable::atom_vector&
CTable::atoms() const
{
    return atoms_;
}

const CTable::bond_vector&
CTable::bonds() const
{
    return bonds_;
}

void
CTable::clear()
{
    atoms_.clear();
    bonds_.clear();
}

void
CTable::operator << ( const Atom& t )
{
    atoms_.push_back( t );
}

void
CTable::operator << ( const Bond& t )
{
    bonds_.push_back( t );
}

const CTable::Atom&
CTable::atom( int idx ) const
{
    return atoms_[ idx ];
}

const CTable::Bond&
CTable::bond( int idx ) const
{
    return bonds_[ idx ];
}

CTable::Atom::Atom() : x(0), y(0), z(0)
                     , mass_difference(0)
                     , charge(0)
                     , atom_stereo_parity(0)
                     , hydrogen_count(0)
                     , stereo_care_box(0)
                     , valence(0)
                     , H0(0)
                     , atom_atom_mapping_number(0)
                     , inversion_retention_flag(0)
                     , exact_change_flag(0)
{
}

CTable::Atom::Atom( const Atom& t ) : x(t.x), y(t.y), z(t.z)
                                    , symbol( t.symbol ) 
                                    , mass_difference(0)
                                    , charge( t.charge )
                                    , atom_stereo_parity( t.atom_stereo_parity )
                                    , hydrogen_count( t.hydrogen_count )
                                    , stereo_care_box( t.stereo_care_box )
                                    , valence( t.valence )
                                    , H0( t.H0 )
                                    , atom_atom_mapping_number( t.atom_atom_mapping_number )
                                    , inversion_retention_flag( t.inversion_retention_flag )
                                    , exact_change_flag( t.exact_change_flag )
{
}

CTable::Bond::Bond() : first_atom_number(0)
                     , second_atom_number(0)
                     , bond_type(0)
                     , bond_stereo(0)
                     , bond_topology(0)
                     , reacting_center_status(0)
{
}

CTable::Bond::Bond( const Bond& t ) : first_atom_number( t.first_atom_number )
                                    , second_atom_number( t.second_atom_number )
                                    , bond_type( t.bond_type )
                                    , bond_stereo( t.bond_stereo )
                                    , bond_topology( t.bond_topology )
                                    , reacting_center_status( t.reacting_center_status )
{
}
