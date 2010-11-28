//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#if defined _DEBUG
#     pragma comment(lib, "adwidgetsd.lib")
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "adplugind.lib")
#     pragma comment(lib, "adcontrolsd.lib")
#     pragma comment(lib, "acewrapperd.lib")
#     pragma comment(lib, "qtwrapperd.lib")
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "QAxContainerd.lib")
#     pragma comment(lib, "xmlwrapperd.lib")
#else
#     pragma comment(lib, "adwidgets.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "adplugin.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "qtwrapper.lib")
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "QAxContainer.lib")
#     pragma comment(lib, "xmlwrapper.lib")
#endif
