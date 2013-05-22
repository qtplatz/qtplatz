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
#include <qlayout.h>

namespace Core { class IMode; }

class QHBoxLayout;
class QWidget;
class QToolButton;
class QAction;

namespace chemistry { 

    class SDFileModel;

	class ChemistryMainWindow : public Utils::FancyMainWindow {
		Q_OBJECT
	public:
        ~ChemistryMainWindow();
		explicit ChemistryMainWindow();

		QWidget * createContents( Core::IMode * );
		void createActions();

		void OnInitialUpdate();
		void activateLayout();
		void setSimpleDockWidgetArrangement();
		QDockWidget * createDockWidget( QWidget *, const QString& title = QString() );
		static QToolButton * toolButton( const char * );
		static QToolButton * toolButton( QAction * );
		static ChemistryMainWindow * instance();
    
    signals:
    
	public slots:
		void actionSearch();
		void handleViewDetails( int raw, const SDFileModel * );
		void handleViewFragments( int raw, const SDFileModel * );

	private:
		static ChemistryMainWindow * instance_;
		QWidget * toolBar_;
		QHBoxLayout * toolBarLayout_;
		QDockWidget * toolBarDockWidget_;

		QAction * actionSearch_;

		void setToolBarDockWidget( QDockWidget * dock );
		QDockWidget * toolBarDockWidget() { return toolBarDockWidget_; }
		void createDockWidgets();
		void createToolbar();
	};

}

#endif // CHEMISTRYMAINWINDOW_HPP
