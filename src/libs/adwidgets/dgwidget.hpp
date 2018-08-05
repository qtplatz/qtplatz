/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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
#include "adwidgets_global.hpp"
#include <adplugin/lifecycle.hpp>
#include <adplugin_manager/lifecycle.hpp>
#include <memory>

class QJsonDocument;

namespace adwidgets {

    class MouseEventFilter;

    class ADWIDGETSSHARED_EXPORT dgWidget : public QFrame
                                          , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )
        
    public:
        
        explicit dgWidget(QWidget *parent = 0);
        ~dgWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;

        //
        QSize sizeHint() const override;

        void setURL( const QString& );
    public slots:
        void handleJson( const QJsonDocument& );
        void handleSSE( const QByteArray );

    signals:
        void hostChanged( const QString&, const QString& );

    private:
        std::unique_ptr< MouseEventFilter > eventFilter_;

    };
}

