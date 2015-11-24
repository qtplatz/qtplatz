/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#pragma once

#include "adcontrols_global.h"
#include <vector>

namespace adcontrols {

	class MassSpectrum;

	class ADCONTROLSSHARED_EXPORT waveform_filter {
	public:
		waveform_filter();

		struct ADCONTROLSSHARED_EXPORT fft {
            static bool lowpass_filter( MassSpectrum&, double freq = 100.0e6 /* 100MHz */ );
            static bool lowpass_filter( std::vector<double>&, double sampInterval /* seconds */, double freq = 100.0e6 /* 100MHz */ );
		};

		struct ADCONTROLSSHARED_EXPORT fft4g {
            static bool lowpass_filter( MassSpectrum&, double freq = 100.0e6 /* 100MHz */ );
            static bool lowpass_filter( size_t, double *, double sampInterval /* seconds */, double freq = 100.0e6 /* 100MHz */ );
		};

		struct ADCONTROLSSHARED_EXPORT fft4c {
            static bool lowpass_filter( MassSpectrum&, double freq = 100.0e6 /* 100MHz */ );
            static bool lowpass_filter( size_t, double *, double sampInterval /* seconds */, double freq = 100.0e6 /* 100MHz */ );
		};
        
        struct ADCONTROLSSHARED_EXPORT sg {
            static bool lowpass_filter( MassSpectrum&, double width = 5.0e-9 /* 5ns */);
            static bool lowpass_filter( size_t, double *, double sampInterval /* seconds */, double width = 5.0e-9 /* 5ns */ );
		};
	};

}

