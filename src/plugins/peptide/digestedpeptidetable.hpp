/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
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

#ifndef DIGESTEDPEPTIDETABLE_HPP
#define DIGESTEDPEPTIDETABLE_HPP

#include <QTableView>
#include <memory>
#include <tuple>

class QStyledItemDelegate;
class QStandardItemModel;

namespace adprot { class protease; class protein; class digestedPeptides; }
namespace adcontrols { class ChemicalFormula; }

namespace peptide {

    class DigestedPeptideTable : public QTableView {
        Q_OBJECT
    public:
        explicit DigestedPeptideTable(QWidget *parent = 0);
        ~DigestedPeptideTable();

        void setData( const std::shared_ptr< adcontrols::ChemicalFormula >& );
        void setData( const adprot::digestedPeptides& );

    private:
        QStandardItemModel * model_;
        void selectionChanged( const QItemSelection&, const QItemSelection& ) override;

        std::weak_ptr< adcontrols::ChemicalFormula > formulaParser_;

        void init( QStandardItemModel& );
        void handleCopyToClipboard();

    signals:
        void selectedFormulae( const QVector< QString >& );

    public slots:

    };

}

#endif // DIGESTEDPEPTIDETABLE_HPP
