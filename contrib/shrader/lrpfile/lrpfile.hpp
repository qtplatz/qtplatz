/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

    class lrpfile {
        lrpfile( const lrpfile& ) = delete;
        lrpfile& operator = (const lrpfile&) = delete;
    public:
        ~lrpfile();
        lrpfile( std::istream& in, size_t fsize );
        typedef std::vector< std::shared_ptr< shrader::msdata > >::iterator iterator;
        typedef std::vector< std::shared_ptr< shrader::msdata > >::const_iterator const_iterator;

        operator bool () const;
        void dump( std::ostream& ) const;

        const lrptic * lrptic() const;
        const msdata * operator []( size_t idx ) const;
        size_t number_of_spectra() const { return msdata_.size(); }
        iterator begin() { return msdata_.begin(); }
        iterator end() { return msdata_.end(); }
        const_iterator begin() const { return msdata_.begin(); }
        const_iterator end() const { return msdata_.end(); }

        bool getTIC( std::vector< double >& time, std::vector< double >& intens ) const;
        bool getMS( const msdata&, std::vector< double >& time, std::vector< double >& intens ) const;

    private:
        bool loaded_;
        std::shared_ptr< shrader::lrpheader > header_;
        std::shared_ptr< shrader::lrphead2 > header2_;
        std::shared_ptr< shrader::lrphead3 > header3_;
        std::shared_ptr< shrader::instsetup > instsetup_;
        std::shared_ptr< shrader::lrpcalib > lrpcalib_;
        std::shared_ptr< shrader::simions > simions_;
        std::shared_ptr< shrader::lrptic > lrptic_;
        std::vector< std::shared_ptr< shrader::msdata > > msdata_;
    };

}

