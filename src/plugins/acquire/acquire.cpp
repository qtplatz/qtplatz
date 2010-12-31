//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "acquire.h"

# pragma warning (disable: 4996)
# include <ace/Service_Config.h>
# include <ace/Process_Manager.h>
# pragma warning (default: 4996)

#if defined _DEBUG
#     pragma comment(lib, "adcontrollerd.lib")
#     pragma comment(lib, "adcontrolsd.lib")   // static
#     pragma comment(lib, "adutilsd.lib")      // static
#     pragma comment(lib, "adinterfaced.lib")  // static
#     pragma comment(lib, "adportabled.lib")   // staitc
#     pragma comment(lib, "acewrapperd.lib")   // static
#     pragma comment(lib, "qtwrapperd.lib")    // static
#     pragma comment(lib, "xmlwrapperd.lib")   // static
#     pragma comment(lib, "adplugind.lib")     // dll
#     pragma comment(lib, "QAxContainerd.lib")
#else
#     pragma comment(lib, "adcontroller.lib")
#     pragma comment(lib, "adcontrols.lib")    // static
#     pragma comment(lib, "adutils.lib")       // static
#     pragma comment(lib, "adinterface.lib")   // static
#     pragma comment(lib, "adportable.lib")    // static
#     pragma comment(lib, "acewrapper.lib")    // static
#     pragma comment(lib, "qtwrapper.lib")     // static
#     pragma comment(lib, "xmlwrapper.lib")    // static
#     pragma comment(lib, "adplugin.lib")      // dll
#     pragma comment(lib, "QAxContainer.lib")
#endif

#if defined ACE_WIN32
#  if defined _DEBUG
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "ACEd.lib")
#  else
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "ACE.lib")
#  endif
#endif



acquire::acquire()
{
}
