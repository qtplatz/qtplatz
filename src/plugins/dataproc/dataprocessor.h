// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <coreplugin/ifile.h>
#include <boost/smart_ptr.hpp>

namespace adcontrols {
    class datafile;
    class LCMSDataSet;
}

namespace dataproc {

    class datafileimpl;

    class Dataprocessor : QObject {
        Q_OBJECT
    public:
        ~Dataprocessor();
        Dataprocessor();

        bool open( const QString& );
        Core::IFile * ifile();

        QString filename() const;
        adcontrols::datafile& file();
        adcontrols::LCMSDataSet* getLCMSDataset();

    signals:

    public slots:
            // void slotTitleChanged( const QString& title );

    private:
        boost::scoped_ptr< datafileimpl > datafileimpl_;
    };

} // dataproc

#endif // DATAPROCESSOR_H
