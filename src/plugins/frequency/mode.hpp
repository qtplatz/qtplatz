#ifndef FREQUENCYMODE_HPP
#define FREQUENCYMODE_HPP

#include <coreplugin/basemode.h>

namespace frequency {

    class Mode : public Core::BaseMode {
        Q_OBJECT
    public:
        explicit Mode(QObject *parent = 0);
        ~Mode();
    private slots:
        void grabEditorManager( Core::IMode * mode );
    signals:
            
    public slots:
        
    };
}

#endif // FREQUENCYMODE_HPP
