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

#include "threshold_result_accessor.hpp"
#include <adacquire/signalobserver.hpp>

namespace acqrscontrols {

    class counting_data_writer : public adacquire::SignalObserver::DataWriter {
    public:
        counting_data_writer( std::shared_ptr< threshold_result_accessor > a ) : DataWriter( a ) {
        }
        
        bool write( adfs::filesystem& fs ) const override;
    };

    ///////////////////////////////////////////////////////////////////////////////
    // T := acqrscontrols::ap240::threshold_result

    template< typename T >
    class counting_data_writer_ : public adacquire::SignalObserver::DataWriter {
    public:
        typedef threshold_result_accessor_< T > threshold_result_accessor_type;
        counting_data_writer_( std::shared_ptr< threshold_result_accessor_type > a ) : DataWriter( a ) {
        }
        
        bool write( adfs::filesystem& fs ) const override;
    };

}

