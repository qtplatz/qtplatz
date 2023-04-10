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
#include "document.hpp"
#include "constants.hpp"
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
#include <QBoxLayout>
#include <QSettings>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

namespace lipidid {

    class MetIdWidget::impl {
    public:
        impl() : model_(0)
               , dirty_( true ) {}
        QStandardItemModel * model_;
        mutable adcontrols::MetIdMethod method_;
        bool dirty_;

        void handleDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight ) {
            if ( topLeft.column() == 0 ) {
                dirty_ = true;
                auto  model = topLeft.model();
                for ( int row = topLeft.row(); row <= bottomRight.row(); ++row ) {
                    auto adduct = model->index( row, 0 ).data().toString().toStdString();
                    model_->setData( model->index( row, 1 ), compute_mass( adduct ) );
                    if ( auto item = model_->item( row, 0 ) ) {
                        if ( !(item->flags() & Qt::ItemIsUserCheckable ) ) {
                            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                            item->setData( Qt::Checked, Qt::CheckStateRole );
                        }
                    }
                }
                if ( ( topLeft.row() == ( model->rowCount() - 1 ) ) &&
                     ( ! model->index( model->rowCount() - 1, 0 ).data().toString().isEmpty() ) ) {
                    model_->setRowCount( model->rowCount() + 1 );
                }
            }
        }

        double compute_mass( const std::string& adducts ) const {
            using adcontrols::ChemicalFormula;
            auto alist = ChemicalFormula::split( adducts );
            return std::accumulate( alist.begin()
                                    , alist.end()
                                    , 0.0
                                    , [&](const auto& a, const auto& b){
                                        int sign = b.second == '-' ? -1 : 1;
                                        return a + sign * ChemicalFormula().getMonoIsotopicMass( b.first, true );
                                    });
        }
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

        layout->setContentsMargins( {} );
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
    if ( auto form = findChild< adwidgets::TargetingForm * >() ) {
        connect( form, &adwidgets::TargetingForm::triggerProcess, [&] { emit triggered(); } );
    }
}

void
MetIdWidget::onInitialUpdate()
{
    auto ba = document::instance()->settings()->value( QString( Constants::THIS_GROUP ) + "/MetIdMethod" ).toByteArray();
    if ( !ba.isEmpty() ) {
        boost::system::error_code ec;
        auto jv = boost::json::parse( ba.toStdString(), ec );
        if ( !ec ) {
            impl_->method_ = boost::json::value_to< adcontrols::MetIdMethod >( jv.as_object().at( "metIdMethod" ) );
        }
    }

    // auto jv = boost::json::value_from( impl_->method_ );
    // ADDEBUG() << jv;

    using adwidgets::TableView;
    using adwidgets::HtmlHeaderView;
    using adwidgets::TargetingForm;

    if ( auto table = findChild< TableView *>() ) {
        auto model = new QStandardItemModel();
        impl_->model_ = model;
        QSignalBlocker block( impl_->model_ );
        connect( model, &QAbstractItemModel::dataChanged
                 , [&](const QModelIndex &topLeft, const QModelIndex &bottomRight){
                     impl_->handleDataChanged( topLeft, bottomRight );
                 });
        const auto& adducts = impl_->method_.adducts();
        model->setColumnCount( 2 );
        model->setRowCount( adducts.size() + 1 );
        model->setHeaderData( 0, Qt::Horizontal, QObject::tr( "adduct/lose" ) );
        model->setHeaderData( 1, Qt::Horizontal, QObject::tr( "+/- mass" ) );

        table->setModel( model );
        table->setItemDelegate( new delegate() );

        table->setHorizontalHeader( new HtmlHeaderView );
        table->setSortingEnabled( true );

        for ( size_t row = 0; row < adducts.size(); ++row ) {
            model->setData( model->index( row, 0 ), QString::fromStdString( adducts.at( row ).second ) );

            auto adduct = adducts.at( row ).second; // first := enable
            auto mass = impl_->compute_mass( adduct );

            if ( auto item = model->item( row, 0 ) ) {
                model->setData( model->index( row, 0 ), adducts.at( row ).first ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
                model->setData( model->index( row, 1 ), mass );
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            } else {
                ADDEBUG() << "has no item: " << row;
            }
        }
    }

    if ( auto form = findChild< TargetingForm *>() ) {
        form->setContents( impl_->method_ );
    }
    impl_->dirty_ = false;
}

MetIdWidget::value_type
MetIdWidget::getContents() const
{
    adcontrols::MetIdMethod t;
    // unkown dirty condition for TargetingForm
    if ( auto form = findChild< adwidgets::TargetingForm *>() ) {
        form->getContents( t );
    }

    const auto model = impl_->model_;
    t.adducts().clear();
    for ( size_t row = 0; row < model->rowCount(); ++row ) {
        auto adducts = model->data( model->index( row, 0 ), Qt::EditRole ).toString().toStdString();
        auto enable  = model->data( model->index( row, 0 ), Qt::CheckStateRole ).toBool();
        if ( !adducts.empty() ) {
            t << std::make_pair( enable, adducts );
        }
    }

    impl_->method_ = t;
    impl_->dirty_ = false;
    auto json = boost::json::serialize( boost::json::object{{ "metIdMethod", impl_->method_ }} );
    document::instance()->settings()->setValue( QString(Constants::THIS_GROUP) + "/MetIdMethod", QByteArray( json.data(), json.size() ) );

    return impl_->method_;
}

bool
MetIdWidget::setContents( const MetIdWidget::value_type& t )
{
    impl_->method_ = t;
    if ( auto form = findChild< adwidgets::TargetingForm *>() ) {
        form->setContents( impl_->method_ );
    }
    QSignalBlocker block( impl_->model_ );
    const auto& adducts = impl_->method_.adducts();
    auto model = impl_->model_;
    model->setRowCount( adducts.size() );

    for ( size_t row = 0; row < adducts.size(); ++row ) {
        model->setData( model->index( row, 0 ), QString::fromStdString( adducts.at( row ).second ) );

        auto adduct = adducts.at( row ).second; // first := enable
        auto mass = impl_->compute_mass( adduct );

        if ( auto item = model->item( row, 0 ) ) {
            model->setData( model->index( row, 0 ), adducts.at( row ).first ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
            model->setData( model->index( row, 1 ), mass );
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        }
    }
    impl_->dirty_ = false;
    return true;
}
