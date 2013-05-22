/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "molwidget.hpp"
#include <adcontrols/ctfile.hpp>
#include <adcontrols/ctable.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <qtwrapper/qstring.hpp>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QPainter>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <string>

using namespace qtwidgets;

MolWidget::MolWidget(QWidget *parent) :
    QWidget(parent)
{
	setAcceptDrops( true );
}

void
MolWidget::dragEnterEvent( QDragEnterEvent * event )
{
	const QMimeData * mimeData = event->mimeData();
	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
		for ( int i = 0; i < urlList.size(); ++i ) {
			boost::filesystem::path path( qtwrapper::wstring::copy( urlList.at(i).toLocalFile() ) );
			if ( path.extension() == L".mol" ) {
				event->acceptProposedAction();
				return;
			}
		}
	}
}

void
MolWidget::dropEvent( QDropEvent * event )
{
	using adcontrols::CTable;
	using adcontrols::CTFile;

	const QMimeData * mimeData = event->mimeData();
	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
		for ( int i = 0; i < urlList.size(); ++i ) {
			QString molfile( urlList.at(i).toLocalFile() );
			boost::filesystem::path path( qtwrapper::wstring::copy( molfile ) );
			if ( path.extension() == L".mol" ) {
                CTable ctab;
				if ( CTFile::load_molfile( path, ctab ) ) {
					ctabs_.push_back( std::make_pair( molfile, ctab ) );
					update();
					emit molChanged( molfile );
				}                 
			}
		}
	}
}

void
MolWidget::paintEvent( QPaintEvent * )
{
	using adcontrols::ChemicalFormula;

    QPainter painter( this );

	if ( ctabs_.empty() ) {
        QRect rc = painter.viewport();
		rc.setRect( rc.x() + 1, rc.y() + 1, rc.width() - 2, rc.height() - 2 );
        painter.drawRect( rc );
        painter.setFont( QFont( "Decorative" ) );
		painter.drawText( rc, Qt::AlignHCenter | Qt::AlignCenter, "Drop MOL file here" );
		return;
	}

	const adcontrols::CTable& ctab = ctabs_.back().second;

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
	label << stdformula << "                mass: " << std::fixed << std::setprecision(10) << m;
	painter.drawText( 16, 24, qtwrapper::qstring( label.str() ) );
	// painter.drawText( 16, 24 + 16, qtwrapper::qstring( formula ) );

	// --- draw molecule --
    painter.translate( painter.window().center() );
	painter.setFont( QFont( "Decorative" ) );
	const double factor = 25;
	for ( size_t n = 0; n < ctab.atoms().size(); ++n ) {
		const adcontrols::CTable::Atom& a = ctab.atom( n );
		if ( hydrogens[ n ].find( L"H" ) == std::string::npos )
			painter.setPen( QColor( Qt::black ) );
		else
			painter.setPen( QColor( Qt::red ) );
		//QRectF rc( a.x * factor - 16, a.y * factor - 16, 32, 32 );
		//painter.drawText( rc, Qt::AlignCenter | Qt::AlignHCenter, qtwrapper::qstring::copy( a.symbol ) );
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

	std::vector< key_ctable_pair_t >::iterator it
		= std::remove_if( ctabs_.begin(), ctabs_.end()
		  , boost::bind( &key_ctable_pair_t::first, _1 ) == QString( "delete me" ) );
	ctabs_.erase( it, ctabs_.end() );
}

bool
MolWidget::getCTable( const QString& molfile, adcontrols::CTable& ctab )
{
	using adcontrols::CTable;

	ctab.clear();

	std::vector< key_ctable_pair_t >::reverse_iterator it = 
		std::find_if( ctabs_.rbegin(), ctabs_.rend(), boost::bind( &key_ctable_pair_t::first, _1 ) == molfile );

	if ( it != ctabs_.rend() ) {
		ctab = it->second;
		return true;
	}

	return false;
}

bool
MolWidget::draw( const adcontrols::CTable& ctab )
{
	ctabs_.push_back( std::make_pair( QString("delete me"), ctab ) );
	update();
	return true;
}
