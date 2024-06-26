/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <memory>
#include <vector>

namespace acqrscontrols {
    template<typename T> class threshold_result_;
    namespace ap240 { class waveform; }
}

namespace adfs { class sqlite; }

class ResultWriter {
public:
    ResultWriter( adfs::sqlite& db );

    ~ResultWriter();
    
    ResultWriter& operator << ( std::shared_ptr< const acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform > > );
    void commitData();
    
private:
    std::vector< std::shared_ptr< const acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform > > > cache_;
    adfs::sqlite& db_;
};

