// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ADPLUGIN_H
#define ADPLUGIN_H

//#include "adplugin_global.h"
#include "manager.h"
#include <string>

class QString;
class QObject;
class QWidget;

namespace adportable {
	class Configuration;
	class Component;
}

namespace adplugin {

    static const wchar_t * iid_iMonitor   =           L"adplugin::ui::iMonitor";
    static const wchar_t * iid_iControlMethodEditor = L"adplugin::ui::iControlMethodEditor";
    static const wchar_t * iid_iLog       =           L"adplugin::ui::iLog";
    static const wchar_t * iid_iSequence  =           L"adplugin::ui::iSequence";
    static const wchar_t * iid_iSequencesForm =       L"adplugin::ui::iSequencesForm";

}

#define EXPORT_FACTORY( FACTORY_CLASS ) \
    extern "C" {  __declspec(dllexport) adplugin::ifactory * ad_plugin_instance(); } \
    adplugin::ifactory * ad_plugin_instance() { return new FACTORY_CLASS; }

#endif // ADPLUGIN_H
