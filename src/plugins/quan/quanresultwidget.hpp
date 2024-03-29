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

#ifndef QUANRESULTWIDGET_HPP
#define QUANRESULTWIDGET_HPP

#include <QWidget>
#include <memory>
#include <string>
#include <set>

namespace boost { namespace uuids { struct uuid;  } }

namespace quan {

    class QuanConnection;
    class QuanResultTable;

    class QuanResultWidget : public QWidget
    {
        Q_OBJECT
    public:
        explicit QuanResultWidget(QWidget *parent = 0);
        void setConnection( QuanConnection * );
        void setCompoundSelected( const std::set<boost::uuids::uuid>& );

    private:
        QuanResultTable * table_;
        std::weak_ptr< QuanConnection > connection_;

        void execQuery( const std::string& );
        void CountingIndexChanged( int );
        void handleIndexChanged( int );
        void handleCurrentChanged( const QModelIndex& );
        int currentIndex_;

    signals:
        void onResponseSelected( int );

    public slots:

    };

}

#endif // QUANRESULTWIDGET_HPP
