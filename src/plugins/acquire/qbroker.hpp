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

#ifndef QBROKER_HPP
#define QBROKER_HPP

#include <adextension/ibroker.hpp>

namespace Broker { class Manager; }

namespace acquire {

    class QBroker : public adextension::iBroker  {
        Q_OBJECT
    public:
        explicit QBroker( Broker::Manager *, QObject *parent = 0);
        ~QBroker();

        Broker::Manager * brokerManager() const override;
        //
        void setBrokerManager( Broker::Manager* );
        
    signals:

    public slots:

    private:
        Broker::Manager * ptr_;

#if defined _MSC_VER
        QBroker(const QBroker&) {}
        QBroker& operator=(const QBroker&) {}
#else
        QBroker(const QBroker&) = delete;
        QBroker& operator=(const QBroker&) = delete;
#endif
    };

}

#endif // QBROKER_HPP
