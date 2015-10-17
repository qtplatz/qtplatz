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
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#include "fticr.hpp"
#include "datafile_factory.hpp"
#include <boost/filesystem.hpp>
#include <memory>
#include <mutex>

static std::shared_ptr< fticr::datafile_factory > instance_;
static std::once_flag flag;

FTICR::FTICR()
{
}

namespace adcontrols {
    class datafile_factory;
}

extern "C" {
    FTICRSHARED_EXPORT adcontrols::datafile_factory * datafile_factory();
	FTICRSHARED_EXPORT adplugin::plugin * adplugin_plugin_instance();
}

adcontrols::datafile_factory *
datafile_factory()
{
	return new fticr::datafile_factory();
}

adplugin::plugin *
adplugin_plugin_instance()
{
#if defined _MSC_VER
    // Workaround for boost/VC bug #6320 according to following artcile
    // https://svn.boost.org/trac/boost/ticket/6320
    boost::filesystem::path p("dummy");
#endif

    std::call_once( flag, [](){ instance_ = std::make_shared< fticr::datafile_factory >(); } );
    return instance_.get();
}
