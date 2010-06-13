#ifndef DATAEDITOR_H
#define DATAEDITOR_H

#include <coreplugin/editormanager/ieditor.h>
#include <QToolBar>

struct DataEditorData;

namespace DataAnalysis {
    namespace Internal {

        class DataAnalysisWindow;

        class DataEditor : public Core::IEditor {
            Q_OBJECT

        public:
            ~DataEditor();
            DataEditor( DataAnalysisWindow * wedget );

            bool createNew( const QString& = QString() );
            QString displayName() const;
            IEditor * duplicate( QWidget * );
            bool duplicateSupported() const;
            Core::IFile * file();
            bool isTemporary() const;
            const char * kind() const;
            bool open( const QString& filename = QString() );
            bool restoreState( const QByteArray& );
            QByteArray saveState() const;
            void setDisplayName( const QString& title );
            QToolBar * toolBar();

            // Core::IContext
            QWidget * widget();
            QList<int> context() const;
        protected slots:
            void slotTitleChanged( const QString& title ) { setDisplayName( title ); }
        private:
            DataEditorData * d_;

        };

    }
}

#endif // DATAEDITOR_H
