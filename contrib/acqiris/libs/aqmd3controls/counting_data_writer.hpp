/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "pkd_result_accessor.hpp"
#include <adacquire/signalobserver.hpp>
#include <adacquire/datawriter.hpp>

namespace aqmd3controls {

    template< typename T > class AQMD3CONTROLSSHARED_EXPORT waveform_accessor_;

    class AQMD3CONTROLSSHARED_EXPORT counting_data_writer : public adacquire::SignalObserver::DataWriter {
    public:
        counting_data_writer( std::shared_ptr< pkd_result_accessor > a ) : DataWriter( a ) {
        }

        bool write( adfs::filesystem& fs, const boost::uuids::uuid& ) const override;
        static bool prepare_storage( adfs::filesystem& fs );
    };

    ///////////////////////////////////////////////////////////////////////////////
    // T := aqmd3::threshold_result

    template< typename T >
    class counting_data_writer_ : public adacquire::SignalObserver::DataWriter {
    public:
        typedef pkd_result_accessor_< T > pkd_result_accessor_type;
        counting_data_writer_( std::shared_ptr< pkd_result_accessor_type > a ) : DataWriter( a ) {
        }

        bool write( adfs::filesystem& fs ) const override;
    };

}
