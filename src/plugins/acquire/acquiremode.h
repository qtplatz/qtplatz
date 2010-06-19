#ifndef ACQUIREMODE_H
#define ACQUIREMODE_H

#include <coreplugin/basemode.h>

namespace Acquire {
  namespace internal {
    
    class AcquireMode : public Core::BaseMode {
      Q_OBJECT
	;
    public:
      ~AcquireMode();
      explicit AcquireMode(QObject *parent = 0);

    signals:

    public slots:

    private:

    };
    //---------
  }
}

#endif // ACQUIREMODE_H
