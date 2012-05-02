/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "chromatography.hpp"
#include <adcontrols/peaks.hpp>
#include <adcontrols/peak.hpp>

namespace chromatogr { namespace internal {

        class ChromatographyImpl {
        public:
            bool setup( const adcontrols::PeakMethod& );
            bool findPeaks( const adcontrols::Chromatogram& );
            void clear();
            inline const adcontrols::Peaks & getPeaks() const { return peaks_; }
        private:
            adcontrols::Peaks peaks_;
        };
    }
}

using namespace chromatogr;

Chromatography::~Chromatography()
{
    delete pImpl_;
}

Chromatography::Chromatography() : pImpl_( new internal::ChromatographyImpl() )
{
}

bool
Chromatography::operator()( const adcontrols::PeakMethod& method, const adcontrols::Chromatogram& c )
{
    pImpl_->clear();
    pImpl_->setup( method );
    return pImpl_->findPeaks( c );
}

const adcontrols::Peaks&
Chromatography::getPeaks() const 
{
    return pImpl_->getPeaks();
}

////
using namespace chromatogr::internal;
void
ChromatographyImpl::clear()
{
}

bool
ChromatographyImpl::setup( const adcontrols::PeakMethod& )
{
    return false;
}

bool
ChromatographyImpl::findPeaks( const adcontrols::Chromatogram& )
{
    return false;
}

