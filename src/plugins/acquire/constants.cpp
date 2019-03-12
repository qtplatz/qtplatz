/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include <adacquire/signalobserver.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <QMetaType>
Q_DECLARE_METATYPE( boost::uuids::uuid );

namespace so = adacquire::SignalObserver;

namespace ads54j {

    // const boost::uuids::uuid ads54j_observer = boost::uuids::string_generator()( "1c64d0c3-d22c-4b79-a59c-4c752b90533a" );
    //const boost::uuids::uuid trace_observer = name_generator( so::Observer::base_uuid() )( acqrscontrols::ads54j::tdcdoc_traces_observer_name );
}
