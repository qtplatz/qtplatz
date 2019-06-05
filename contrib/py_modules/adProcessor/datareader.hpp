/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC
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

#include <adcontrols/datafile.hpp>
#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/datareader.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/python.hpp>
#include <boost/uuid/uuid.hpp>

namespace adcontrols {
    class MassSpectrum;
}

class DataReader {
public:
    DataReader( std::shared_ptr< const adcontrols::DataReader > );
    DataReader();
    DataReader( const DataReader& );
    ~DataReader();
    boost::uuids::uuid objuuid() const;
    std::string objtext() const;
    std::string display_name() const;

    size_t size( int fcn = (-1) ) const;
    bool rewind();
    bool next();
    int64_t rowid() const;
    int64_t epoch_time() const;
    int64_t elapsed_time() const;
    double time_since_inject() const;
    int protocol() const;

    std::shared_ptr< adcontrols::MassSpectrum > readSpectrum() const;
private:
    std::shared_ptr< const adcontrols::DataReader > reader_;
    adcontrols::DataReader::const_iterator it_;
};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS( DataReader_overloads, size, 0, 1 );
