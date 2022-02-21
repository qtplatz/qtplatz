/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <tuple>
#include <string>
#include <map>

namespace adchem { class SDMol; }

namespace lipidid {

    class mol {
    public:
        typedef std::tuple< size_t          // id
                            , std::string   // formula
                            , std::string   // smiles
                            , std::string   // inchikey
                            , double        // logP
                            > value_type;
        ~mol();
        mol();
        mol( const mol& );
        mol( value_type&& ); // id, formula, smiles, inchicky
        inline size_t index() const { return std::get<0>( mol_ ); }
        inline const std::string& formula() const { return std::get<1>( mol_ ); }
        inline const std::string& smiles() const { return std::get<2>( mol_ ); }
        inline const std::string& inchikey() const { return std::get<3>( mol_ ); }
        inline double mass() const { return mass_; }
        inline double logP() const { return std::get< 4 >( mol_ ); }
    private:
        value_type mol_;
        const double mass_;
    };

    class moldb {
        std::map< std::string, std::shared_ptr< const mol > > mols_;
    public:
        static moldb& instance();
        std::shared_ptr< const mol > find( const std::string& InChIKey ) const;
        static double logP( const std::string& InChIKey );
        moldb& operator << ( std::shared_ptr< const mol > );
    };

}
