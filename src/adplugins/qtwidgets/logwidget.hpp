// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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

#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <adplugin/lifecycle.hpp>
#include <adportable/configuration.hpp>
#include <string>

namespace Ui {
    class LogWidget;
}

namespace qtwidgets {

    class LogWidget : public QWidget
                    , public adplugin::LifeCycle {
        Q_OBJECT
        
    public:
        explicit LogWidget(QWidget *parent = 0);
        ~LogWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
    public slots:
        void handle_eventLog( QString );
        void handle_debug_print( unsigned long priority, unsigned long category, QString text );
        void getLifeCycle( adplugin::LifeCycle*& );

    private:
        Ui::LogWidget *ui;
        adportable::Configuration config_;
    };

}

#endif // LOGWIDGET_H
