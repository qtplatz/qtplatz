#include "datafile.h"

using namespace DataAnalysis::Internal;


namespace DataAnalysis {
    namespace Internal {

        struct DatafileData {
            DatafileData() : mimeType_( "text/html" )
                            , editorWidget_(0)
                            , editor_(0)
                            , modified_(false) {}
            const QString mimeType_;
            DataAnalysisWindow * editorWidget_;
            DataEditor * editor_;
            QString fileName_;
            bool modified_;
        };
    }
}

Datafile::Datafile( DataEditor * editor, DataAnalysisWindow * window ) : d_(0)
{
    d_ = new DatafileData;
    d_->editor_ = editor;
    d_->editorWidget_ = window;
}

Datafile::~Datafile()
{
}

void
Datafile::setModified( bool value )
{
    if ( d_ )
        d_->modified_ = value;
}

QString
Datafile::mimeType() const
{
     return d_->mimeType_;
}


bool
Datafile::save( const QString & filename )
{
    return false;
}

bool
Datafile::open( const QString & filename )
{
    return false;
}

QString 
Datafile::fileName() const
{
	return QString();
}

QString 
Datafile::defaultPath() const
{
	return QString();
}

QString
Datafile::suggestedFileName() const
{
	return QString();
}

bool
Datafile::isModified() const
{
	return true;
}

bool
Datafile::isReadOnly() const
{
	return false;
}

bool
Datafile::isSaveAsAllowed() const
{
	return true;
}

void
Datafile::modified( Core::IFile::ReloadBehavior * )
{
}


