// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#ifndef SEQUENCEFILE_H
#define SEQUENCEFILE_H

#include <coreplugin/idocument.h>
#include <map>
#include <string>
#include <memory>

namespace adsequence { class sequence; }
namespace adcontrols { class ProcessMethod; class ControlMethod; }

namespace sequence {

    class SequenceEditor;

    class SequenceFile : public Core::IDocument {
      Q_OBJECT
    public:
      ~SequenceFile();
      explicit SequenceFile( SequenceEditor&, QObject *parent = 0 );

      bool load( const QString& path );
      void setModified( bool val = true );

      // implement Core::IFile
        // Core::IDocument
        bool save( QString* errorString, const QString& filename = QString(), bool autoSave = false ) override;
        bool reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType ) override;

        QString defaultPath() const override;
        QString suggestedFileName() const override;
        bool isModified() const override;
        bool isSaveAsAllowed() const override;
        bool isFileReadOnly() const override;
        // virtual bool save(const QString &fileName);
        // virtual QString fileName() const;
      
        // virtual QString defaultPath() const;
        // virtual QString suggestedFileName() const;
        // virtual QString mimeType() const;
      
        // virtual bool isModified() const;
        // virtual bool isReadOnly() const;
        // virtual bool isSaveAsAllowed() const;
      
        // virtual void modified(ReloadBehavior *behavior);
        // virtual void checkPermissions() {}
        // 
        adsequence::sequence& adsequence();
        const adsequence::sequence& adsequence() const;

        void fileName( const QString& );

        void removeProcessMethod( const std::wstring& );
        void removeControlMethod( const std::wstring& );
        const adcontrols::ProcessMethod * getProcessMethod( const std::wstring& ) const;
        const adcontrols::ControlMethod * getControlMethod( const std::wstring& ) const;
        void setProcessMethod( const std::wstring&, const adcontrols::ProcessMethod& );
        void setControlMethod( const std::wstring&, const adcontrols::ControlMethod& );

    signals:

    protected slots:
        void modified() { setModified( true ); }

    private:
		SequenceEditor& editor_;
        const QString mimeType_;
        QString defaultPath_;
        QString filename_;
        bool modified_;
        typedef std::map< std::wstring, std::shared_ptr< adcontrols::ControlMethod > > control_method_map_type;
        typedef std::map< std::wstring, std::shared_ptr< adcontrols::ProcessMethod > > process_method_map_type;
        std::unique_ptr< adsequence::sequence > adsequence_;
        control_method_map_type ctrlmethods_;
        process_method_map_type procmethods_;
    };

}

#endif // SEQUENCEFILE_H
