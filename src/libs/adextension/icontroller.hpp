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

#include <QObject>
#include "adextension_global.hpp"
#include <functional>

namespace adcontrols {
    namespace ControlMethod { class Method; }
}

namespace adextension {

    class ADEXTENSIONSHARED_EXPORT iController : public QObject {
        Q_OBJECT
    public:
        explicit iController(QObject *parent = 0);

        virtual bool connect() = 0;
        virtual bool wait_for_connection_ready() = 0;
        virtual bool preparing_for_run( adcontrols::ControlMethod::Method& ) = 0;

        /* module_name identify the instrument/peripheral model name
         * which match up with the name on control method item filed
         */
        virtual QString module_name() const = 0;

        /* module_number identify instrument where same instruments are configured
         * such as two UV-ditectors, multiple 6-way valves etc.
         */
        virtual int module_number() const = 0;        
        
    signals:
        void onControlMethodChanged();
        void connected( iController * self );

    public slots:

    };

}

