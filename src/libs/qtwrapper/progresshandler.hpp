/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <QObject>
#include <QFuture>

namespace qtwrapper {

    class ProgressHandler : public QObject {
        Q_OBJECT
    public:
        QFutureInterface<void> progress;
        int currentValue;

        ProgressHandler( const ProgressHandler& t ) : QObject( 0 )
                                                    , progress( t.progress )
                                                    , currentValue( t.currentValue ) {
        }

        ~ProgressHandler() {
            progress.reportFinished();
            disconnect( this, &ProgressHandler::setProgressRange, this, &ProgressHandler::handleProgressSetRange );
            disconnect( this, &ProgressHandler::setProgressValue, this, &ProgressHandler::handleProgressSetValue );            
        }

        ProgressHandler( int beg = 0, int end = 100 ) : currentValue(beg) {
            connect( this, &ProgressHandler::setProgressRange, this, &ProgressHandler::handleProgressSetRange );
            connect( this, &ProgressHandler::setProgressValue, this, &ProgressHandler::handleProgressSetValue );
            progress.reportStarted();
            progress.setProgressRange( beg, end );
            
        }
        void operator()( int beg, int end ) { currentValue = beg; emit setProgressRange( beg, end ); }
        void operator()( int value ) { emit setProgressValue( value ); }
        bool operator()() { emit setProgressValue( ++currentValue ); return false; }

    private slots:
        void handleProgressSetValue( int value ) { progress.setProgressValue( value ); }
        void handleProgressSetRange( int beg, int end ) { progress.setProgressRange( beg, end ); }

    signals:
        void setProgressValue( int value );
        void setProgressRange( int beg, int end );
    };
}
