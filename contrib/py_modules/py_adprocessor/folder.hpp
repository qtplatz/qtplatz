/**************************************************************************
** Copyright (C) 2020 MS-Cheminformatics LLC
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

#include <adfs/folder.hpp>
#include <boost/python.hpp>

namespace py_module {

    class folder {
        adfs::folder folder_;
    public:
        ~folder();
        folder();
        folder( const folder& );
        folder( const adfs::folder& );

        uint64_t rowid() const;
        std::wstring name() const;
        std::wstring id() const;
        boost::python::dict attributes() const;

        //std::vector< std::shared_ptr< folder > > folders() const;
        boost::python::list folders() const;
        boost::python::list files() const;
    };

}
