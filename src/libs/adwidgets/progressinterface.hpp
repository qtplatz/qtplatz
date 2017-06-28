/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

namespace adwidgets {

    class ProgressInterface {

    public:
        QFutureInterface<void> progress;
        int currentValue;

        ~ProgressInterface() {
            progress.reportFinished();
        }

        ProgressInterface( int beg = 0, int end = 100 ) {
            progress.setProgressRange( beg, end );
            progress.reportStarted();
        }

        ProgressInterface( const ProgressInterface& t ) : progress( t.progress )
                                                        , currentValue( t.currentValue ) {
        }

        void operator()( int beg, int end ) {
            currentValue = beg;
            progress.setProgressRange( beg, end );
        }

        void operator()( int value ) {
            progress.setProgressValue( value );
        }
        
        bool operator()() {
            progress.setProgressValue( ++currentValue );
            return false;
        }
    };
}
