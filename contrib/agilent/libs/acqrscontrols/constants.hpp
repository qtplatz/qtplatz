/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
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

namespace acqrscontrols {

    namespace u5303a {
        const char * const waveform_observer_name       = "1.u5303a.ms-cheminfo.com";
        const char * const histogram_observer_name      = "histogram.1.u5303a.ms-cheminfo.com";
        const char * const tdcdoc_avgr_observer_name    = "tdcdoc.waveform.1.u5303a.ms-cheminfo.com";
        const char * const tdcdoc_traces_observer_name  = "tdcdoc.traces.1.u5303a.ms-cheminfo.com";
        enum { nchannels = 2 };
    }

    namespace ap240 {
        const char * const waveform_observer_name   = "1.ap240.ms-cheminfo.com";
        const char * const histogram_observer_name  = "histogram.1.ap240.ms-cheminfo.com";
        enum { nchannels = 2 };        
    }
    
}
