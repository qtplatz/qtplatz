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

#include "dropwidget.hpp"
#include <adcontrols/datafile.hpp>
#include <boost/filesystem.hpp>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>
#include <QPainter>
#include <QLabel>
#include <QGridLayout>
#include <QDebug>

namespace batchproc {

    struct scoped_icon_state {
        QLabel& label_;
        QIcon& icon_;
        QIcon::Mode mode_;
        QIcon::State state_;
        bool enable_;

        scoped_icon_state( QLabel& label, QIcon& icon, QIcon::Mode mode, QIcon::State state, bool enable )
            : label_( label ), icon_(icon) {

            enable_ = label.isEnabled();
            label_.setEnabled( enable );
        }
        ~scoped_icon_state() { 
            label_.setEnabled( enable_ );
        }
    };

}

using namespace batchproc;

DropWidget::DropWidget(QWidget *parent) :  QWidget(parent)
	, icon_( ":/batchproc/images/drop128.png" )
{
    setAcceptDrops( true );

    label_.setAlignment( Qt::AlignCenter );
    label_.setFrameShape( QFrame::Box );
	label_.setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    label_.setBackgroundRole( QPalette::Base );
    label_.setAutoFillBackground( true );
    label_.setMinimumSize( 132, 132 );

    label_.setPixmap( icon_.pixmap( QSize(128,128), QIcon::Normal, QIcon::On ) );
    label_.setEnabled( false );

	QGridLayout * layout = new QGridLayout;
	setLayout( layout );
	layout->addWidget( &label_ );
}

void
DropWidget::dragEnterEvent( QDragEnterEvent * event )
{
    bool canAccept = false;
	const QMimeData * mimeData = event->mimeData();
	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
		for ( int i = 0; i < urlList.size(); ++i ) {
			boost::filesystem::path path( urlList.at(i).toLocalFile().toStdWString() );
            if ( boost::filesystem::is_regular_file( path ) ) {
                if ( adcontrols::datafile::access( path.wstring() ) || path.parent_path().extension() == L".d" )
                    canAccept = true;
            } else if ( boost::filesystem::is_directory( path ) ) {
                if ( path.extension() == L".d" ) {
                    canAccept = true;
                } else {
                    boost::filesystem::directory_iterator edir;
                    for ( boost::filesystem::directory_iterator dir( path ); dir != edir; ++dir ) {
                        if ( boost::filesystem::is_directory( dir->status() ) && dir->path().extension() == L".d" ) {
                            canAccept = true;
                            break;
                        }
                    }
                }
            }
        }
	}
    if ( canAccept ) {
        label_.setPixmap( icon_.pixmap( QSize(128,128), QIcon::Active, QIcon::On ) );
        label_.setEnabled( true );
        event->accept();
    }
}

void
DropWidget::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
DropWidget::dragLeaveEvent( QDragLeaveEvent * event )
{
    label_.setPixmap( icon_.pixmap( QSize(128,128), QIcon::Normal, QIcon::On ) );
    label_.setEnabled( false );
	event->accept();
}

void
DropWidget::dropEvent( QDropEvent * event )
{
	const QMimeData * mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
        emit dropFiles( urlList );
	}
    label_.setPixmap( icon_.pixmap( QSize(128,128), QIcon::Normal, QIcon::On ) );
    label_.setEnabled( false );
}

