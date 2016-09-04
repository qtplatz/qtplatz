/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include "acqiris.hpp"
#include "task.hpp"
#include "waveform.hpp"
#include <iostream>

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

acqiris *
document::digitizer()
{
    static acqiris __acqiris;
    return &__acqiris;
}

document::document( QObject * parent ) : QObject( parent )
{
}

document::~document()
{
}

bool
document::initialSetup()
{
    task::instance()->initialize();
    
    auto aqrs = digitizer();
    if ( aqrs->initialize() ) {
        if ( aqrs->findDevice() ) {
            task::instance()->strand().post( [&]{ digitizer()->digitizer_setup( 0.0, 10.0e-6 ); } );
            task::instance()->strand().post( [&]{ task::instance()->acquire( digitizer() ); } );
        }
    }
}

bool
document::finalClose()
{
    task::instance()->finalize();
    return true;
}

QSettings *
document::settings()
{
}

void
document::push( std::shared_ptr< waveform >&& d )
{
    que_.emplace_back( d );
    if ( que_.size() >= 4096 )
        que_.erase( que_.begin(), que_.begin() + 2048  );
}

std::shared_ptr< waveform >
document::recentWaveform()
{
    if ( !que_.empty() )
        return que_.back();
    return nullptr;
}
