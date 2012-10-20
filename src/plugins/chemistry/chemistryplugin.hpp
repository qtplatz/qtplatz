#ifndef CHEMISTRY_H
#define CHEMISTRY_H

#include "chemistry_global.h"
#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>

namespace Chemistry { namespace Internal {

  class ChemistryMode;
  class ChemistryManager;

  class ChemistryPlugin : public ExtensionSystem::IPlugin {
    Q_OBJECT
    
  public:
    ChemistryPlugin();
    ~ChemistryPlugin();
    
    bool initialize(const QStringList &arguments, QString *errorString);
    void extensionsInitialized();
  private:
	  boost::scoped_ptr< ChemistryMode > mode_;
	  boost::scoped_ptr< ChemistryManager > manager_;
    
  private slots:
    void triggerAction();
  };

} // namespace Internal
} // namespace Chemistry

#endif // CHEMISTRY_H

