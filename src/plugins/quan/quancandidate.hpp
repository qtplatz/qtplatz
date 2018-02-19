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

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <tuple>

namespace adcontrols { class MassSpectrum; class MSPeakInfo; }

namespace quan {

    class QuanChromatogram;
    class QuanTarget;

    /**
     * QuanTarget lists all possible masses and formulae represent for a user specified compunds (formula)
     */

    class QuanCandidate {
        
    public:
        QuanCandidate( const QuanCandidate& );
        QuanCandidate& operator = ( const QuanCandidate& );

        QuanCandidate( const std::string& formula
                       , const std::string& reader_objtext
                       , double exactMass
                       , double matchedMass
                       , std::pair< double, double > fwhm_range
                       , double tRetention
                       , uint32_t fcn
                       , uint32_t idx = ( -1 )
                       , std::shared_ptr< adcontrols::MassSpectrum > profile = 0
                       , std::shared_ptr< adcontrols::MassSpectrum > centroid = 0
                       , std::shared_ptr< adcontrols::MassSpectrum > filtered = 0
                       , std::shared_ptr< adcontrols::MSPeakInfo > mspkinfo = 0 );
        
        const std::string& formula() const; // core formula

        double width() const { return fwhm_range_.second - fwhm_range_.first; }

        double exactMass() const { return exactMass_; }

        double matchedMass() const { return matchedMass_; }

        uint32_t fcn() const { return fcn_; }
        uint32_t idx() const { return idx_; }

        std::shared_ptr< adcontrols::MassSpectrum > profile() const { return profile_; }

        std::shared_ptr< adcontrols::MassSpectrum > centroid() const { return centroid_; }

        std::shared_ptr< adcontrols::MassSpectrum > filtered() const { return filtered_; }

        std::shared_ptr< adcontrols::MSPeakInfo > mspkinfo() const { return mspkinfo_; }

        std::shared_ptr< QuanChromatogram > quanChromatogram() { return qcrms_; }
        void setQuanChromatogram( std::shared_ptr< QuanChromatogram > t ) { qcrms_ = t; }
        void setReferenceDataGuid( const std::wstring& guid ) { dataGuid_ = guid; }
        
    private:
        double exactMass_;
        double matchedMass_;
        std::pair< double, double > fwhm_range_;
        double tR_;
        std::string formula_;
        uint32_t fcn_;
        uint32_t idx_;
        std::shared_ptr< adcontrols::MassSpectrum > profile_;
        std::shared_ptr< adcontrols::MassSpectrum > centroid_;
        std::shared_ptr< adcontrols::MassSpectrum > filtered_;
        std::shared_ptr< adcontrols::MSPeakInfo > mspkinfo_;
        std::shared_ptr< QuanChromatogram > qcrms_;
        std::wstring dataGuid_;
        std::string reader_objtext_;
    };

}

