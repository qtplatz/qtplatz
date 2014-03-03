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

#ifndef SEQUENCEWIDGET_HPP
#define SEQUENCEWIDGET_HPP

#include <QWidget>
#include <map>
#include <memory>

namespace adsequence { class sequence; class schema; }

namespace Ui {
class SequenceWidget;
}

class QStandardItemModel;
class QModelIndex;

namespace adsequence { class schema; }
namespace adcontrols { class ProcessMethod; }
namespace ControlMethod { struct Method; }

namespace sequence {

    class SequenceDelegate;
    class SequenceEditor;

    class SequenceWnd : public QWidget {
        Q_OBJECT
    public:
        explicit SequenceWnd( const adsequence::schema&, QWidget *parent = 0 );
        ~SequenceWnd();

        void OnInitialUpdate( const adsequence::schema& );
        void OnFinalClose();
        void setSequenceName( const QString& );
        void setDataSaveIn( const QString& );

        void getSequence( adsequence::sequence& ) const;
        void setSequence( const adsequence::sequence& );
        QString getControlMethodName( size_t row ) const;
        QString getProcessMethodName( size_t row ) const;

    private:
        Ui::SequenceWidget *ui;
        std::unique_ptr< QStandardItemModel > model_;
        std::unique_ptr< SequenceDelegate > delegate_;
        std::unique_ptr< adsequence::schema > schema_;
    private slots:
        void handleCurrentChanged( const QModelIndex&, const QModelIndex& );
        void showContextMenu( const QPoint& );
        void addLine();
        void delLine();
        void browse();
        void saveAs();

    signals:
        void currentChanged( size_t row, size_t col );
		void lineAdded( size_t row );
		void lineDeleted( size_t row );
    };
}

#endif // SEQUENCEWIDGET_HPP
