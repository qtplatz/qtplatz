/**************************************************************************
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
** Author: Toshinobu Hondo, Ph.D.
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

#include "threshold_result_accessor.hpp"
#include <adacquire/signalobserver.hpp>

namespace acqrscontrols {

    template< typename T > class ACQRSCONTROLSSHARED_EXPORT waveform_accessor_;

    class pkd_counting_data_writer : public adacquire::SignalObserver::DataWriter {
    public:
        pkd_counting_data_writer( std::shared_ptr< acqrscontrols::waveform_accessor_< acqrscontrols::u5303a::waveform > > a );
        bool write( adfs::filesystem& fs ) const override;

        //
        static bool prepare_storage( adfs::filesystem& fs );
    private:
        bool write( adfs::filesystem& fs, const acqrscontrols::u5303a::waveform& ) const;
    };

}

