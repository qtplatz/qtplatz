// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

namespace adcontrols {

    class IsotopeClusterImpl;
    class MassSpectrum;

    class IsotopeCluster {
    public:
        ~IsotopeCluster();
        IsotopeCluster();

        bool Compute( const std::wstring& stdFormula, double threshold, bool resInDa, double rp, MassSpectrum&, size_t& nPeaks );
        bool Compute( const std::wstring& stdFormula, double threshold, bool resInDa, double rp, MassSpectrum&, const std::wstring& adduct, size_t charges, size_t& nPeaks, bool bAccountForElectrons );

        void clearFormulae();
        bool addFormula( const std::wstring& stdFormula, const std::wstring& adduct, size_t chargeState, double relativeAmount );
        bool computeFormulae(double threshold, bool resInDa, double resolution,	MassSpectrum&, size_t& nPeaks, bool bAccountForElectrons );

    private:
        IsotopeClusterImpl * impl_;
    };

}


