/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <adcontrols/countinghistogram.hpp>
#include <adcontrols/countingdata.hpp>
#include <adcontrols/threshold_method.hpp>
#include <adfs/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <functional>

namespace acqrscontrols {
    namespace ap240 {
        class waveform;
    }
    template< typename T > class threshold_result_;
}

namespace adprocessor { class dataprocessor; }

class acqrsdata {
public:
    enum polarity { positive_polarity, negative_polarity };
    acqrsdata();
    bool open( const boost::filesystem::path& );
    void setThreshold( double );
    void setPolairty( polarity );
    polarity polarity() const;

    bool processIt( std::function< void( size_t, size_t, const std::string& ) > progress );

    void tdc( std::shared_ptr< acqrscontrols::ap240::waveform > );

    std::shared_ptr< acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform > >  processThreshold3(
        std::shared_ptr< const acqrscontrols::ap240::waveform > waveform
        , const adcontrols::threshold_method& method );

private:
    enum polarity polarity_;
    double threshold_;
    boost::filesystem::path path_;
    std::shared_ptr< adprocessor::dataprocessor > processor_;
    adcontrols::CountingHistogram hgrm_;
};
