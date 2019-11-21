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
#include <adprocessor/dataprocessor.hpp>
#include <compiler/boost/workaround.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/uuid/uuid.hpp>

namespace adcontrols {
    class MassSpectrum;
    class DataReader;
}

class DataReader;

namespace ws_adprocessor {
    // public adcontrols::dataSubscriber {
    class dataProcessor : public adprocessor::dataprocessor {
                                    
        //const adcontrols::LCMSDataset * raw_;
        //adcontrols::datafile * file_;
    public:
        dataProcessor();
        ~dataProcessor();
        
        bool subscribe( const adcontrols::LCMSDataset& raw ) override;
        bool subscribe( const adcontrols::ProcessedDataset& ) override;
        void notify( adcontrols::dataSubscriber::idError, const std::string& json ) override;
        //
        // const adcontrols::LCMSDataset * raw() const { return raw_; }
        
        bool open( const std::wstring& filename );
        std::vector< std::shared_ptr< DataReader > > dataReaders();
        std::shared_ptr< DataReader > dataReader( const std::string& uuid );
    };

}
