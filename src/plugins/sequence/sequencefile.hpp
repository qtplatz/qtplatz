// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef SEQUENCEFILE_H
#define SEQUENCEFILE_H

#include <coreplugin/ifile.h>
#include <boost/smart_ptr.hpp>
#include <map>
#include <string>

namespace adsequence { class sequence; }
namespace adcontrols { class ProcessMethod; }
namespace ControlMethod { struct Method; }

namespace sequence {

    class SequenceEditor;

    class SequenceFile : public Core::IFile {
      Q_OBJECT
    public:
      ~SequenceFile();
      explicit SequenceFile( const SequenceEditor&, QObject *parent = 0 );

      bool load( const QString& path );
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

      // 
      adsequence::sequence& adsequence();
      const adsequence::sequence& adsequence() const;

    signals:

    protected slots:
        void modified() { setModified( true ); }

    private:
        const SequenceEditor& editor_;
        const QString mimeType_;
        QString defaultPath_;
        QString filename_;
        bool modified_;
        typedef std::map< std::wstring, boost::shared_ptr< ControlMethod::Method > > control_method_map_type;
        typedef std::map< std::wstring, boost::shared_ptr< adcontrols::ProcessMethod > > process_method_map_type;
        boost::scoped_ptr< adsequence::sequence > adsequence_;
        control_method_map_type ctrlmethods_;
        process_method_map_type procmethods_;
    };

}

#endif // SEQUENCEFILE_H
