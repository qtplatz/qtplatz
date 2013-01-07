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

#ifndef SEQUENCEEDITOR_H
#define SEQUENCEEDITOR_H

#include <coreplugin/editormanager/ieditor.h>
#include <boost/smart_ptr.hpp>

namespace sequence {
  namespace internal {

    class Sequence;

    class SequenceEditor : public Core::IEditor {
      Q_OBJECT
    public:
      explicit SequenceEditor(QObject *parent = 0);

      // implement Core::IEditor
      virtual bool createNew(const QString &contents = QString());
      virtual bool open(const QString &fileName = QString());
      virtual Core::IFile *file();
      virtual const char *kind() const;
      virtual QString displayName() const;
      virtual void setDisplayName(const QString &title);
      virtual bool duplicateSupported() const;
      virtual IEditor *duplicate(QWidget *parent);
      virtual QByteArray saveState() const;
      virtual bool restoreState(const QByteArray &state);
      virtual int currentLine() const { return 0; }
      virtual int currentColumn() const { return 0; }
      virtual bool isTemporary() const;
      virtual QWidget *toolBar();
      // <-- end Core::IEditor

      // implement IContext
      virtual QList<int> context() const { return context_; }
      virtual QWidget * widget() { return widget_; }
      // <--

    signals:

    public slots:
      void slotTitleChanged( const QString& title );

    private:
      QList<int> context_;
      boost::shared_ptr< Sequence > sequence_;
      QString displayName_;
      QWidget * widget_;
    };

  } // internal
} // sequence

#endif // SEQUENCEEDITOR_H
