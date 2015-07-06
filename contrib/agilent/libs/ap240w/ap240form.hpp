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
#include <adplugin/lifecycle.hpp>
#include "ap240w_global.hpp"
#include <memory>

namespace Ui {
    class ap240form;
}

namespace adcontrols { class ControlMethod; }
namespace ap240 { class method; }

class AP240WSHARED_EXPORT ap240form : public QWidget
                                    , public adplugin::LifeCycle {

    Q_OBJECT

public:
    explicit ap240form(QWidget *parent = 0);
    ~ap240form();
    enum idCategory { idHorizontal, idVertical, idTrigger };

    // LifeCycle
    void OnCreate( const adportable::Configuration& ) override;
    void OnInitialUpdate() override;
    void OnFinalClose() override;
    bool getContents( boost::any& ) const override;
    bool setContents( boost::any& ) override;
    
    void onInitialUpdate();
    void onStatus( int );
    
    void get( ap240::method& ) const;
    void set( const ap240::method& );

signals:
    void valueChanged( idCategory cat, int id, int ch, const QVariant& );
    
private:
    Ui::ap240form *ui;
};

#endif // AP240FORM_HPP
