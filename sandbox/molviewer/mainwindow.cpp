/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include <qtwrapper/qstring.hpp>
#include <adcontrols/ctfile.hpp>
#include <adcontrols/ctable.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <QDragEnterEvent>
#include <QUrl>
#include <QDebug>
#include <QPainter>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <sstream>
#include <cmath>
#include <iomanip>

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
MainWindow::paintEvent( QPaintEvent * )
{
	using adcontrols::ChemicalFormula;

	if ( ctabs_.empty() )
		return;

    QPainter painter( this );
	const adcontrols::CTable& ctab = ctabs_.back();

	std::wstring formula = ChemicalFormula::getFormula( ctab );
	std::vector< std::wstring > hydrogens;
	do {
		typedef boost::char_separator< wchar_t > separator;
		typedef boost::tokenizer< boost::char_separator<wchar_t>
			, std::wstring::const_iterator
			, std::wstring > tokenizer;
		separator sep( L" ", L"" );
		tokenizer tokens( formula, sep );
		for ( tokenizer::iterator it = tokens.begin(); it != tokens.end(); ++it )
			hydrogens.push_back( *it );
	} while(0);
    
	std::wstring stdformula = ChemicalFormula().standardFormula( formula );
	double m = ChemicalFormula().getMonoIsotopicMass( stdformula );
	std::wostringstream label;
	label << stdformula << "  mass: " << std::fixed << std::setprecision(10) << m;
	painter.drawText( 16, 24, qtwrapper::qstring( label.str() ) );
	painter.drawText( 16, 24 + 16, qtwrapper::qstring( formula ) );

	// --- draw molecule --
    painter.translate( painter.window().center() );

	const double factor = 25;
	for ( size_t n = 0; n < ctab.atoms().size(); ++n ) {
		// BOOST_FOREACH( const adcontrols::CTable::Atom& a, ctab.atoms() ) {
		const adcontrols::CTable::Atom& a = ctab.atom( n );
		if ( hydrogens[ n ].find( L"H" ) == std::string::npos )
			painter.setPen( QColor( Qt::black ) );
		else
			painter.setPen( QColor( Qt::red ) );
		painter.drawText( a.x * factor, a.y * factor, qtwrapper::qstring::copy( a.symbol ) );
	}
	BOOST_FOREACH( const adcontrols::CTable::Bond& b, ctab.bonds() ) {
		adcontrols::CTable::Atom a1 = ctab.atom( b.first_atom_number - 1 );
		adcontrols::CTable::Atom a2 = ctab.atom( b.second_atom_number - 1 );
        a1.x *= factor;
        a1.y *= factor;
        a2.x *= factor;
        a2.y *= factor;
		if ( b.bond_type == 1 ) {
			painter.setPen( QColor( Qt::blue ) );
			painter.drawLine( a1.x, a1.y, a2.x, a2.y );
		} else if ( b.bond_type == 2 ) {
			painter.setPen( QColor( Qt::blue ) );
			double dx = a2.x - a1.x;
			double dy = a2.y - a1.y;
			double L = std::sqrt( ( dx * dx ) + ( dy * dy ) );
			const int offset = 2;
			do {
				double x1 = a1.x - offset * dy / L;
				double x2 = a2.x - offset * dy / L;
				double y1 = a1.y - offset * (-dx) / L;
				double y2 = a2.y - offset * (-dx) / L;
				painter.drawLine( x1, y1, x2, y2 );
			} while(0);
			do {
				double x1 = a1.x + offset * dy / L;
				double x2 = a2.x + offset * dy / L;
				double y1 = a1.y + offset * (-dx) / L;
				double y2 = a2.y + offset * (-dx) / L;
				painter.drawLine( x1, y1, x2, y2 );
			} while(0);
		} else if ( b.bond_type == 3 ) {
			painter.setPen( QColor( Qt::blue ) );
			painter.drawLine( a1.x, a1.y, a2.x, a2.y );

			double dx = a2.x - a1.x;
			double dy = a2.y - a1.y;
			double L = std::sqrt( ( dx * dx ) + ( dy * dy ) );
			const int offset = 3;
			do {
				double x1 = a1.x - offset * dy / L;
				double x2 = a2.x - offset * dy / L;
				double y1 = a1.y - offset * (-dx) / L;
				double y2 = a2.y - offset * (-dx) / L;
				painter.drawLine( x1, y1, x2, y2 );
			} while(0);
			do {
				double x1 = a1.x + offset * dy / L;
				double x2 = a2.x + offset * dy / L;
				double y1 = a1.y + offset * (-dx) / L;
				double y2 = a2.y + offset * (-dx) / L;
				painter.drawLine( x1, y1, x2, y2 );
			} while(0);
		}
	}
}

void
MainWindow::molfile_open( const boost::filesystem::path& path )
{
	using adcontrols::CTable;
	using adcontrols::CTFile;
	
	CTable ctable;
	if ( CTFile::load_molfile( path, ctable ) ) {
		ctabs_.push_back( ctable );
        update();
	}
}