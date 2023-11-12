/**************************************************************************
** Copyright (C) 2022-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2023 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "ionreactionwidget.hpp"
#include "document.hpp"
#include "constants.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/ionreactionmethod.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/is_type.hpp>
#include <adwidgets/create_widget.hpp>
#include <adwidgets/delegatehelper.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <adwidgets/ionreactionform.hpp>
#include <adwidgets/tableview.hpp>
#include <qtwrapper/settings.hpp>
#include <boost/json.hpp>
#include <QAbstractButton>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QSettings>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>

namespace lipidid {

    class IonReactionWidget::impl {
    public:
        impl() : models_{ 0 }, dirty_( true ) {}
        std::array< QStandardItemModel *, 2 > models_;
        mutable adcontrols::IonReactionMethod method_;
        bool dirty_;

        void handleDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight ) {
            if ( topLeft.column() == 0 ) {
                dirty_ = true;
                auto  model = qobject_cast< QStandardItemModel *>( const_cast< QAbstractItemModel *>( topLeft.model() ) );
                for ( int row = topLeft.row(); row <= bottomRight.row(); ++row ) {
                    auto adduct = model->index( row, 0 ).data().toString().toStdString();
                    model->setData( model->index( row, 1 ), compute_mass( adduct ) );
                    if ( auto item = model->item( row, 0 ) ) {
                        if ( !(item->flags() & Qt::ItemIsUserCheckable ) ) {
                            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                            item->setData( Qt::Checked, Qt::CheckStateRole );
                        }
                    }
                }
                if ( ( topLeft.row() == ( model->rowCount() - 1 ) ) &&
                     ( ! model->index( model->rowCount() - 1, 0 ).data().toString().isEmpty() ) ) {
                    model->setRowCount( model->rowCount() + 1 );
                }
            }
        }

        double compute_mass( const std::string& adducts ) const {
            using adcontrols::ChemicalFormula;
            auto alist = ChemicalFormula::split( adducts ); // -[H]+ --> {[H]+, -}
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

using lipidid::IonReactionWidget;

IonReactionWidget::~IonReactionWidget()
{
}

IonReactionWidget::IonReactionWidget( QWidget * parent ) : QWidget( parent )
                                                         , impl_( std::make_unique< IonReactionWidget::impl >() )
{
    // ------------------------------
    // positive mode | negative mode
    // ------------------------------
    // i8n | apply button
    // -----------

    using adwidgets::create_widget;
    using adwidgets::add_widget;
    using adcontrols::polarity_positive;
    using adcontrols::polarity_negative;

    if ( QVBoxLayout * vLayout = new QVBoxLayout( this ) ) {
        vLayout->setContentsMargins( {} );
        vLayout->setSpacing(4);

        if ( auto hLayout = new QHBoxLayout() ) {
            hLayout->setContentsMargins( {} );
            vLayout->addLayout( hLayout );
            for ( auto pol: { polarity_positive, polarity_negative } ) {
                auto title = pol == polarity_positive ? "Positive ion" : "Negative ion";
                auto objname = ( pol == polarity_positive ? "POS" : "NEG" );
                if ( auto gbox = new QGroupBox( title ) ) {
                    if ( auto layout = new QVBoxLayout ) {
                        layout->setContentsMargins( 2, 2, 2, 2 );
                        layout->setSpacing( 2 );
                        if ( QSplitter * splitter = new QSplitter ) {
                            auto form = create_widget< adwidgets::IonReactionForm >( objname );
                            form->setPolarity( pol, false );
                            splitter->addWidget( form );
                            splitter->addWidget( create_widget< adwidgets::TableView >( objname ) );
                            splitter->setStretchFactor( 0, 1 );
                            splitter->setStretchFactor( 1, 4 );
                            splitter->setOrientation ( Qt::Horizontal );
                            layout->addWidget( splitter );
                        }
                        gbox->setLayout( layout );
                    }
                    hLayout->addWidget( gbox );
                }
            }
        }
        if ( auto layout = new QHBoxLayout() ) {
            vLayout->addLayout( layout );
            layout->setContentsMargins( 20, 0, 20, 0 );
            if ( auto label = add_widget( layout, create_widget< QLabel >( "li8n", "Ionization:" ) ) ) {
            }
            if ( auto edit = add_widget( layout, create_widget< QLineEdit >( "i8n", "MVCI" ) ) ) {
                edit->setMaximumWidth( 100 );
            }
            layout->addSpacing( 10 );
            if ( auto edit = add_widget( layout, create_widget< QLineEdit >( "description", "description" ), 2 ) ) {
            }
            if ( auto edit = add_widget( layout, create_widget< QLineEdit >( "dbfile" ), 4 ) ) {
                // edit->setMaximumWidth( 400 );
                edit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
                setSQLiteFilename( qtwrapper::settings( *document::instance()->settings() ).recentFile( "LIPID_MAPS", "Files" ) );
                edit->setReadOnly( true );
            }
            layout->addSpacerItem( new QSpacerItem( 20, 1, QSizePolicy::Expanding ) );

            if ( auto buttonBox = add_widget( layout, create_widget< QDialogButtonBox >( "buttonBox" ) ) ) {
                buttonBox->addButton("Apply", QDialogButtonBox::AcceptRole);
                buttonBox->addButton("Testing", QDialogButtonBox::RejectRole);
                connect( buttonBox, &QDialogButtonBox::accepted, [this]() { emit triggered(); } );
                connect( buttonBox, &QDialogButtonBox::rejected, [this]() { emit rejected(); } );
            }
        }
    }
}

void
IonReactionWidget::setSQLiteFilename( const QString& name )
{
    if ( auto edit = findChild< QLineEdit * >( "dbfile" ) )
        edit->setText( name );
}

void
IonReactionWidget::onInitialUpdate()
{
    auto ba = document::instance()->settings()->value( QString( Constants::THIS_GROUP ) + "/IonReactionMethod" ).toByteArray();
    if ( !ba.isEmpty() ) {
        boost::system::error_code ec;
        auto jv = boost::json::parse( ba.toStdString(), ec );
        if ( !ec ) {
            impl_->method_ = boost::json::value_to< adcontrols::IonReactionMethod >( jv.as_object().at( "IonReactionMethod" ) );
        }
    }

    // auto jv = boost::json::value_from( impl_->method_ );
    // ADDEBUG() << jv;

    using adwidgets::TableView;
    using adwidgets::HtmlHeaderView;
    using adwidgets::IonReactionForm;
    using adcontrols::polarity_positive;
    using adcontrols::polarity_negative;

    for ( auto pol: { polarity_positive, polarity_negative } ) {
        const auto objname = pol == polarity_positive ? "POS" : "NEG";

        if ( auto table = findChild< TableView *>( objname ) ) {
            auto model = new QStandardItemModel();
            impl_->models_[ pol ] = model;
            QSignalBlocker block( model );
            connect( model, &QAbstractItemModel::dataChanged
                     , [&](const QModelIndex &topLeft, const QModelIndex &bottomRight){
                         impl_->handleDataChanged( topLeft, bottomRight );
                     });
            const auto& adducts = impl_->method_.addlose( pol );
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
        if ( auto form = findChild< IonReactionForm *>( objname ) ) {
            form->setContents( impl_->method_ );
        }
    }

    impl_->dirty_ = false;
}

adcontrols::IonReactionMethod
IonReactionWidget::getContents() const
{
    adcontrols::IonReactionMethod t;
    if ( auto form = findChild< adwidgets::IonReactionForm *>( "POS" ) ) {
        form->getContents( t );
    }
    if ( auto form = findChild< adwidgets::IonReactionForm *>( "NEG" ) ) {
        form->getContents( t );
    }
    if ( auto edit = findChild< QLineEdit * >( "i8n" ) ) {
        t.set_i8n( edit->text().toStdString() );
    }
    if ( auto edit = findChild< QLineEdit * >( "description" ) ) {
        t.set_description( edit->text().toStdString() );
    }

    for ( auto pol: { adcontrols::polarity_positive, adcontrols::polarity_negative } ) {
        const auto model = impl_->models_[ pol ];
        t.addlose( pol ).clear();
        for ( size_t row = 0; row < model->rowCount(); ++row ) {
            auto adducts = model->data( model->index( row, 0 ), Qt::EditRole ).toString().toStdString();
            auto enable  = model->data( model->index( row, 0 ), Qt::CheckStateRole ).toBool();
            if ( !adducts.empty() ) {
                t.addlose( pol ).emplace_back( enable, adducts );
            }
        }
    }
    impl_->method_ = t;
    impl_->dirty_ = false;
    auto json = boost::json::serialize( boost::json::object{{ "IonReactionMethod", boost::json::value_from( impl_->method_ ) }} );
    document::instance()->settings()->setValue( QString(Constants::THIS_GROUP) + "/IonReactionMethod", QByteArray( json.data(), json.size() ) );

    return impl_->method_;
}

bool
IonReactionWidget::setContents( const adcontrols::IonReactionMethod& t )
{
    impl_->method_ = t;
    if ( auto form = findChild< adwidgets::IonReactionForm *>( "POS" ) ) {
        form->setContents( impl_->method_ );
    }
    if ( auto form = findChild< adwidgets::IonReactionForm *>( "NEG" ) ) {
        form->setContents( impl_->method_ );
    }
    for ( auto pol: { adcontrols::polarity_positive, adcontrols::polarity_negative } ) {
        QSignalBlocker block( impl_->models_[ pol ] );
        const auto& adducts = impl_->method_.addlose( pol );
        auto model = impl_->models_[ pol ];

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
    }
    impl_->dirty_ = false;
    return true;
}
