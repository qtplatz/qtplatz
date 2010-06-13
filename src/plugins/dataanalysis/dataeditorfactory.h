#ifndef DATAEDITORFACTORY_H
#define DATAEDITORFACTORY_H

#include <coreplugin/editormanager/ieditorfactory.h>

namespace DataAnalysis {
    namespace Internal {

        class DataAnalysisPlugin;
        struct DataEditorFactoryData;

        class DataEditorFactory : public Core::IEditorFactory
        {
            Q_OBJECT

        public:
            ~DataEditorFactory();
            DataEditorFactory( DataAnalysisPlugin * owner );

            QStringList mimeTypes() const;
            QString kind() const;
            Core::IEditor * createEditor( QWidget * parment );
            Core::IFile * open( const QString & filename );

        private:
            DataEditorFactoryData * d_;
        };

    }
}


#endif // DATAEDITORFACTORY_H
