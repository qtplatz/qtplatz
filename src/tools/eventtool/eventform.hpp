/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef EVENTFORM_HPP
#define EVENTFORM_HPP

#include <QWidget>
#include <chrono>

namespace Ui {
class EventForm;
}

namespace eventtool {

    class EventForm : public QWidget {
        Q_OBJECT

    public:
        explicit EventForm( QWidget *parent = 0 );
        ~EventForm();

    public slots:
        void handle_timeout();

     private slots:
        void on_pushButton_clicked();

        void on_checkBox_clicked(bool checked);

    private:
        Ui::EventForm *ui;
        QString host_;
        QString port_;
        short recvPort_;
        std::chrono::steady_clock::time_point last_inject_;
    };
}

#endif // EVENTFORM_HPP
