//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "servant.hpp"

#if defined _DEBUG
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "adcontrolsd.lib")
#     pragma comment(lib, "adplugind.lib")
#     pragma comment(lib, "qtwrapperd.lib")
#     pragma comment(lib, "xmlparserd.lib")
#     pragma comment(lib, "acewrapperd.lib")
#else
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "adplugin.lib")
#     pragma comment(lib, "qtwrapper.lib")
#     pragma comment(lib, "xmlparser.lib")
#     pragma comment(lib, "acewrapper.lib")
#endif

#  if defined _DEBUG
#     pragma comment(lib, "ACEd.lib")
#     pragma comment(lib, "TAOd.lib")
#     pragma comment(lib, "TAO_Utilsd.lib")
#     pragma comment(lib, "TAO_PId.lib")
#     pragma comment(lib, "TAO_PortableServerd.lib")
#     pragma comment(lib, "TAO_AnyTypeCoded.lib")
#     pragma comment(lib, "adcontrollerd.lib")
#     pragma comment(lib, "adbrokerd.lib")
#     pragma comment(lib, "adinterfaced.lib")
#  else
#     pragma comment(lib, "ACE.lib")
#     pragma comment(lib, "TAO.lib")
#     pragma comment(lib, "TAO_Utils.lib")
#     pragma comment(lib, "TAO_PI.lib")
#     pragma comment(lib, "TAO_PortableServer.lib")
#     pragma comment(lib, "TAO_AnyTypeCode.lib")
#     pragma comment(lib, "adcontroller.lib")
#     pragma comment(lib, "adbroker.lib")
#     pragma comment(lib, "adinterface.lib")
#  endif

Servant::Servant()
{
}
