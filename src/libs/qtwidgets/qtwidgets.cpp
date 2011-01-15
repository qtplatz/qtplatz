//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "qtwidgets.h"

#if defined WIN32
# if defined _DEBUG
#     pragma comment(lib, "adportabled.lib")
#     pragma comment(lib, "adcontrolsd.lib")
#     pragma comment(lib, "adplugind.lib")
# else
#     pragma comment(lib, "adportable.lib")
#     pragma comment(lib, "adcontrols.lib")
#     pragma comment(lib, "adplugin.lib")
# endif
#endif


QtWidgets::QtWidgets()
{
}
