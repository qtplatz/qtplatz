/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#include "molwidget.hpp"
#include <adcontrols/ctfile.hpp>
#include <adcontrols/ctable.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <qtwrapper/qstring.hpp>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QPainter>
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
}

bool
MolWidget::getCTable( const QString& molfile, adcontrols::CTable& ctab )
{
	return false;
}

bool
MolWidget::draw( const adcontrols::CTable& ctab )
{
	ctabs_.push_back( std::make_pair( QString("delete me"), ctab ) );
	update();
	return true;
}
