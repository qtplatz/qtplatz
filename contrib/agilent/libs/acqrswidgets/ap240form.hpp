/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef AP240FORM_HPP
#define AP240FORM_HPP

#include <QWidget>
#include <adwidgets/lifecycle.hpp>
#include <adplugin/lifecycle.hpp>
#include "acqrswidgets_global.hpp"
#include "constants.hpp"
#include <memory>

namespace adcontrols { namespace ControlMethod { class Method; } class threshold_method; class threshold_action; }
namespace acqrscontrols { namespace ap240 { class method; } }
namespace adwidgets {
    class MouseRButtonFilter;
}

namespace acqrswidgets {

    namespace Ui {
        class ap240form;
    }

    class ACQRSWIDGETSSHARED_EXPORT ap240form : public QWidget
                                        , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit ap240form(QWidget *parent = 0);
        ~ap240form();

        static constexpr const char * clsid_text = "{E13469DF-372B-4825-A6D3-FA93BEC2101D}";

        // LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any&& ) override;

        void onInitialUpdate();
        void onStatus( int );

        void get( std::shared_ptr< acqrscontrols::ap240::method > ) const;
        void set( std::shared_ptr< const acqrscontrols::ap240::method> );

        void get( int ch, adcontrols::threshold_method& ) const;
        void set( int ch, const adcontrols::threshold_method& );
        void get( adcontrols::threshold_action& ) const;
        void set( const adcontrols::threshold_action& );

        void get( QJsonObject& ) const;

        void setRemoteAccess( bool, const QString& host, const QString& port );
        bool remoteAccess( QString& host, QString& port ) const;

    signals:
        void valueChanged( idCategory cat, int ch );
        //void deviceConfigChanged( bool remote_access, const QString& host, const QString& port = "80" );
        void hostChanged( bool remote_access, const QString& host, const QString& port );

    private:
        Ui::ap240form *ui;
        bool remote_;
        QString host_;
        QString port_;
        std::unique_ptr< adwidgets::MouseRButtonFilter > eventFilter_;
    };
}

#endif // AP240FORM_HPP
