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

#include "mol.hpp"
#include <compiler/disable_unused_parameter.h>
#include <openbabel/mol.h>
#include <boost/foreach.hpp>

using namespace adchem;

Mol::~Mol()
{
}

Mol::Mol() : obmol_( new OpenBabel::OBMol )
           , dirty_(true)
           , exactmass_( 0 )
{
}

Mol::Mol( const Mol& t ) : obmol_( t.obmol_ )
	                     , dirty_( t.dirty_ )
						 , exactmass_( t.exactmass_ )
						 , formula_( t.formula_ )
{
}

void
Mol::update()
{
	exactmass_ = obmol_->GetExactMass();
	formula_ = obmol_->GetFormula();
	dirty_ = false;
}

void
Mol::obmol( OpenBabel::OBMol& omol )
{
    obmol_.reset( new OpenBabel::OBMol( omol ) );
    update();
}

const OpenBabel::OBMol *
Mol::obmol() const
{
    return obmol_.get();
}

Mol&
Mol::operator = ( const Mol& t )
{
    obmol_ = t.obmol_; // CAUTION -- This shareing the instance
	dirty_ = t.dirty_;
	exactmass_ = t.exactmass_;
	formula_ = t.formula_;
	return * this;
}

void
Mol::setAttribute( const char * key, const char * value )
{
	OpenBabel::OBPairData * data = new OpenBabel::OBPairData();
	data->SetAttribute( key );
    data->SetValue( value );
	obmol_->SetData( data ); // will be deleted when OBMol's destractor was called
}

Mol::operator OpenBabel::OBMol& ()
{
	return *obmol_;
}

Mol::operator const OpenBabel::OBMol& () const
{
	return *obmol_;
}

const char *
Mol::getFormula() const
{
	if ( dirty_ )
		const_cast< Mol * >(this)->update();
    return formula_.c_str();
}

double
Mol::getExactMass( bool implicitH ) const
{
	if ( dirty_ )
		const_cast< Mol * >(this)->update();
	return exactmass_;
}

attributes
Mol::attributes() const
{
    adchem::attributes attrs;
    for ( OpenBabel::OBDataIterator it = obmol_->BeginData(); it != obmol_->EndData(); ++it ) {
        const OpenBabel::OBGenericData& data = **it;
        if ( data.GetDataType() == OpenBabel::OBGenericDataType::PairData ) {
            const OpenBabel::OBPairData& pair = static_cast< const OpenBabel::OBPairData& >( data );
            adchem::attribute attr;
            attr.key( pair.GetAttribute().c_str() );
            attr.value( pair.GetValue().c_str() );
            attrs << attr;
        }
    }
    return attrs;
}

