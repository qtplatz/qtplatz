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

#include "datafile_factory.hpp"
#include "shrader_lrpfile_global.hpp"
#include <boost/filesystem.hpp>

extern "C" {
    SHRADER_LRPFILE_LIBRARY_EXPORT adcontrols::datafile_factory * datafile_factory();
	SHRADER_LRPFILE_LIBRARY_EXPORT adplugin::plugin * adplugin_plugin_instance();
}

adplugin::plugin *
adplugin_plugin_instance()
{
#if defined _MSC_VER
    // Workaround for boost/VC bug #6320 according to following artcile
    // https://svn.boost.org/trac/boost/ticket/6320
    boost::filesystem::path p("dummy");
#endif   
    return new shrader::datafile_factory();
}

