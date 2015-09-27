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
#include <adplugin_manager/lifecycle.hpp>
#include <adplugin/lifecycle.hpp>
#include "ap240w_global.hpp"
#include "constants.hpp"
#include <memory>

namespace adcontrols { namespace ControlMethod { class Method; } class threshold_method; }
namespace ap240controls { class method; }

namespace ap240w {
    
    namespace Ui {
        class ap240form;
    }

    class AP240WSHARED_EXPORT ap240form : public QWidget
                                        , public adplugin::LifeCycle {

        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )

    public:
        explicit ap240form(QWidget *parent = 0);
        ~ap240form();

        // LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any& ) override;
    
        void onInitialUpdate();
        void onStatus( int );

        void get( ap240controls::method& ) const;
        void set( const ap240controls::method& );
        void get( int ch, adcontrols::threshold_method& ) const;    
        void set( int ch, const adcontrols::threshold_method& );    

    signals:
        void valueChanged( ap240w::idCategory cat, int ch );
        // void valueChanged( int, const QVariant& );
    
    private:
        Ui::ap240form *ui;
    };
}

#endif // AP240FORM_HPP
