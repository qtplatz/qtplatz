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

#ifndef QUANCONNECTION_HPP
#define QUANCONNECTION_HPP

#include <memory>
#include <string>
#include <vector>
#include <boost/variant.hpp>
#include <adfs/sqlite.hpp>
#include <adfs/file.hpp>

namespace adfs { class filesystem; class stmt; class sqlite; }
namespace adcontrols { class QuanMethod; class QuanSequence; class MassSpectrum; class MSPeakInfo; class ProcessMethod; }

namespace quan {

    class QuanQuery;
    class QuanPlotData;
    class QuanMethodComplex;

    class QuanConnection : public std::enable_shared_from_this < QuanConnection > {
        QuanConnection( const QuanConnection& ) = delete;
    public:
        QuanConnection();
        ~QuanConnection();

        bool connect( const std::wstring& database );
        std::shared_ptr< QuanQuery > query();
        adfs::sqlite& db();

        adfs::file select_file( const std::wstring& dataClass );

        QuanPlotData * fetch( const std::wstring& dataGuid );

        const std::wstring& filepath() const { return filename_; }
        
        const adcontrols::ProcessMethod * processMethod();
        const adcontrols::QuanSequence * quanSequence();

    private:
        std::wstring filename_;
        std::shared_ptr< adfs::filesystem > fs_;
        std::map< std::wstring, std::shared_ptr< QuanPlotData > > cache_;
        std::shared_ptr< adcontrols::QuanSequence > sequence_;
        std::shared_ptr< adcontrols::ProcessMethod > procmethod_;

        bool readMethods();
    };

}

#endif // QUANCONNECTION_HPP
