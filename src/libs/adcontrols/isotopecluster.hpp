// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <string>

namespace adcontrols {

    class IsotopeClusterImpl;
    class MassSpectrum;

    class ADCONTROLSSHARED_EXPORT IsotopeCluster {
    public:
        ~IsotopeCluster();
        IsotopeCluster();

		static bool isotopeDistribution( MassSpectrum& ms, const std::wstring& formula, size_t charges = 1, bool accountElectron = true );

        bool Compute( const std::wstring& stdFormula, double threshold, bool resInDa, double rp, MassSpectrum&, size_t& nPeaks );
        bool Compute( const std::wstring& stdFormula, double threshold, bool resInDa, double rp, MassSpectrum&, const std::wstring& adduct, size_t charges, size_t& nPeaks, bool bAccountForElectrons );

        void clearFormulae();
        bool addFormula( const std::wstring& stdFormula, const std::wstring& adduct, size_t chargeState, double relativeAmount );
        bool computeFormulae(double threshold, bool resInDa, double resolution,	MassSpectrum&, size_t& nPeaks, bool bAccountForElectrons, double sf );

    private:
        IsotopeClusterImpl * impl_;
    };

}


