// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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

#include "constants.hpp"
#include <netcdf.h>
#include <boost/filesystem/path.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <variant>

namespace adnetcdf {

    namespace netcdf {

        class ncfile;
        class dimension;
        class variable;
        class attribute;

        ncfile open( const std::filesystem::path&, open_mode = nc_nowrite );
        ncfile open( const boost::filesystem::path&, open_mode = nc_nowrite );

        /////////////////////
        struct null_datum_t {};

        typedef std::variant< null_datum_t
                              , std::string
                              , std::vector< std::string >
                              , std::vector< float >
                              , std::vector< double >
                              , std::vector< int >
                              > datum_t;

        class ncfile {
            ncfile( const ncfile& ) = delete;
            ncfile& operator = ( const ncfile& ) = delete;
        public:
            ~ncfile();
            ncfile();
            ncfile( const std::filesystem::path&, open_mode = nc_nowrite );
            inline operator bool() const { return rcode_ == NC_NOERR; }
            int32_t rcode() const;
            int32_t ncid() const;
            int32_t ndims() const;
            int32_t nvars() const;
            int32_t ngatts() const;
            int32_t unlimdimid() const;
            const std::filesystem::path& path() const;
            std::pair<int, std::string > kind() const;
            std::tuple< int, uint16_t, std::string > kind_extended() const;
            const std::vector< dimension >& dims() const;
            const std::vector< variable >& vars() const;
            const std::vector< attribute >& atts() const;
            std::vector< attribute > atts( const variable& ) const;
            std::vector< dimension > dims( const variable& ) const;

            std::optional< int > inq_format() const;
            std::optional< std::pair<int, int> > inq_format_extended() const;
            std::optional< dimension > inq_dim( int dimid ) const;
            std::optional< attribute > inq_att( int varid, int attid ) const;
            std::optional< variable > inq_var( int varid ) const;

            datum_t readData( const attribute& t ) const;
            datum_t readData( const variable& t ) const;

        private:
            std::filesystem::path path_;
            int32_t rcode_;
            int ncid_;
            int ndims_;
            int nvars_;
            int ngatts_;
            int unlimdimid_;

            std::vector< dimension > dims_;
            std::vector< attribute > atts_; // global attribute
            mutable std::vector< variable > vars_;
        };

    } // namespace netcdf
}
