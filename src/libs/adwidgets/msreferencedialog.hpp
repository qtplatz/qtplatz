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

#ifndef MSREFERENCEDIALOG_HPP
#define MSREFERENCEDIALOG_HPP

#include <QDialog>
#include <functional>

namespace Ui {
class MSReferenceDialog;
}

namespace adcontrols { class MSReference;  }

namespace adwidgets {

    class MSReferenceDialog : public QDialog {
        Q_OBJECT

    public:
        explicit MSReferenceDialog(QWidget *parent = 0);
        ~MSReferenceDialog();

        void register_handler( std::function< void( const adcontrols::MSReference& ) > );

    private:
        Ui::MSReferenceDialog *ui;

        void handleIndexChanged( int index );
        void handleAddReference();

        std::function< void( const adcontrols::MSReference& ) > reference_receiver_;
    };

}

#endif // MSREFERENCEDIALOG_HPP
