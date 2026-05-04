/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <memory>
#include <vector>

namespace shrader {

    class lrpheader;
    class lrphead2;
    class lrphead3;
    class instsetup;
    class lrpcalib;
    class simions;
    class lrptic;
    class msdata;

    using tic_t = std::tuple< double, double, int32_t, int32_t >; // time, intensity, ptr, overload
    using ticc_t = std::vector< tic_t >;
    enum { tic_time, tic_intensity, tic_ptr, tic_overload };

    class lrpfile {
        lrpfile( const lrpfile& ) = delete;
        lrpfile& operator = (const lrpfile&) = delete;
    public:
        ~lrpfile();
        lrpfile();

        typedef std::vector< std::shared_ptr< shrader::msdata > >::iterator iterator;
        typedef std::vector< std::shared_ptr< shrader::msdata > >::const_iterator const_iterator;

        bool load( std::istream& in, size_t fsize );
        operator bool () const;

        void dump( std::ostream&, size_t limit = 0 ) const;

        // const shrader::lrptic * lrptic() const;
        const msdata * operator []( size_t idx ) const;
        size_t number_of_spectra() const;
        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        const shrader::lrpheader& header() const;
        const shrader::lrphead2& header2() const;
        const shrader::lrphead3& header3() const;
        const shrader::instsetup& instsetup() const;
        const shrader::lrpcalib& lrpcalib() const;
        const shrader::simions& simions() const;
        const shrader::lrptic& lrptic() const;
        const std::vector< std::shared_ptr< shrader::msdata > >& msdata() const;

        std::string time_of_injection() const;

        ticc_t get_ticc() const;

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
