/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef IMPORT_HPP
#define IMPORT_HPP

#pragma once

#include <adcontrols/datasubscriber.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace adcontrols { class datafile; class LCMSDataset; class Chromatogram; }
namespace adfs { class filesystem; }

namespace batchproc {

    class import : public std::enable_shared_from_this< import >
                 , public adcontrols::dataSubscriber {
    public:
        ~import();
        import();
        import( int row
                , const std::wstring& source_file
                , const std::wstring& destination_file
                , std::function<bool(int, int, int)> );

        operator bool () const { return datafile_ != 0; }
        bool operator()();

        // adcontrols::dataSubscriber
        bool subscribe( const adcontrols::LCMSDataset& ) override;
        // bool subscribe( const adcontrols::ProcessedDataset& ) override;
        
    private:
        int rowId_;
        std::wstring source_file_;
        std::wstring destination_file_;
        std::function<bool(int, int, int)> progress_;
        adcontrols::datafile * datafile_;
        const adcontrols::LCMSDataset* accessor_;
        std::unique_ptr< adfs::filesystem > fs_;
        std::vector< std::shared_ptr< adcontrols::Chromatogram > > tic_;
    };

}

#endif // IMPORT_HPP
