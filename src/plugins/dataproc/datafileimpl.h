// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

#include <adcontrols/datasubscriber.h>
#include <adcontrols/datafile.h>
#include <adcontrols/chromatogram.h>

#include <coreplugin/ifile.h>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <vector>

namespace adcontrols {
    class LCMSDataset;
    class Chromatogram;
}

namespace dataproc {

    class datafileimpl : public Core::IFile
                       , public adcontrols::dataSubscriber
                       , boost::noncopyable {
        Q_OBJECT
    public:
        ~datafileimpl();
        explicit datafileimpl( adcontrols::datafile *, QObject *parent = 0);

        void setModified( bool val = true );

        // implement Core::IFile
        virtual bool save(const QString &fileName);
        virtual QString fileName() const;
      
        virtual QString defaultPath() const;
        virtual QString suggestedFileName() const;
        virtual QString mimeType() const;
      
        virtual bool isModified() const;
        virtual bool isReadOnly() const;
        virtual bool isSaveAsAllowed() const;
      
        virtual void modified(ReloadBehavior *behavior);
        virtual void checkPermissions() {}

        // implement adcontrols::dataSubscriber
        virtual void subscribe( adcontrols::LCMSDataset& );
        // <------------------------

        adcontrols::LCMSDataset* getLCMSDataset();
        adcontrols::datafile& file();

    signals:

    public slots:
            void modified() { setModified( true ); }

    private:
        const QString mimeType_;
        const QString filename_;
        bool modified_;
        adcontrols::datafile* file_;
        adcontrols::LCMSDataset* accessor_;
        std::vector< adcontrols::Chromatogram > ticVec_;
    };

} // namespace dataproc


