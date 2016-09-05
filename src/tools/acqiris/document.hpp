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

#pragma once

#include <QObject>
#include <boost/numeric/ublas/fwd.hpp>
#include <boost/msm/back/state_machine.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>

class QSettings;
class digitizer;
class waveform;

class document : public QObject {
    Q_OBJECT
    ~document();
    document( const document& ) = delete;
    document( QObject * parent = 0 );
public:
    static document * instance();
    static class digitizer * digitizer();

    bool initialSetup();
    bool finalClose();
    QSettings * settings();

    void push( std::shared_ptr< waveform >&& );
    std::shared_ptr< waveform > recentWaveform();

signals:
    void updateData();
    
private:
    std::deque< std::shared_ptr< waveform > > que_;
};
