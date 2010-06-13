#ifndef DATAFILE_H
#define DATAFILE_H

#include <coreplugin/ifile.h>

namespace DataAnalysis {
        namespace Internal {

        struct DatafileData;
        class DataEditor;
        class DataAnalysisWindow;

    class Datafile : public Core::IFile {
        Q_OBJECT
    public:
        ~Datafile();
        Datafile( DataEditor *, DataAnalysisWindow *);

        void setModified(bool value = true );
        bool save( const QString& );
        bool open( const QString& );
        QString mimeType() const;
        QString fileName() const;
		QString defaultPath() const;
		QString suggestedFileName() const;
		bool isModified() const;
		bool isReadOnly() const;
		bool isSaveAsAllowed() const;
		void modified( Core::IFile::ReloadBehavior * );
    protected slots:
        void modified() { setModified(true); }

    private:

        DatafileData * d_;

    };
  }
}

#endif // DATAFILE_H
