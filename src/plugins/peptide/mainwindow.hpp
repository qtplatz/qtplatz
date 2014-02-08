/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#pragma once

#include <utils/fancymainwindow.h>
#include <memory>
#include <array>

namespace Core { class IMode; }
namespace Utils { class StyledBar; }
namespace adprot { class protein; class protfile; }

class QToolButton;
class QAction;
class QLineEdit;

namespace peptide {

    class MainWindow : public Utils::FancyMainWindow {
        Q_OBJECT
    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

        void createActions();

        QWidget * createContents( Core::IMode * );
        void onInitialUpdate();
        
    signals:
            
    public slots:
        void actFileOpen();

    private slots:

    private:
        enum idActions { idActFileOpen, numActions };

        QDockWidget * toolBarDockWidget_;
        std::array< QAction *, numActions > actions_;
        std::unique_ptr< QLineEdit > topLineEdit_;
        std::vector< QWidget * > wnds_;
        std::shared_ptr< adprot::protfile > protfile_;

        void createDockWidgets();
        QDockWidget * createDockWidget( QWidget *, const QString& title = QString() );
        void setSimpleDockWidgetArrangement();
        Utils::StyledBar * createTopStyledBar();
        Utils::StyledBar * createMidStyledBar();

        QToolButton * toolButton( QAction * );
        QToolButton * toolButton( const char * );
        QAction * createAction( const QString& iconname, const QString& msg, QObject * parent );
    };

}

#endif // MAINWINDOW_HPP
