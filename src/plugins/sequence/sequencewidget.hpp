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

#ifndef SEQUENCEWIDGET_HPP
#define SEQUENCEWIDGET_HPP

#include <QWidget>
#include <boost/smart_ptr.hpp>

namespace adsequence { class sequence; }

namespace Ui {
class SequenceWidget;
}

class QStandardItemModel;

namespace sequence {

    class SequenceDelegate;

    class SequenceWidget : public QWidget {
        Q_OBJECT
    public:
        explicit SequenceWidget(QWidget *parent = 0);
        ~SequenceWidget();

        void OnInitialUpdate();
        void OnFinalClose();
    
    private:
        Ui::SequenceWidget *ui;
        boost::scoped_ptr< QStandardItemModel > model_;
        boost::scoped_ptr< SequenceDelegate > delegate_;
        boost::scoped_ptr< adsequence::sequence > sequence_;

    private slots:
        void showContextMenu( const QPoint& );
        void addLine();
        void delLine();
    };
}

#endif // SEQUENCEWIDGET_HPP
