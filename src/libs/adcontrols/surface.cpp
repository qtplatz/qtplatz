// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC
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

#include "surface.hpp"
#include "massspectrum.hpp"
#include "msproperty.hpp"

using namespace adcontrols;

Surface::Surface()
{
}

void
Surface::operator << ( std::shared_ptr< adcontrols::MassSpectrum >&& ms )
{
    if ( yValues_.empty() ) {
        yValues_.resize( ms->size() );
        std::copy( ms->getMassArray(), ms->getMassArray() + ms->size(), yValues_.begin() );
    }
    xValues_.emplace_back( ms->getMSProperty().timeSinceInjection() );

    data_.emplace_back( ms );
}

boost::numeric::ublas::matrix<double>
Surface::getSurface() const
{
    if ( !data_.empty() ) {
        auto ysize = data_[0]->size();
        auto xsize = data_.size();
        boost::numeric::ublas::matrix<double> m( xsize, ysize );
        for ( size_t i = 0; i < data_.size(); ++i ) {
            const auto intens = data_[i]->getIntensityArray();
            for ( size_t j = 0; j < ysize && j < data_[i]->size(); ++j )
                m( i, j ) = intens[ j ];
        }
        return m;
    }
    return {};
}

const std::vector< double >&
Surface::yValues() const
{
    return yValues_;
}

const std::vector< double >&
Surface::xValues() const
{
    return xValues_;
}
