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

#ifndef PROCESS_HPP
#define PROCESS_HPP

#include "batchprocconstants.hpp"
#include <QString>

namespace batchproc {

    enum process_state {
        PROCESS_IDLE
        , PROCESS_RUNNING
        , PROCESS_CANCELING
        , PROCESS_DORMANT
        , PROCESS_COMPLETED
    };
    
    enum process_kind {
        PROCESS_NONE
        , PROCESS_IMPORT
    };

    class process {
        process_kind kind_;
        process_state state_;
    public:
        process( process_kind t = PROCESS_NONE, process_state s = PROCESS_IDLE );
        process_kind kind() const;
        void kind( process_kind );
        process_state state() const;
        void state( process_state );

        QString display_name() const;
    };
    
}

#endif // PROCESS_HPP
