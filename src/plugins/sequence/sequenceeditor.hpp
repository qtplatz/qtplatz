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

#ifndef SEQUENCEEDITOR_H
#define SEQUENCEEDITOR_H

#include <coreplugin/editormanager/ieditor.h>
#include <map>

namespace adsequence { class sequence; }
namespace adcontrols { class ProcessMethod; class ControlMethod; }
namespace ControlMethod { struct Method; }

namespace sequence {

    class SequenceWnd;
    class SequenceFile;

    class SequenceEditor : public Core::IEditor {
        Q_OBJECT
    public:
        ~SequenceEditor();
        explicit SequenceEditor(QObject *parent = 0);
        bool eventFilter( QObject * object, QEvent * event );
        
        // implement Core::IEditor
        bool open( QString*, const QString&, const QString& ) override;
        Core::IDocument * document() override;
        QByteArray saveState() const override;
        bool restoreState( const QByteArray &state ) override;
        QWidget *toolBar() override;
        Core::Context context() const override;
        // <-- end Core::IEditor
        
		void setSequence( const adsequence::sequence& );
        void getSequence( adsequence::sequence& ) const;

		void getDefault( adcontrols::ProcessMethod& ) const;
        void getDefault( adcontrols::ControlMethod& ) const;

		void setModified( bool );

        // interface to SequenceFile

    signals:
        void fileNameChanged( const QString&, const QString& );
        
    public slots:
        void slotTitleChanged( const QString& title );

    private slots:
		void onLineAdded( size_t row );
		void onLineDeleted( size_t prevRow );
        void onCurrentChanged( size_t row, size_t column );
            
    private:
        void saveToObject( size_t row );
        void saveToWidget( size_t row );
        QList<int> context_;
        QString displayName_;  // this will shows on Navigator's 'Open Documents' pane
        SequenceFile * file_;
        SequenceWnd * widget_;
        size_t currRow_;
        size_t currCol_;
    };
    
} // sequence

#endif // SEQUENCEEDITOR_H
