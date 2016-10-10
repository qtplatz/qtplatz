/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include <QPlainTextEdit>
#include <memory>

class QCompleter;
class QContextMenuEvent;

namespace query {

    namespace detail { class text_writer; }

    class SqlEdit : public QPlainTextEdit {
        Q_OBJECT
        SqlEdit( const SqlEdit& ) = delete;
        friend class detail::text_writer;
    public:
        ~SqlEdit();
        explicit SqlEdit(QWidget *parent = 0);

        void setCompleter( QCompleter * );
        QCompleter * completer() const;

    protected:
        void keyPressEvent( QKeyEvent * );
        void focusInEvent( QFocusEvent * );

    private:
        QCompleter * completer_;

        QString textUnderCursor() const;
        void contextMenuEvent( QContextMenuEvent * );

    signals:

    public slots:

    private slots:
        void insertCompletion(const QString &completion);
        void addSummaryTable();
    };

}
