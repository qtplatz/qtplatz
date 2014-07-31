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

#include "progresswnd.hpp"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QColor>
#include <QFrame>
#include <QProgressBar>

using namespace adwidgets;

ProgressWnd * ProgressWnd::instance_;

ProgressWnd * 
ProgressWnd::instance()
{
    if ( !instance_ )
        instance_ = new ProgressWnd;
    return instance_;
}

ProgressWnd::~ProgressWnd()
{
}

ProgressWnd::ProgressWnd(QWidget *parent) : QDialog( parent, Qt::Tool )
                                          , stop_requested_( false )
{
    QPalette pal;
    QColor background( 128, 128, 128, 128 );
    pal.setColor( QPalette::All, QPalette::Window, background );
    setPalette(pal);

    resize(200,100);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum );
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    setSizePolicy( sizePolicy );
    
    auto topLayout = new QVBoxLayout(this);
    //topLayout->setMargin( 0 );
    topLayout->setSpacing( 2 );
    topLayout->setContentsMargins( 9, 9, 7, 7 );
    
    layout_ = new QGridLayout;
    topLayout->addLayout( layout_ );

    //setWindowFlags seems not working on Mac, so change Qt:Tool in QDialog ctor
    //setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint );
    //setWindowFlags( windowFlags() | Qt::FramelessWindowHint );

    connect( this, &ProgressWnd::onProgress, this, &ProgressWnd::handleProgress );
    connect( this, &ProgressWnd::onRemove, this, &ProgressWnd::handleRemove );
    connect( this, &ProgressWnd::onAdd, this, &ProgressWnd::handleAdd );
}

void
ProgressWnd::handleAdd( int id )
{
    auto progressBar = new QProgressBar( this );
    progressive_[ id ] = progressBar;

    progressBar->setStyleSheet( QString( "QProgressBar { color: lightgreen}" ) );

    progressBar->setObjectName(QStringLiteral("progressBar"));
    QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(progressBar->sizePolicy().hasHeightForWidth());
    progressBar->setSizePolicy(sizePolicy1);
    progressBar->setMaximumSize(QSize(16777215, 12));
    progressBar->setBaseSize(QSize(0, 0));
    progressBar->setValue(24);
    
    layout_->addWidget( progressBar, int( progressive_.size() ), 0, 1, 1 );
}

void
ProgressWnd::stop()
{
    stop_requested_ = true;
}

std::shared_ptr< Progress >
ProgressWnd::addbar()
{
    int id = idGen_++;
    emit onAdd( id );
    return std::make_shared< Progress >( this, id );
}

void
ProgressWnd::handleProgress( int id, int current, int total )
{
    show();
    raise();
    auto it = progressive_.find( id );
    if ( it != progressive_.end() ) {
        auto bar = it->second;
        if ( current == 0 ) {
            bar->setRange( 0, total );
            bar->setVisible( true );
            bar->setTextVisible( true );
            bar->setValue( 3.0 );
        } else 
            bar->setValue( current );
    }
}

void
ProgressWnd::handleRemove( int id )
{
    auto it = progressive_.find( id );
    if ( it != progressive_.end() ) {

        progressive_.erase( it );
        if ( progressive_.empty() ) { // this is GUI thread
            ProgressWnd * tmp = instance_;
            instance_ = 0;

            QLayoutItem * child;
            while ( (child = layout_->takeAt( 0 )) != 0 )
                delete child;
            delete tmp;
        }
    }
}

////////////////////////////

Progress::Progress(ProgressWnd * p, int id) : wnd_(p), id_(id)
{
}

Progress::~Progress()
{
    emit wnd_->onRemove( id_ );
}

bool
Progress::operator()( int current, int total )
{
    emit wnd_->onProgress( id_, current, total );
    return wnd_->stop_requested_;
}
