/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#include "progressbar.hpp"
#include <QProgressBar>
#include <QFuture>
#include <coreplugin/icore.h>
#include <coreplugin/progressmanager/progressmanager.h>

using namespace qtwrapper;

qtwrapper::ProgressBar::ProgressBar(QObject *parent) : QObject(parent)//, progress_(0)
{
#if 0
    static int ident;
    
	if ( Core::ProgressManager * pMgr = Core::ICore::instance()->progressManager() ) {
		QFuture<void> future;
		type_ = QString("prog.%1").arg( ident++ );
		progress_ = pMgr->addTask( future, "chromatogram", type_, Core::ProgressManager::CloseOnSuccess ); 
	}
    connect( this, SIGNAL( on_dispose() ), this, SLOT( handleDispose() ) );
    connect( this, SIGNAL( started() ), this, SLOT( handleStarted() ) );
    connect( this, SIGNAL( finished() ), this, SLOT( handleFinished() ) );
    connect( this, SIGNAL( progressRange(int, int) ), this, SLOT( handleProgressRange(int, int) ) );
    connect( this, SIGNAL( progressValue(int) ), this, SLOT( handleProgressValue(int) ) );
    connect( this, SIGNAL( progressText(const QString&) ), this, SLOT( handleProgressText(const QString&) ) );
#endif
}

qtwrapper::ProgressBar::~ProgressBar()
{
}

void
qtwrapper::ProgressBar::dispose()
{
    emit on_dispose();
}

void
qtwrapper::ProgressBar::setStarted()
{
    emit started();
}


void
qtwrapper::ProgressBar::setFinished()
{
    emit finished();
}

void
qtwrapper::ProgressBar::setProgressRange( int min, int max )
{
    emit progressRange( min, max );
}

void
qtwrapper::ProgressBar::setProgressValue( int val )
{
    emit progressValue( val );
}

void
qtwrapper::ProgressBar::setProgressText( const QString& text )
{
    emit progressText( text );
}

void
qtwrapper::ProgressBar::handleStarted()
{
    //progress_->setVisible( true );
}

void
qtwrapper::ProgressBar::handleFinished()
{
    //progress_->setFinished();
    //progress_->setVisible( false );
    //progress_ = 0; // will reuse | destroy by Core library
	delete this;
}

void
qtwrapper::ProgressBar::handleDispose()
{
    handleFinished();
}

void
qtwrapper::ProgressBar::handleProgressRange( int min, int max )
{
    //progress_->setProgressRange( min, max );
}

void
qtwrapper::ProgressBar::handleProgressValue( int val )
{
    //progress_->setProgressValue( val );
}

void
qtwrapper::ProgressBar::handleProgressText( const QString& text )
{
    //progress_->setProgressText( text );
}

