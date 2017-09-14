/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <QWidget>
#include <adplugin_manager/lifecycle.hpp>
#include <adplugin/lifecycle.hpp>
#include "acqrswidgets_global.hpp"
#include "constants.hpp"
#include <memory>

namespace adcontrols { namespace ControlMethod { class Method; } class threshold_method; class threshold_action; }
namespace acqrscontrols { namespace ap240 { class method; } }

// ap240widget was derived from ap240form class
// this is the subset of ap240form where the threshold_method and threshold_action is removed for
// simple 'time-of-flight' data acquisition as a part of adicontroller archetecture of qtplatz.

namespace acqrswidgets {
    
    class ACQRSWIDGETSSHARED_EXPORT ap240widget : public QWidget
                                                , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit ap240widget(QWidget *parent = 0);
        ~ap240widget();

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

        // void get( int ch, adcontrols::threshold_method& ) const;    
        // void set( int ch, const adcontrols::threshold_method& );    
        // void get( adcontrols::threshold_action& ) const;    
        // void set( const adcontrols::threshold_action& );

        // void setRemoteAccess( bool, const QString& );
        // QPair< bool, QString> remoteAccess() const;

    signals:
        void valueChanged( idCategory cat, int ch );

        // void deviceConfigChanged( bool remote_access, const QString& remote_host );
    
    private:

    };
}


