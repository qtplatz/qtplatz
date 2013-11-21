/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef DROPTARGETFORM_HPP
#define DROPTARGETFORM_HPP

#include <QWidget>
#include <QUrl>
#include <memory>

class QStandardItemModel;

namespace Ui {
class DropTargetForm;
}

template<class T> class QList;
class QUrl;

namespace batchproc {

    class DropTargetForm : public QWidget {
        Q_OBJECT

    public:
        explicit DropTargetForm(QWidget *parent = 0);
        ~DropTargetForm();

        const std::vector< std::wstring >& dropped_files() const;

    signals:
        void dropped( const QList<QString>& );

    private slots:
        void handleDropFiles( const QList<QUrl>& );

    private:
        Ui::DropTargetForm *ui;
        std::unique_ptr< QStandardItemModel > model_;
        std::vector< std::wstring > dropfiles_;
    };

}

#endif // DROPTARGETFORM_HPP
