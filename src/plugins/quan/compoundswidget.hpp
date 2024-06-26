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

#ifndef COMPOUNDSWIDGET_HPP
#define COMPOUNDSWIDGET_HPP

#include <QWidget>
#include <memory>

class QGridLayout;

namespace boost { namespace filesystem { class path; } }
namespace adcontrols { class QuanCompounds; }

namespace quan {

    class CompoundsTable;

    class CompoundsWidget : public QWidget {
        Q_OBJECT
    public:
        ~CompoundsWidget();
        explicit CompoundsWidget(QWidget *parent = 0);

        void commit(); // comit ui data to document

    private:

        QGridLayout * layout_;
        std::unique_ptr< CompoundsTable > table_;

        void handleDataChanged( int, bool );
        void importCompounds();

    signals:

    public slots:

    };

}

#endif // COMPOUNDSWIDGET_HPP
