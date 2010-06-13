#include "dataeditorfactory.h"
#include <QStringList>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/ifilefactory.h>
#include <coreplugin/editormanager/ieditor.h>

#include "dataanalysiswindow.h"
#include "dataeditor.h"

using namespace DataAnalysis::Internal;

namespace DataAnalysis {
    namespace Internal {

        struct DataEditorFactoryData {
            DataEditorFactoryData() : kind_( "Data Editor" ) {
                mimeTypes_ << "Data Editor";
            }
            QString kind_;
            QStringList mimeTypes_;
        };
    }
}

DataEditorFactory::~DataEditorFactory()
{
    delete d_;
}

DataEditorFactory::DataEditorFactory( DataAnalysisPlugin * owner )
{
    d_ = new DataEditorFactoryData;
}

QString
DataEditorFactory::kind() const
{
    return d_->kind_;
}

Core::IFile *
DataEditorFactory::open( const QString& filename )
{
	Core::EditorManager * em = Core::EditorManager::instance();
	Core::IEditor * iface = em->openEditor( filename, d_->kind_ );
	return iface ? iface->file() : 0;
}

QStringList
DataEditorFactory::mimeTypes() const
{
    return d_->mimeTypes_;
}

Core::IEditor *
DataEditorFactory::createEditor( QWidget * parent )
{
    DataAnalysisWindow * editorWidget = new DataAnalysisWindow( parent );
    DataEditor * editor = new DataEditor( editorWidget );
    return editor;
}
