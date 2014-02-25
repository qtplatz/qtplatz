/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@MS-Cheminformatics.com
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
** General Public License version 2.1 as published by the Free Software/ 
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

// This module is depending on CompassXtract 3.1.2, Bruker Daltonics
// http://www.bdal.com/nc/library/downloads-patches.html?tx_abdownloads_pi1%5Bcategory_uid%5D=34

#include "compassxtract.hpp"
#include "datafile_factory.hpp"

Compassxtract::Compassxtract()
{
}

namespace adcontrols {
    class datafile_factory;
}

extern "C" {
	COMPASSXTRACTSHARED_EXPORT adcontrols::datafile_factory * datafile_factory();
	COMPASSXTRACTSHARED_EXPORT adplugin::plugin * adplugin_plugin_instance();
}

adcontrols::datafile_factory *
datafile_factory()
{
	return new compassxtract::datafile_factory();
}

adplugin::plugin *
adplugin_plugin_instance()
{
    return new compassxtract::datafile_factory();
}
