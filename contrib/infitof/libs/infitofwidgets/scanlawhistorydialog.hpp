/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#ifndef SCANLAWHISTORYDIALOG_HPP
#define SCANLAWHISTORYDIALOG_HPP

#include "infitofwidgets_global.hpp"
#include <QDialog>
#include <memory>

class QSqlDatabase;

namespace adcontrols {
    class MSMolTable;
}

namespace infitofwidgets {
    
    namespace Ui {
        class ScanLawHistoryDialog;
    }

    class INFITOFWIDGETSSHARED_EXPORT ScanLawHistoryDialog : public QDialog  {
        Q_OBJECT

    public:
        explicit ScanLawHistoryDialog(QWidget *parent = 0);
        ~ScanLawHistoryDialog();
        bool openDatabase( const QString& file );

        std::shared_ptr< adcontrols::MSMolTable > selectedData();

    signals:

    private:
        Ui::ScanLawHistoryDialog *ui;
        std::unique_ptr< QSqlDatabase > sqldb_;
    };

}

#endif // SCANLAWHISTORYDIALOG_HPP
