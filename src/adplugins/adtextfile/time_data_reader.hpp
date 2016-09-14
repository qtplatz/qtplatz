// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
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

#include <adcontrols/countingdata.hpp>
#include <boost/filesystem/path.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace adtextfile {

    class time_data_reader {
    public:
        time_data_reader();

    public:
        bool load( const std::string&
                   , std::function<bool( size_t, size_t )> progress = std::function<bool( size_t, size_t )>() );

        static bool is_time_data( const std::string& path, std::string& adfsname );

        static bool readScanLaw( const std::string& adfsname
                                 , double& acceleratorVoltage, double& tDelay, double& fLength
                                 , std::string& spectrometer );

        const std::vector< adcontrols::CountingData >& data() const;

    private:
        std::vector< adcontrols::CountingData > data_;
    };

}

