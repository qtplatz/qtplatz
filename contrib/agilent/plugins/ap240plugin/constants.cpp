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

#include "constants.hpp"
#include <acqrscontrols/constants.hpp>
#include <adicontroller/signalobserver.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <QMetaType>
Q_DECLARE_METATYPE( boost::uuids::uuid );

namespace so = adicontroller::SignalObserver;

namespace ap240 {

    const boost::uuids::uuid u5303a_observer = boost::uuids::name_generator( so::Observer::base_uuid() )( acqrscontrols::u5303a::waveform_observer_name );

    const boost::uuids::uuid histogram_observer = boost::uuids::name_generator( so::Observer::base_uuid() )( acqrscontrols::u5303a::histogram_observer_name );

    const boost::uuids::uuid ap240_observer = boost::uuids::name_generator( so::Observer::base_uuid() )( acqrscontrols::ap240::waveform_observer_name );
    
}
