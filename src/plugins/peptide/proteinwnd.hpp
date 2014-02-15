/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef PROTEINWND_HPP
#define PROTEINWND_HPP

#include <QWidget>
#include <memory>

namespace adprot { class protfile; class digestedPeptides; class peptides; }
namespace adcontrols { class MassSpectrum; }
namespace adwplot { class SpectrumWidget; }

namespace peptide {

    class MainWindow;
    class DigestedPeptideTable;
    class ProteinTable;

    class ProteinWnd : public QWidget {
        Q_OBJECT
    public:
        explicit ProteinWnd(QWidget *parent = 0);

        void setData( const adprot::protfile& );

        enum { idSequence, idStdFormula, idNeutralMass };
        typedef std::tuple< std::string, std::string, double > peptide_formula_mass_type;
        
    private:

        std::shared_ptr< adcontrols::MassSpectrum > spectrum_;
        adwplot::SpectrumWidget * spectrumWidget_;
        DigestedPeptideTable * peptideTable_;
        ProteinTable * proteinTable_;

        void init();
        void sort_and_unique( adprot::peptides& );
        void setData( const adprot::peptides& );
    
    signals:
    
    public slots:

    private slots:
        void protSelChanged( int row );
        void handleSelectionChanged( const QVector< int >& );
        void handleFormulaeSelected( const QVector< QString >& );
    };

}

#endif // PROTEINWND_HPP
