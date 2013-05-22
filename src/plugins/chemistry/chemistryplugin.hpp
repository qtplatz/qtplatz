/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
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
#ifndef CHEMISTRY_H
#define CHEMISTRY_H

#include "chemistry_global.hpp"
#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>

namespace chemistry { 

  class ChemistryMode;
  class ChemistryMainWindow;

  class ChemistryPlugin : public ExtensionSystem::IPlugin {
	  Q_OBJECT
		  
  public:
	  ChemistryPlugin();
	  ~ChemistryPlugin();
	  
	  bool initialize(const QStringList &arguments, QString *errorString);
	  void extensionsInitialized();
  private:
	  boost::scoped_ptr< ChemistryMode > mode_;
	  boost::scoped_ptr< ChemistryMainWindow > mainWindow_;
	  
  private slots:
	  void triggerAction();
  };
	  
} // namespace chemistry

#endif // CHEMISTRY_H

