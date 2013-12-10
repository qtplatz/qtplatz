/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef CHEMISTRYMAINWINDOW_HPP
#define CHEMISTRYMAINWINDOW_HPP

#include "chemistry_global.hpp"
#include <utils/fancymainwindow.h>
#include <QLayout>
#include <QUrl>
#include <memory>

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

namespace chemistry { 

    class MolTableView;
    class MolTableDelegate;

	class MainWindow : public Utils::FancyMainWindow {
		Q_OBJECT
	public:
        ~MainWindow();
		explicit MainWindow( QWidget * parent = 0 );

		QWidget * createContents( Core::IMode * );
		void createActions();

		void OnInitialUpdate();
		void activateLayout();
		void setSimpleDockWidgetArrangement();
		QDockWidget * createDockWidget( QWidget *, const QString& title = QString() );
		static QToolButton * toolButton( const char * );
		static QToolButton * toolButton( QAction * );
		static MainWindow * instance();
    
    signals:
    
	public slots:
		void actionSearch();
        void actSDFileOpen();

    private slots:
        void handleDropped( const QList< QUrl >& );

	private:
        std::unique_ptr< MolTableView > tableView_;
		static MainWindow * instance_;
		QWidget * toolBar_;
		QHBoxLayout * toolBarLayout_;
		QDockWidget * toolBarDockWidget_;
		QAction * actionSearch_;
        QLineEdit * topLineEdit_;
        QProgressBar * progressBar_;

		void setToolBarDockWidget( QDockWidget * dock );
		QDockWidget * toolBarDockWidget() { return toolBarDockWidget_; }
		void createDockWidgets();
		void createToolbar();
        Utils::StyledBar * createTopStyledBar();
        Utils::StyledBar * createMidStyledBar();
	};

}

#endif // CHEMISTRYMAINWINDOW_HPP
