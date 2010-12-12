//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "portfolio.h"
#include "portfolioimpl.h"
#include "folder.h"
#include "folium.h"

# if defined _DEBUG
#    pragma comment(lib, "adcontrolsd.lib")
#    pragma comment(lib, "xmlwrapperd.lib")
#    pragma comment(lib, "adportabled.lib")
# else
#    pragma comment(lib, "adcontrols.lib")
#    pragma comment(lib, "xmlwrapper.lib")
#    pragma comment(lib, "adportable.lib")
# endif

using namespace portfolio;

Portfolio::~Portfolio()
{
}

Portfolio::Portfolio() : impl_( new internal::PortfolioImpl() )
{
}

Portfolio::Portfolio( const Portfolio& t ) : impl_(t.impl_)
{
}

Portfolio::Portfolio( const std::wstring& xml ) : impl_( new internal::PortfolioImpl( xml ) )
{
}

