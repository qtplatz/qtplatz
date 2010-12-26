// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <coreplugin/ifile.h>
#include <boost/smart_ptr.hpp>
#include <adcontrols/dataSubscriber.h>

namespace adcontrols {
    class datafile;
    class LCMSDataset;
}

namespace portfolio {
    class Portfolio;
    class Folium;
}

namespace dataproc {

    class datafileimpl;

    class Dataprocessor : QObject
                        , public adcontrols::dataSubscriber {
        Q_OBJECT
    public:
        ~Dataprocessor();
        Dataprocessor();

        bool open( const QString& );
        Core::IFile * ifile();

        QString filename() const;
        adcontrols::datafile& file();
        adcontrols::LCMSDataset* getLCMSDataset();
        portfolio::Portfolio getPortfolio();
        void setCurrentSelection( portfolio::Folium& );

        // implement adcontrols::dataSubscriber
        virtual void subscribe( adcontrols::LCMSDataset& );
        virtual void subscribe( adcontrols::ProcessedDataset& );
        // <------------------------

    signals:
        // void changeSelection( portfolio::Folium& );

    public slots:
        // void slotTitleChanged( const QString& title );
        // void handle_changeSelection( portfolio::Folium& );

    private:
        boost::scoped_ptr< datafileimpl > datafileimpl_;
        boost::scoped_ptr< portfolio::Portfolio > portfolio_;
    };

} // dataproc

#endif // DATAPROCESSOR_H
