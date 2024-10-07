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

#pragma once

#include <utils/fancymainwindow.h>
namespace Core { class IMode; }
namespace Utils { class StyledBar; }
class QStandardItemModel;
class QTableView;
class QHBoxLayout;
class QWidget;
class QToolButton;
class QAction;
class QLineEdit;
class QProgressBar;

namespace rest {

    class MainWindow : public Utils::FancyMainWindow {

        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);

		void OnInitialUpdate();
        void OnClose();

        QWidget * createContents();
        void createActions();
		static MainWindow * instance();
    private:
		void setSimpleDockWidgetArrangement();
		QDockWidget * createDockWidget( QWidget *, const QString& title = QString(), const QString& objname = QString() );
        void createDockWidgets();

        Utils::StyledBar * createTopStyledBar();
        Utils::StyledBar * createMidStyledBar();

    private:
        class impl;
        impl * impl_;
    };

} // namespace HelloWorld
