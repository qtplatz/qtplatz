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

#ifndef CHROMATOGRAPHY_HPP
#define CHROMATOGRAPHY_HPP

#include "chromatogr_global.hpp"

namespace adcontrols {
    class Chromatogram;
    class Peak;
    class Peaks;
	class Baseline;
	class Baselines;
    class PeakMethod;
}

namespace chromatogr {

    namespace internal { class ChromatographyImpl; }

    class CHROMATOGRSHARED_EXPORT Chromatography {
    public:
        ~Chromatography();
        Chromatography();
		Chromatography( const adcontrols::PeakMethod& );

        // peak finder
        bool operator()( const adcontrols::PeakMethod&, const adcontrols::Chromatogram& );
        bool operator()( const adcontrols::Chromatogram& );
		const adcontrols::Baselines& getBaselines() const;
        const adcontrols::Peaks& getPeaks() const;
	private:
        internal::ChromatographyImpl * pImpl_;
    };
    
}

#endif // CHROMATOGRAPHY_HPP
