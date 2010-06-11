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
                                                                        , modified_(false)
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
  modified_ = value;
}

