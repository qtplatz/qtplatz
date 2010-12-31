//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#if defined _DEBUG
#     pragma comment(lib, "adwidgetsd.lib")   // dll
#     pragma comment(lib, "adportabled.lib")  // static
#     pragma comment(lib, "adplugind.lib")    // dll
#     pragma comment(lib, "adcontrolsd.lib")  // static
#     pragma comment(lib, "adutilsd.lib")     // static
#     pragma comment(lib, "acewrapperd.lib")  // static
#     pragma comment(lib, "qtwrapperd.lib")   // static
#     pragma comment(lib, "ACEd.lib")         // dll
#     pragma comment(lib, "QAxContainerd.lib")
#     pragma comment(lib, "xmlwrapperd.lib")  // static
#     pragma comment(lib, "portfoliod.lib")   // dll
#     pragma comment(lib, "adutilsd.lib")     // static
#else
#     pragma comment(lib, "adwidgets.lib")
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "adplugin.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "adutils.lib")      // static
#     pragma comment(lib, "acewrapper.lib")
#     pragma comment(lib, "qtwrapper.lib")
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "QAxContainer.lib")
#     pragma comment(lib, "xmlwrapper.lib")
#     pragma comment(lib, "portfolio.lib")
#     pragma comment(lib, "adutils.lib")
#endif
