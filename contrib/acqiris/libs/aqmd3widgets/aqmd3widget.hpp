/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "aqmd3widgets_global.hpp"
#include "constants.hpp"
#include <QWidget>
#include <adwidgets/lifecycle.hpp>
#include <adplugin/lifecycle.hpp>

namespace aqmd3controls { class method; }

namespace aqmd3widgets {

    class AQMD3WIDGETSSHARED_EXPORT AQMD3Widget : public QWidget
                                                , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit AQMD3Widget(QWidget *parent = 0);

        // LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;

        void onInitialUpdate();
        void onStatus( int );

        bool get( aqmd3controls::method& ) const;
        bool set( const aqmd3controls::method& );

        void setEnabled( const QString&, bool );

    private:

    signals :
        void valueChanged( idCategory, int channel ); // historical, deprecated
        void dataChanged();
        void applyTriggered();

    public slots:

    };

}
