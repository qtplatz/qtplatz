/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "molwidget.hpp"
#include <adchem/drawing.hpp>

// #include <RDGeneral/Invariant.h>
// #include <GraphMol/RDKitBase.h>
// #include <GraphMol/SmilesParse/SmilesParse.h>
// #include <GraphMol/SmilesParse/SmilesWrite.h>
// #include <GraphMol/Substruct/SubstructMatch.h>
// #include <GraphMol/Depictor/RDDepictor.h>
// #include <GraphMol/FileParsers/FileParsers.h>
// #include <GraphMol/Descriptors/MolDescriptors.h>
// #include <GraphMol/FileParsers/MolSupplier.h>

// #ifdef _MSC_VER
// # pragma warning(push)
// # pragma warning(disable:4018) // signed/unsigned
// # pragma warning(disable:4189) // local variable not referenced
// #endif
// #include <GraphMol/MolDrawing/MolDrawing.h>
// #include <GraphMol/MolDrawing/DrawingToSVG.h>
// #ifdef _MSC_VER
// # pragma warning(pop)
// #endif

#include <boost/filesystem/path.hpp>

#include <QPainter>
#include <QSvgRenderer>
#include <QByteArray>
#include <QMimeData>
#include <QDragEnterEvent>

using namespace adwchem;

MolWidget::MolWidget(QWidget *parent) :  QWidget(parent)
{
    setAcceptDrops( true );
}

void
MolWidget::setMol( const RDKit::ROMol& mol )
{
    svg_ = adchem::drawing::toSVG( mol );
}

void
MolWidget::handleDropped( const QList< QUrl >& )
{
}

void
MolWidget::paintEvent( QPaintEvent * )
{
    QPainter painter( this );
    
	QByteArray svg( svg_.data(), svg_.size() );
    QSvgRenderer renderer( svg );
    // QRectF viewport = painter->viewport();
    // painter->scale( 1.0, 1.0 );
    // QRect target( 0, 0, rect.width(), rect.height() );
	renderer.render( &painter ); 
}

void
MolWidget::dragEnterEvent( QDragEnterEvent * event )
{
    static const wchar_t * extensions [] = {
        L".mol", L".sdf"
    };

	const QMimeData * mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
        for ( auto& url: urlList ) {
            boost::filesystem::path path( url.toLocalFile().toStdWString() );
            for ( auto ext: extensions ) {
                if ( path.extension() == ext ) {
                    event->accept();
                    return;
                }
            }
        }
	}
}

void
MolWidget::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
MolWidget::dragLeaveEvent( QDragLeaveEvent * event )
{
	event->accept();
}

void
MolWidget::dropEvent( QDropEvent * event )
{
	const QMimeData * mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
        emit dropped( urlList );
        event->accept();
	}
}

