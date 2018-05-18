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

#ifndef QUANPLOTDATA_HPP
#define QUANPLOTDATA_HPP

#include <boost/optional.hpp>
#include <memory>

namespace adcontrols { class MassSpectrum; class MSPeakInfo; class Chromatogram; class PeakResult; class QuanSample; class ProcessMethod; }

namespace quan {

    class QuanPlotData {
    public:
    public:
        ~QuanPlotData();
        QuanPlotData();
        QuanPlotData( const QuanPlotData& );
        
        void setProfile( std::shared_ptr< adcontrols::MassSpectrum > d )      {  profile = d;     }
        void setProfiledHist( std::shared_ptr< adcontrols::MassSpectrum > d ) {  profiledHist = d;}
        void setFilterd( std::shared_ptr< adcontrols::MassSpectrum > d )      {  filterd = d;     }
        void setCentroid( std::shared_ptr< adcontrols::MassSpectrum > d )     {  centroid = d;    }
        void setPkinfo( std::shared_ptr< adcontrols::MSPeakInfo > d )         {  pkinfo = d;      }
        void setChromatogram( std::shared_ptr< adcontrols::Chromatogram > d ) {  chromatogram = d;}
        void setPkResult( std::shared_ptr< adcontrols::PeakResult > d )       {  pkResult = d;    }
        void setSample( std::shared_ptr< adcontrols::QuanSample > d )         {  sample = d;      }
        void setProcmethod( std::shared_ptr< adcontrols::ProcessMethod > d )  {  procmethod = d ; }

        boost::optional< std::shared_ptr< adcontrols::MassSpectrum > > profile;
        boost::optional< std::shared_ptr< adcontrols::MassSpectrum > > profiledHist;
        boost::optional< std::shared_ptr< adcontrols::MassSpectrum > > filterd;
        boost::optional< std::shared_ptr< adcontrols::MassSpectrum > > centroid;
        boost::optional< std::shared_ptr< adcontrols::MSPeakInfo > > pkinfo;
        boost::optional< std::shared_ptr< adcontrols::Chromatogram > > chromatogram;
        boost::optional< std::shared_ptr< adcontrols::PeakResult > > pkResult;
        boost::optional< std::shared_ptr< adcontrols::QuanSample > > sample;
        boost::optional< std::shared_ptr< adcontrols::ProcessMethod > > procmethod;
    };

}

#endif // QUANPLOTDATA_HPP
