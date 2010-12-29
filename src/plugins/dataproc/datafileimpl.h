// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

namespace portfolio {
    class Portfolio;
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
        virtual void subscribe( adcontrols::ProcessedDataset& );
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


