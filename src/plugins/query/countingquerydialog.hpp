/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef COUNTINGQUERYDIALOG_HPP
#define COUNTINGQUERYDIALOG_HPP

#include <QDialog>

class QTableView;
class QAbstractItemModel;

namespace query {
    
    namespace Ui {
        class CountingQueryDialog;
    }

    class CountingQueryDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit CountingQueryDialog(QWidget *parent = 0);
        ~CountingQueryDialog();

        QTableView * tableView();
        QAbstractItemModel * model();
        void setCommandText( const QString& );
        QString commandText() const;

        void clear();

    public slots:
        void accept() override;

    signals:
        void applied();

    private:
        Ui::CountingQueryDialog *ui;
    };
}

#endif // COUNTINGQUERYDIALOG_HPP
