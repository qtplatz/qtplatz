/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quancandidate.hpp"
#include "quanchromatogram.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <tuple>

using namespace quan;

QuanCandidate::QuanCandidate( const std::string& formula
                              , const std::string& reader_objtext
                              , double exactMass
                              , double matchedMass
                              , std::pair< double, double > fwhm_range
                              , double tRetention
                              , uint32_t fcn
                              , uint32_t idx
                              , std::shared_ptr< adcontrols::MassSpectrum > profile
                              , std::shared_ptr< adcontrols::MassSpectrum > centroid
                              , std::shared_ptr< adcontrols::MassSpectrum > filtered
                              , std::shared_ptr< adcontrols::MSPeakInfo > mspkinfo ) : exactMass_( exactMass )
                                                                                     , matchedMass_( matchedMass )
                                                                                     , fwhm_range_( fwhm_range )
                                                                                     , tR_( tRetention )
                                                                                     , formula_( formula )
                                                                                     , fcn_( fcn )
                                                                                     , idx_( idx )
                                                                                     , profile_( profile )
                                                                                     , centroid_( centroid )
                                                                                     , filtered_( filtered )
                                                                                     , mspkinfo_( mspkinfo )
                                                                                     , reader_objtext_( reader_objtext )

{
}

QuanCandidate::QuanCandidate( const QuanCandidate& t ) : exactMass_( t.exactMass_ )
                                                       , matchedMass_( t.matchedMass_ )
                                                       , fwhm_range_( t.fwhm_range_ )
                                                       , tR_( t.tR_ )
                                                       , formula_( t.formula_ )
                                                       , fcn_( t.fcn_ )
                                                       , idx_( t.idx_ )
                                                       , profile_( t.profile_ )
                                                       , centroid_( t.centroid_ )
                                                       , filtered_( t.filtered_ )
                                                       , mspkinfo_( t.mspkinfo_ )
                                                       , reader_objtext_( t.reader_objtext_ )
{
}

QuanCandidate&
QuanCandidate::operator = ( const QuanCandidate& t )
{
    exactMass_ =  t.exactMass_;
    matchedMass_ =  t.matchedMass_;
    fwhm_range_ = t.fwhm_range_;
    tR_ = t.tR_;
    formula_ =  t.formula_;
    fcn_ =  t.fcn_ ;
    idx_ = t.idx_;
    profile_ = t.profile_;
    centroid_ = t.centroid_;
    filtered_ = t.filtered_;
    mspkinfo_ = t.mspkinfo_;
    reader_objtext_ = t.reader_objtext_;
    return *this;
}

const std::string&
QuanCandidate::formula() const
{
    return formula_;
}

