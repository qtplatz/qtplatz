// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "adtofprocessor_global.hpp"
#include <adprocessor/dataprocessor.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>

namespace adtofprocessor {

    class AddContextMenu;
    class EstimateScanLaw;
    class OnCreate;
    
    class ProcessReactor {
        ProcessReactor();
        ProcessReactor( const ProcessReactor& ) = delete;
        ProcessReactor& operator = ( const ProcessReactor& ) = delete;
    public:

        void initialSetup();
        void finalClose();

    public:
        ~ProcessReactor();
        static ProcessReactor * instance();
    private:
        const boost::uuids::uuid clsid_; // InfiTOF uuid
        std::unique_ptr< OnCreate > onCreate_;
        std::unique_ptr< AddContextMenu > addContextMenu_;
        std::unique_ptr< EstimateScanLaw > estimateScanLaw_;
    };

}
