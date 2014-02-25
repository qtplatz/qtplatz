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

#include "digestedpeptides.hpp"
#include <adprot/protein.hpp>
#include <adprot/peptides.hpp>
#include <adprot/peptide.hpp>
#include <adprot/protease.hpp>

using namespace adprot;

digestedPeptides::digestedPeptides() : protein_( std::make_shared< adprot::protein >() )
                                     , protease_( std::make_shared< adprot::protease >() )
                                     , peptides_( std::make_shared< adprot::peptides >() )
{
}

digestedPeptides::digestedPeptides( const digestedPeptides& t ) : protein_( t.protein_ )
                                                                , protease_( t.protease_ )
                                                                , peptides_( t.peptides_ )
{
}

digestedPeptides::digestedPeptides( const adprot::protein& protein
                                    , const adprot::protease& protease ) : protein_( std::make_shared< adprot::protein >( protein ) )
                                                                         , protease_( std::make_shared< adprot::protease >( protease ) )
                                                                         , peptides_( std::make_shared< adprot::peptides >() )
{
}

const protein&
digestedPeptides::protein() const
{
    return *protein_;
}

const
protease&
digestedPeptides::protease() const
{
    return *protease_;
}

const peptides&
digestedPeptides::peptides() const
{
    return *peptides_;
}

digestedPeptides&
digestedPeptides::operator << ( const peptide& p )
{
    *peptides_ << p;
    return *this;
}

