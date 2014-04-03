/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mainwindow.hpp"
#include <adwplot/spectrumwidget.hpp>
#include <adportable/profile.hpp>
#include <QAction>
#include <QBoxLayout>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTextEdit>
#include <QTreeView>
#include <QToolBar>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

MainWindow::MainWindow(QWidget *parent) :  QMainWindow(parent)
{
	this->resize( 800, 600 );
    QWidget * centralWidget = new QWidget( this );
    centralWidget->setObjectName( "centralWidget" );
    this->setCentralWidget( centralWidget );
    this->setStatusBar( new QStatusBar( this ) );
    this->setMenuBar( new QMenuBar( this ) );

	if ( QMenu * fileMenu = this->menuBar()->addMenu( tr("&File") ) ) {
		fileMenu->addAction( tr("Open Grams .spc file..."), this, SLOT( actFileOpen() ) );
	}

    QToolBar * toolBar = new QToolBar( this );
    do {
        toolBar->setProperty( "topBorder", true );
        QHBoxLayout * layout = new QHBoxLayout( toolBar );
        layout->setMargin( 0 );
        layout->setSpacing( 0 );
        layout->addWidget( new QLabel( "Status:" ) );
    } while(0);

    QVBoxLayout * toolBarLayout = new QVBoxLayout( centralWidget );
    toolBarLayout->setMargin( 0 );
    toolBarLayout->setSpacing(0);
    toolBarLayout->addWidget( new adwplot::SpectrumWidget );
    toolBarLayout->addWidget( toolBar );

    this->addDockWidget( Qt::BottomDockWidgetArea, addDockForWidget( new QTextEdit ) );
}

MainWindow::~MainWindow()
{
}

QDockWidget *
MainWindow::addDockForWidget( QWidget * widget )
{
    QDockWidget *dockWidget = new QDockWidget(widget->windowTitle(), this);
    dockWidget->setObjectName(widget->windowTitle());
    dockWidget->setWidget(widget);
    updateDockWidget(dockWidget);
    return dockWidget;
}

void
MainWindow::updateDockWidget(QDockWidget *dockWidget)
{
    const QDockWidget::DockWidgetFeatures features =
        QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable;
    QWidget *titleBarWidget = dockWidget->titleBarWidget();
    if ( dockWidget->isFloating() && titleBarWidget) {
        delete titleBarWidget;
        titleBarWidget = 0;
    }
    dockWidget->setTitleBarWidget(titleBarWidget);
    dockWidget->setFeatures(features);
}

void
MainWindow::actFileOpen()
{
    boost::filesystem::path datapath( adportable::profile::user_data_dir<char>() );
    
    QString name =
        QFileDialog::getOpenFileName( this
                                      , tr("Open Grams .spc file")
                                      , datapath.string().c_str()
                                      , tr("Galactic SPC Files(*.spc)") );
    if ( !name.isEmpty() ) {
        if ( adwplot::SpectrumWidget * w = findChild< adwplot::SpectrumWidget * >() )
            w->setTitle( name.toStdString() );
        Open( name.toStdString() );
    }
}

void
MainWindow::Open( const std::string& filename )
{
}
