// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "adcontrols_global.h"
#include <string>
#include <memory>

namespace adcontrols {

    namespace lockmass { class fitter; }

    class ADCONTROLSSHARED_EXPORT MSFractuation : public std::enable_shared_from_this< MSFractuation > {

        MSFractuation(const MSFractuation &) = delete;
        MSFractuation & operator = (const MSFractuation & rhs) = delete;

    public:
		MSFractuation(void);
		~MSFractuation(void);

        static std::shared_ptr< MSFractuation > create();

        void insert( int64_t rowid, const lockmass::fitter& );
        const lockmass::fitter find( int64_t rowid );

    private:
        class impl;
        std::unique_ptr< impl > impl_;

    };

}

