// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef ADPLUGIN_H
#define ADPLUGIN_H

//#include "adplugin_global.h"
#include "manager.hpp"
#include <string>

class QString;
class QObject;
class QWidget;

namespace adportable {
	class Configuration;
	class Component;
}

#define iid_iMonitor               L"adplugin::ui::iMonitor"
#define iid_iControlMethodEditor   L"adplugin::ui::iControlMethodEditor"
#define iid_iLog                   L"adplugin::ui::iLog"
#define iid_iSequence              L"adplugin::ui::iSequence"
#define iid_iSequencesForm         L"adplugin::ui::iSequencesForm"

#if 0
# define EXPORT_FACTORY( FACTORY_CLASS ) \
    extern "C" {  __declspec(dllexport) adplugin::ifactory * ad_plugin_instance(); } \
    adplugin::ifactory * ad_plugin_instance() { return new FACTORY_CLASS; }
#endif

#include <qglobal.h>
#define EXPORT_FACTORY( FACTORY_CLASS ) \
    extern "C" {							\
	Q_DECL_EXPORT adplugin::ifactory * ad_plugin_instance();	\
    }									\
    adplugin::ifactory * ad_plugin_instance() { return new FACTORY_CLASS; }

#endif // ADPLUGIN_H
