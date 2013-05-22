// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once

#include <adcontrols/datasubscriber.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/chromatogram.hpp>

#include <coreplugin/ifile.h>
#include <boost/noncopyable.hpp>
#include <boost/smart_ptr.hpp>
#include <vector>

namespace adcontrols {
    class LCMSDataset;
    class Chromatogram;
}

namespace portfolio {
    class Portfolio;
}

namespace dataproc {

    class Dataprocessor;

    class IFileImpl : public Core::IFile
                       , public adcontrols::dataSubscriber
                       , boost::noncopyable {
        Q_OBJECT
    public:
        ~IFileImpl();
        explicit IFileImpl( adcontrols::datafile *, Dataprocessor&, QObject *parent = 0);

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
        virtual bool subscribe( const adcontrols::LCMSDataset& );
        virtual bool subscribe( const adcontrols::ProcessedDataset& );
        // <------------------------

        const adcontrols::LCMSDataset* getLCMSDataset();
        adcontrols::datafile& file();

    signals:

    public slots:
        void modified() { setModified( true ); }

    private:
        const QString mimeType_;
        QString filename_;
        bool modified_;
        adcontrols::datafile* file_;
        const adcontrols::LCMSDataset* accessor_;
        std::vector< adcontrols::Chromatogram > ticVec_;
        Dataprocessor& dprocessor_;
    };

} // namespace dataproc


