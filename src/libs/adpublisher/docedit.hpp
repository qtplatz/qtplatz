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

#include <QTextEdit>
#include <memory>
#include "adpublisher_global.hpp"

class QCompleter;
class QContextMenuEvent;

namespace pugi { class xml_document; }

namespace adpublisher {

    class document;

    namespace detail { class text_writer; }

    class ADPUBLISHERSHARED_EXPORT docEdit : public QTextEdit {
        Q_OBJECT
        docEdit( const docEdit& ) = delete;
        friend class detail::text_writer;
    public:
        ~docEdit();
        explicit docEdit(QWidget *parent = 0);

        // std::shared_ptr< adpublisher::document > document();
        void setDocument( std::shared_ptr< adpublisher::document >& );
        
        void setCompleter( QCompleter * );
        QCompleter * completer() const;
        void fetch( pugi::xml_document& xml );

    protected:
        void keyPressEvent( QKeyEvent * );
        void focusInEvent( QFocusEvent * );

    private:
        std::weak_ptr< adpublisher::document > doc_;
        QCompleter * completer_;

        void repaint( const pugi::xml_document& );
        QString textUnderCursor() const;
        void contextMenuEvent( QContextMenuEvent * );

    signals:

    public slots:

    private slots:
        void insertCompletion(const QString &completion);
        void addSummaryTable();
        void handleBlockDeleted();

    };

}
