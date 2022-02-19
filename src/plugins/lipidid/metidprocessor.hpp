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

#include <memory>

namespace adwidgets { class ProgressInterface; }
namespace adcontrols { class MassSpectrum; class MetIdMethod; }
namespace adfs { class sqlite; }

namespace lipidid {

    class simple_mass_spectrum;

    class MetIdProcessor : public std::enable_shared_from_this< MetIdProcessor > {
        MetIdProcessor( const MetIdProcessor& ) = delete;
        MetIdProcessor& operator = ( const MetIdProcessor& ) = delete;
        MetIdProcessor();
        MetIdProcessor( const adcontrols::MetIdMethod& );
    public:
        ~MetIdProcessor();
        typedef MetIdProcessor this_type;

        // https://stackoverflow.com/questions/8147027/how-do-i-call-stdmake-shared-on-a-class-with-only-protected-or-private-const/8147101#8147101
        template<typename ...Args> std::shared_ptr<this_type> static create(Args&&...arg) {
            struct enable_make_shared : public this_type {
                enable_make_shared(Args&&...arg) : this_type(std::forward<Args>(arg)...) {}
            };
            return std::make_shared<enable_make_shared>(std::forward<Args>(arg)...);
        }

        std::tuple< std::shared_ptr< const adcontrols::MassSpectrum > // acquired spectrum
                    , std::shared_ptr< adcontrols::MassSpectrum > // reference (calculated) spectrum
                    , std::shared_ptr< lipidid::simple_mass_spectrum > // reference (calculated) spectrum
                    >
        find_all( adfs::sqlite& db
                    , std::shared_ptr< const adcontrols::MassSpectrum >
                    , std::shared_ptr< adwidgets::ProgressInterface > );

    private:
        class impl;
        std::unique_ptr< impl > impl_;
    };

}
