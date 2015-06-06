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

#ifndef QUERYQUERYFORM_HPP
#define QUERYQUERYFORM_HPP

#include <QWidget>

namespace query {
    namespace Ui {
        class QueryQueryForm;
    }

    class QueryQueryForm : public QWidget
    {
        Q_OBJECT

    public:
        explicit QueryQueryForm(QWidget *parent = 0);
        ~QueryQueryForm();

        void setSQL( const QString& t);
        QString sql() const;
        //QSize sizeHint() const override { return QSize( 40, 600 ); }

    private slots:
        void on_plainTextEdit_textChanged();
        void on_pushButton_pressed();

        void on_comboBox_currentIndexChanged(int index);

    signals:
        void triggerQuery( const QString& );

    private:
        Ui::QueryQueryForm *ui;
        bool semiColonCaptured_;
        bool eventFilter( QObject *object, QEvent *event );
    };
}

#endif // QUERYQUERYFORM_HPP
