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

#include "isotopecluster.h"
#include "tableofelements.h"
#include "import_sacontrols.h"

using namespace adcontrols;
using namespace SACONTROLSLib;

namespace adcontrols {

    class IsotopeClusterImpl {
    public:
        ~IsotopeClusterImpl();
        IsotopeClusterImpl();

        SACONTROLSLib::ISAIsoClusterPtr pi_;
    };
}

IsotopeCluster::~IsotopeCluster()
{
    delete impl_;
}

IsotopeCluster::IsotopeCluster() : impl_( new IsotopeClusterImpl )
{
}

bool
IsotopeCluster::Compute( const std::wstring& formula, double threshold, bool resInDa, double rp, MassSpectrum& ms, size_t& nPeaks )
{
    SACONTROLSLib::ISAMassSpectrum5Ptr pims;
    if ( pims.CreateInstance( SACONTROLSLib::CLSID_SAMassSpectrum ) != S_OK )
        return false;

    using namespace adcontrols::internal;
    long n;
    if ( impl_->pi_->Compute( _bstr_t( formula.c_str() ), threshold, variant_bool::to_variant( resInDa ), rp, pims, &n ) == S_OK ) {
        nPeaks = n;
        return true;
    }
    return false;
}

bool
IsotopeCluster::Compute( const std::wstring& formula, double threshold, bool resInDa, double rp, MassSpectrum&, const std::wstring& adduct, size_t charges, size_t& nPeaks, bool bAccountForElectrons )
{
    SACONTROLSLib::ISAMassSpectrum5Ptr pims;
    if ( pims.CreateInstance( SACONTROLSLib::CLSID_SAMassSpectrum ) != S_OK )
        return false;

    using namespace adcontrols::internal;
    long n;
    if ( impl_->pi_->ComputeCharged( _bstr_t( formula.c_str() )
                                    , threshold
                                    , variant_bool::to_variant( resInDa )
                                    , rp
                                    , pims
                                    , _bstr_t( adduct.c_str() )
                                    , charges
                                    , &n
                                    , variant_bool::to_variant( bAccountForElectrons ) ) == S_OK ) {
        nPeaks = n;
        return true;
    }
    return false;
}

void
IsotopeCluster::clearFormulae()
{
    impl_->pi_->ClearFormulae();
}

bool
IsotopeCluster::addFormula( const std::wstring& formula, const std::wstring& adduct, size_t chargeState, double relativeAmount )
{
    if ( impl_->pi_->AddFormula( _bstr_t( formula.c_str() ), _bstr_t( adduct.c_str() ), chargeState, relativeAmount ) == S_OK )
        return true;
    return false;
}

bool
IsotopeCluster::computeFormulae(double threshold, bool resInDa, double rp,	MassSpectrum&, size_t& nPeaks, bool bAccountForElectrons )
{
    nPeaks = 0;
    SACONTROLSLib::ISAMassSpectrum5Ptr pims;
    if ( pims.CreateInstance( SACONTROLSLib::CLSID_SAMassSpectrum ) != S_OK )
        return false;

    using namespace adcontrols::internal;
    long n;
    if ( impl_->pi_->ComputeFormulae( threshold
                                    , variant_bool::to_variant( resInDa )
                                    , rp, pims, &n, variant_bool::to_variant( bAccountForElectrons ) ) == S_OK ) {
        nPeaks = n;
        return true;
    }
    return false;
}

//////////////////////

IsotopeClusterImpl::~IsotopeClusterImpl()
{
}

IsotopeClusterImpl::IsotopeClusterImpl()
{
    TableOfElements * toe = TableOfElements::instance();
    if ( pi_.CreateInstance( SACONTROLSLib::CLSID_SAIsoCluster ) == S_OK ) {
        pi_->ElementData = *toe;
    }
}