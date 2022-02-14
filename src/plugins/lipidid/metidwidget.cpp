/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "metidwidget.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/metidmethod.hpp>
#include <adwidgets/delegatehelper.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <adwidgets/targetingform.hpp>
#include <adwidgets/moltable.hpp>
#include <adportable/is_type.hpp>
#include <adprot/digestedpeptides.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <boost/json.hpp>
#include <QSplitter>
#include <QBoxLayout>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

namespace lipidid {

    class MetIdWidget::impl {
    public:
        impl() {}
        adcontrols::MetIdMethod method_;
    };

}

namespace {
    class delegate : public QStyledItemDelegate {
        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            QStyleOptionViewItem opt(option);
            initStyleOption( &opt, index );
            opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            if ( index.column() == 0 ) {
                using adcontrols::ChemicalFormula;
                std::string formula = ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                adwidgets::DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );
            } else {
                QStyledItemDelegate::paint( painter, opt, index );
            }
        }
    };
}

using lipidid::MetIdWidget;

MetIdWidget::~MetIdWidget()
{
}

MetIdWidget::MetIdWidget( QWidget * parent ) : QWidget( parent )
                                             , impl_( std::make_unique< MetIdWidget::impl >() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( new adwidgets::TargetingForm );
            splitter->addWidget( new adwidgets::TableView );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 2 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
    // if ( auto widget = findChild< TargetingAdducts * >() )
    //     connect( widget, &TargetingAdducts::resetAdducts, this, &TargetingWidget::handleResetAdducts );
    // connect( form_, &TargetingForm::triggerProcess, [this] { emit triggerProcess( "TargetingWidget" ); } );
}

void
MetIdWidget::onInitialUpdate()
{
    auto jv = boost::json::value_from( impl_->method_ );
    ADDEBUG() << jv;

    using adwidgets::TableView;
    using adwidgets::HtmlHeaderView;
    using adwidgets::TargetingForm;

    if ( auto table = findChild< TableView *>() ) {
        auto model = new QStandardItemModel();
        const auto& adducts = impl_->method_.adducts();
        model->setColumnCount( 1 );
        model->setRowCount( adducts.size() );
        model->setHeaderData( 0, Qt::Horizontal, QObject::tr( "adduct/lose" ) );

        table->setModel( model );
        table->setItemDelegate( new delegate() );

        table->setHorizontalHeader( new HtmlHeaderView );
        table->setSortingEnabled( true );
        for ( size_t row = 0; row < adducts.size(); ++row ) {
            model->setData( model->index( row, 0 ), QString::fromStdString( adducts.at( row ).second ) );
            if ( auto item = model->item( row, 0 ) ) {
                ADDEBUG() << "has item: " << row;
                model->setData( model->index( row, 0 ), adducts.at( row ).first ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            } else {
                ADDEBUG() << "has no item: " << row;
            }
        }

        // table->onInitialUpdate();
        // table->setContents( m.molecules() );
        // table->setColumnHidden( MolTable::c_abundance, true );
    }
    if ( auto form = findChild< TargetingForm *>() ) {
        form->setContents( impl_->method_ );
    }
}
