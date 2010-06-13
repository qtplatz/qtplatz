#include "dataeditor.h"
#include "datafile.h"
#include <coreplugin/uniqueidmanager.h>
#include "dataanalysiswindow.h"

using namespace DataAnalysis::Internal;

struct DataEditorData {
    DataEditorData() : widget_(0), file_(0) {}

    DataAnalysisWindow * widget_;
    QString displayName_;
    Datafile * file_;
    QList<int> context_;
};


DataEditor::~DataEditor()
{
    delete d_;
}

DataEditor::DataEditor( DataAnalysisWindow * widget ) : Core::IEditor( widget )
{
    d_ = new DataEditorData;
    if ( d_ ) {
        d_->widget_ = widget;
        d_->file_ = new Datafile( this, widget );

        Core::UniqueIDManager * uidm = Core::UniqueIDManager::instance();
        
        d_->context_ << uidm->uniqueIdentifier( "Data Editor" );

        connect( d_->widget_, SIGNAL(contentModified()), d_->file_, SLOT(modified()));
        connect( d_->widget_, SIGNAL(titleChanged(QString)), this, SLOT(slotTitleChanged(QString)));
        connect( d_->widget_, SIGNAL(contentModified()), this, SIGNAL(changed() ) );
    }
}

QWidget * DataEditor::widget()
{
	return d_->widget_;
}

QList<int>
DataEditor::context() const
{
    return d_->context_;
}

Core::IFile *
DataEditor::file()
{
	return d_->file_;
}

bool
DataEditor::createNew( const QString& contents )
{
	Q_UNUSED(contents);

	d_->widget_->setContent( QByteArray() );
	//d_->file_->setFilename( QString() );

	return true;
}

bool
DataEditor::open( const QString& filename )
{
	return true;
}


bool 
DataEditor::restoreState( const QByteArray& )
{
	return true;
}

QByteArray 
DataEditor::saveState() const
{
	return QByteArray();
}

void 
DataEditor::setDisplayName( const QString& title )
{
}

QToolBar * 
DataEditor::toolBar()
{
	return 0;
}

const char *
DataEditor::kind() const
{
	return "Data Editor";
}

QString
DataEditor::displayName() const
{
	return d_->displayName_;
}

bool
DataEditor::duplicateSupported() const
{
	return false;
}

bool
DataEditor::isTemporary() const
{
	return false;
}

Core::IEditor *
DataEditor::duplicate( QWidget * parent )
{
	return 0;
}