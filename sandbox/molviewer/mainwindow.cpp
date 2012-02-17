/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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
#include "ui_mainwindow.h"
#include <qtwrapper/qstring.hpp>
#include <QDragEnterEvent>
#include <QUrl>
#include <QDebug>
#include <adcontrols/ctfile.hpp>
#include <adcontrols/ctable.hpp>
#include <adcontrols/chemicalformula.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops( true );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::dragEnterEvent( QDragEnterEvent * event )
{
	const QMimeData * mimeData = event->mimeData();
	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
		for ( int i = 0; i < urlList.size(); ++i ) {
			std::wstring file = qtwrapper::wstring( urlList.at(i).toLocalFile() );
			boost::filesystem::path path( file ); 
			if ( path.extension() == L".mol" ) {
				event->acceptProposedAction();
				return;
			}
		}
	}
}

void
MainWindow::dropEvent( QDropEvent * event )
{
	const QMimeData * mimeData = event->mimeData();
	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
		for ( int i = 0; i < urlList.size(); ++i ) {
			std::wstring file = qtwrapper::wstring( urlList.at(i).toLocalFile() );
			boost::filesystem::path path( file ); 
			if ( path.extension() == L".mol" ) {
                molfile_open( path );
			}
		}
	}
}

void
MainWindow::molfile_open( const boost::filesystem::path& path )
{
	using adcontrols::CTable;
	using adcontrols::CTFile;
	using adcontrols::ChemicalFormula;
	
	CTable ctable;
	if ( CTFile::load_molfile( path, ctable ) ) {
		std::wstring formula;
		ChemicalFormula::getFormula( ctable );
        long x = 0;
	}
}