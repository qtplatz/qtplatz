/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "quanmethodcomplex.hpp"
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/portable_binary_oarchive.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adpublisher/document.hpp>
#include <boost/filesystem/path.hpp>
#include <sstream>

using namespace quan;

QuanMethodComplex::~QuanMethodComplex()
{
}

QuanMethodComplex::QuanMethodComplex() : quanMethod_( std::make_shared< adcontrols::QuanMethod >() )
                                       , quanCompounds_( std::make_shared< adcontrols::QuanCompounds >() )
                                       , procMethod_( std::make_shared< adcontrols::ProcessMethod >() )
                                       , docTemplate_( std::make_shared< adpublisher::document >() )
{
}

QuanMethodComplex::QuanMethodComplex( const QuanMethodComplex& t ) : ident_( t.ident_ )
                                                                   , quanMethod_( t.quanMethod_ )
                                                                   , quanCompounds_( t.quanCompounds_ )
                                                                   , procMethod_( t.procMethod_ )
                                                                   , docTemplate_( t.docTemplate_ )
{
}

std::shared_ptr< adcontrols::QuanMethod >
QuanMethodComplex::quanMethod()
{
    return quanMethod_;
}

std::shared_ptr< adcontrols::ProcessMethod >
QuanMethodComplex::procMethod()
{
    return procMethod_;
}

std::shared_ptr< adcontrols::QuanCompounds >
QuanMethodComplex::quanCompounds()
{
    return quanCompounds_;
}

std::shared_ptr< adpublisher::document >
QuanMethodComplex::docTemplate()
{
    return docTemplate_;
}

void
QuanMethodComplex::operator = ( std::shared_ptr< adcontrols::QuanMethod >& ptr )
{
    quanMethod_ = ptr;
}

void
QuanMethodComplex::operator = ( std::shared_ptr< adcontrols::ProcessMethod >& ptr )
{
    procMethod_ = ptr;
}

void
QuanMethodComplex::operator = ( std::shared_ptr< adcontrols::QuanCompounds > & ptr )
{
    quanCompounds_ = ptr;
}

void
QuanMethodComplex::operator = ( std::shared_ptr< adpublisher::document > & ptr )
{
    docTemplate_ = ptr;
}

const wchar_t *
QuanMethodComplex::filename() const
{
    return quanMethod_->quanMethodFilename();
}

void
QuanMethodComplex::setFilename( const wchar_t * filename )
{
    boost::filesystem::path path(filename);
    quanMethod_->quanMethodFilename( path.generic_wstring().c_str() );
}

bool
QuanMethodComplex::archive( std::ostream& os, const QuanMethodComplex& m )
{
    portable_binary_oarchive ar( os );
    ar << m;
    return true;
}


//static bool restore( std::istream&, QuanMethodComplex& );
bool
QuanMethodComplex::restore( std::istream& is, QuanMethodComplex& m )
{
    portable_binary_iarchive ar( is );
    ar >> m;
    return true;
}

