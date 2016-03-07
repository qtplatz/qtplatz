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

#include "quanresponse.hpp"

using namespace adcontrols;

QuanResponse::~QuanResponse()
{
}

QuanResponse::QuanResponse() : idx_( 0 )
                             , fcn_( 0 )
                             , intensity_( 0 )
                             , amounts_( 0 )
                             , mass_( 0 )
                             , tR_( 0 )
{
}

QuanResponse::QuanResponse( const QuanResponse& t ) : idx_( t.idx_ )
                                                    , idTable_( t.idTable_ )
                                                    , idCompound_( t.idCompound_ )
                                                    , dataGuid_( t.dataGuid_ )
                                                    , fcn_( t.fcn_ )
                                                    , intensity_( t.intensity_ )
                                                    , amounts_( t.amounts_ )
                                                    , mass_( t.mass_ )
                                                    , tR_( t.tR_ )
                                                    , formula_( t.formula_ )
{
}
