/**************************************************************************
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <memory>
#include <vector>

namespace adcontrols {
    class MassSpectrum;
    namespace counting { class trigger_data; }
}
namespace adfs { class sqlite; }
namespace adportable { namespace counting { class counting_result; } }

namespace tools {

    class resultwriter {
    public:
        resultwriter( adfs::sqlite& db );

        ~resultwriter();

        bool insert( std::shared_ptr< const adcontrols::MassSpectrum >, const adcontrols::counting::trigger_data&, adportable::counting::counting_result&& );
        void commit();

    private:
        adfs::sqlite& db_;
    };

}
