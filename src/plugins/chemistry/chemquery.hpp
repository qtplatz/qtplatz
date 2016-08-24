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

#pragma once

#include <memory>
#include <adfs/sqlite.hpp>
#include <workaround/boost/uuid/uuid.hpp>
#include <QString>
#include <QVariant>

namespace RDKit { class ROMol; }

namespace chemistry {
    
    class ChemQuery : public std::enable_shared_from_this< ChemQuery > {
    public:
        ChemQuery( adfs::sqlite& );
        ChemQuery( const ChemQuery& );
        adfs::stmt& stmt() { return sql_; }

        bool prepare( const std::string& sql );
        bool prepare( const std::wstring& sql );

        adfs::sqlite_state step();

        size_t column_count() const;

        QString column_name( size_t idx ) const;
        static QString column_name_tr( const QString& );
        QVariant column_value( size_t idx ) const;

        bool insert( const RDKit::ROMol&, const std::string& smiles, const std::string& synonym, const std::string& inchi = std::string() );

    private:
        adfs::sqlite_state state_;
        adfs::stmt sql_;
    };

}

Q_DECLARE_METATYPE(std::shared_ptr< chemistry::ChemQuery >)


