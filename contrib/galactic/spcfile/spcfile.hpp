/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>
#include "spcfile_global.hpp"

namespace galactic {

	class spchdr;
	class subhdr;

    class SPCFILESHARED_EXPORT spcfile {
    public:
        ~spcfile();
        spcfile( std::istream& in, size_t fsize );
        
        operator bool () const;
        const galactic::spchdr * spchdr() const;
        const galactic::subhdr * subhdr( size_t idx = 0 ) const;
        size_t number_of_subfiles() const;

        bool isDeprecated() const;
        bool isLittleEndian() const;
        const char * date() const;
        const char * axis_x_label() const;
        const char * axis_y_label() const;
        const char * source_instrument_description() const;

        static const char * axis_type_x_string( int );
        static const char * axis_type_y_string( int );
    
    private:
        std::vector< char > device_;
        std::vector< std::shared_ptr<galactic::subhdr> > subhdrs_;
		galactic::spchdr * spchdr_;
        bool loaded_;
        std::string date_;
        std::string axis_x_label_;
        std::string axis_y_label_;
    };

}

