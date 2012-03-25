#ifndef CHEMSPIDER_HPP
#define CHEMSPIDER_HPP

#include "chemspider_global.hpp"
#include <extensionsystem/iplugin.h>
#include <boost/smart_ptr.hpp>

namespace adportable { class Configuration; }

namespace ChemSpider {	namespace Internal {

	class ChemSpiderMode;
    class ChemSpiderManager;

	class ChemSpiderPlugin : public ExtensionSystem::IPlugin
	{
		Q_OBJECT
    
	public:
		ChemSpiderPlugin();
		~ChemSpiderPlugin();
    
		bool initialize(const QStringList &arguments, QString *errorString);
		void extensionsInitialized();
		// ShutdownFlag aboutToShutdown();
    
	private slots:
		void triggerAction();
	private:
		boost::scoped_ptr< ChemSpiderMode > mode_;
		boost::scoped_ptr< ChemSpiderManager > manager_;
		boost::scoped_ptr< adportable::Configuration > pConfig_;
	};

} // namespace Internal
} // namespace ChemSpider

#endif // CHEMSPIDER_HPP

