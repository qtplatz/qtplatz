/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include <QFrame>
#include <adplugin/lifecycle.hpp>
#include <adwidgets/lifecycle.hpp>
#include "infitofwidgets_global.hpp"

class QNetworkReply;

namespace infitofwidgets {

    class INFITOFWIDGETSSHARED_EXPORT hvWidget : public QFrame
                                               , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:

        explicit hvWidget( const QString&, const QString& port = "http", QWidget *parent = 0 );
        ~hvWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;

        //
        QSize sizeHint() const override;

    private slots:
        void handleReply( const QString&, const QByteArray& );
        void handleProtocols( const QByteArray& );
        void handleSwitchToggled( QObject *, bool );
        void handleValueChanged( QObject *, double );

    private:
        class Impl;
        Impl * impl_;

    };
}
