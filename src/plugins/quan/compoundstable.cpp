/**************************************************************************
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "compoundstable.hpp"
#include "quandocument.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adplot/constants.hpp>
#include <adwidgets/delegatehelper.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <QComboBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <boost/format.hpp>
#include <functional>
#include <sstream>


namespace quan {
    namespace compounds_table {

        enum {
            c_formula
            , c_protocol
            , c_mass
            , c_tR
            , c_isCounting
            , c_isTimeReference // only for chromatography relative retention time
            , c_isLKMSReference // for lock mass
            , c_isISTD
            , c_idISTD
            , c_level_0
            , c_level_1
            , c_level_2
            , c_level_3
            , c_level_4
            , c_level_5
            , c_level_6
            , c_level_7
            , c_level_8
            , c_level_9
            , c_level_10
            , c_level_11
            , c_level_12
            , c_level_13
            , c_level_14
            , c_level_15
            , c_level_16
            , c_level_17
            , c_level_18
            , c_level_19
            , c_level_20
            , c_level_21
            , c_level_22
            , c_level_23
            , c_level_24
            , c_level_25
            , c_level_26
            , c_level_27
            , c_level_28
            , c_level_29
            , c_level_last
            , c_description
            , c_criteria_0
            , c_criteria_1
            , nbrColums
        };

        class CompoundsDelegate : public QStyledItemDelegate {
        
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {

                QStyleOptionViewItem opt(option);
                initStyleOption( &opt, index );

                if ( index.column() == c_formula ) {
                    
                    std::string formula = adcontrols::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                    adwidgets::DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );

                } else if ( index.column() == c_protocol ) {

                    opt.displayAlignment = Qt::AlignCenter | Qt::AlignVCenter;
                    painter->drawText( opt.rect, opt.displayAlignment, index.data().toInt() < 0 ? "*" : index.data().toString() );

                } else if ( index.column() == c_mass ) {

                    opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    QStyledItemDelegate::paint( painter, opt, index );

                } else if ( index.column() == c_tR ) {

                    double data = index.data( Qt::EditRole ).toDouble();
                    std::string text = ( boost::format( "%.3f" ) % data ).str();
                    painter->drawText( opt.rect, Qt::AlignRight | Qt::AlignVCenter, text.c_str() );

                } else if ( c_level_0 <= index.column() && index.column() <= c_level_last ) {

                    double data = index.data( Qt::EditRole ).toDouble();
                    const char * fmt = "%.4lf";
                    if ( data > 1.0e4 || data < 1.0e-4 )
                        fmt = "%.4le";
                    std::string text = ( boost::format( fmt ) % data ).str();
                    painter->drawText( opt.rect, Qt::AlignRight | Qt::AlignVCenter, text.c_str() );

                } else {
                    QStyledItemDelegate::paint( painter, opt, index );
                }
            }

            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
                if ( index.column() == c_protocol ) {
                    if ( auto combo = qobject_cast< QComboBox *>( editor ) ) {
                        int idx = ( combo->currentIndex() - 1 ); // -1 = none, 0, 1, 2, 3
                        model->setData( index, combo->currentText(), Qt::DisplayRole );
                        model->setData( index, idx, Qt::EditRole );
                    }
                } else {
                    QStyledItemDelegate::setModelData( editor, model, index );
                }
                if ( valueChanged_ )
                    valueChanged_( index );
            }

            QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
                if ( index.column() == c_tR || ( c_level_0 <= index.column() && index.column() <= c_level_last ) ) {
                    QLineEdit * edit = new QLineEdit( parent );
                    edit->setText( QString::fromStdString( (boost::format( "%g" ) % index.data().toDouble() ).str() ) );
                    return edit;
                } else if ( index.column() == c_protocol ) {
                    auto combo = new QComboBox( parent );
                    combo->addItem( "*" ); // none
                    for ( int proto = 0; proto < 4; ++proto )
                        combo->addItem( QString::number( proto ) );
                    combo->setCurrentIndex( index.data( Qt::EditRole ).toInt() + 1 );
                    return combo;
                } else {
                    return QStyledItemDelegate::createEditor( parent, option, index );
                }
            }
            
        public:
            void register_handler( std::function< void( const QModelIndex& ) > f ) {
                valueChanged_ = f;
            }
        private:
            std::function< void( const QModelIndex& ) > valueChanged_;
        };

    }
}

using namespace quan;
using namespace quan::compounds_table;

CompoundsTable::CompoundsTable(QWidget *parent) : TableView(parent)
                                                , model_( new QStandardItemModel() )
                                                , levels_( 1 )
{
    setModel( model_ );
    auto delegate = new CompoundsDelegate;
    delegate->register_handler( [=]( const QModelIndex& index ){ handleValueChanged( index ); } );
	setItemDelegate( delegate );
    setSortingEnabled( true );
    // QFont font;
    // setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QTableView::customContextMenuRequested, this, &CompoundsTable::handleContextMenu );
    connect( QuanDocument::instance(), &QuanDocument::onMSLockEnabled, this, [=] ( bool checked ){ setColumnHidden( c_isLKMSReference, !checked ); } );

    setEditTriggers( QAbstractItemView::AllEditTriggers );

	model_->setColumnCount( nbrColums );
    model_->setRowCount( 1 );

    onInitialUpdate();
}

CompoundsTable::~CompoundsTable()
{
    delete model_;
}

void
CompoundsTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

    setHorizontalHeader( new adwidgets::HtmlHeaderView );

    // horizontalHeader()->setResizeMode( QHeaderView::Stretch );

    model.setColumnCount( nbrColums );
    model.setHeaderData( c_formula,  Qt::Horizontal, tr( "formula" ) );
    model.setHeaderData( c_protocol,  Qt::Horizontal, tr( "protocol#" ) );
    model.setHeaderData( c_mass,  Qt::Horizontal, tr( "<i>m/z</i>" ) );

    using namespace adplot::constants;
    model.setHeaderData( c_tR,  Qt::Horizontal, tr( "t<sub>R</sub>(%1)" ).arg( default_chromatogram_time == chromatogram_time_seconds ? "s" : "min" ) );
    model.setHeaderData( c_isCounting,  Qt::Horizontal, tr( "Counting" ) );
    model.setHeaderData( c_isTimeReference,  Qt::Horizontal, tr( "t<sub>R</sub> ref." ) );
    model.setHeaderData( c_isLKMSReference,  Qt::Horizontal, tr( "lock mass" ) );
    model.setHeaderData( c_isISTD,  Qt::Horizontal, tr( "ISTD" ) );
    model.setHeaderData( c_idISTD,  Qt::Horizontal, tr( "ISTD ID" ) );
    model.setHeaderData( c_description, Qt::Horizontal, tr( "memo" ) );
    model.setHeaderData( c_criteria_0, Qt::Horizontal, tr( "Pass/Fail Criteria(1)" ) );
    model.setHeaderData( c_criteria_1, Qt::Horizontal, tr( "Pass/Fail Criteria(2)" ) );

    for ( int col = c_level_0; col < c_level_last; ++col )
        model.setHeaderData( col, Qt::Horizontal, QString( "amounts [%1]" ).arg( col - c_level_0 + 1 ) );

    if ( auto qm = QuanDocument::instance()->getm< adcontrols::QuanMethod >() )
        handleQuanMethod( *qm );
}

void
CompoundsTable::handleValueChanged( const QModelIndex& index )
{
    QStandardItemModel& model = *model_;

    if ( index.column() == c_formula ) {

        std::string formula = index.data( Qt::EditRole ).toString().toStdString();

        adcontrols::ChemicalFormula cformula;
        double exactMass = cformula.getMonoIsotopicMass( formula );
        model_->setData( model_->index( index.row(), c_mass ), exactMass );

        if ( model_->index( index.row(), c_protocol ).data( Qt::EditRole ).isNull() )
            model_->setData( model_->index( index.row(), c_protocol ), -1 );

        // set default values for amounts (avoid zero value in the table)
        for ( int col = c_level_0; col < int(c_level_0 + levels_); ++col ) {
            if ( model_->index( index.row(), col ).data().isNull() || 
                 model_->index( index.row(), col ).data().toDouble() <= 1.0e-30 )
                model_->setData( model_->index( index.row(), col ), double( col - c_level_0 + 1 ) );
        }

        if ( auto cbx = model.itemFromIndex( model.index( index.row(), c_isLKMSReference ) ) ) {
            cbx->setEditable( false );
            cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
            model.setData( model.index( index.row(), c_isLKMSReference ), false, Qt::EditRole );
            model.setData( model.index( index.row(), c_isLKMSReference ), Qt::Unchecked, Qt::CheckStateRole );
        }

        if ( auto cbx = model.itemFromIndex( model.index( index.row(), c_isTimeReference ) ) ) {
            cbx->setEditable( false );
            cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
            model.setData( model.index( index.row(), c_isTimeReference ), false, Qt::EditRole );
            model.setData( model.index( index.row(), c_isTimeReference ), Qt::Unchecked, Qt::CheckStateRole );
        }

        if ( auto cbx = model.itemFromIndex( model.index( index.row(), c_isISTD ) ) ) {
            cbx->setEditable( false );
            cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
            model.setData( model.index( index.row(), c_isISTD ), false, Qt::EditRole );
            model.setData( model.index( index.row(), c_isISTD ), Qt::Unchecked, Qt::CheckStateRole );
        }

        if ( auto cbx = model.itemFromIndex( model.index( index.row(), c_isCounting ) ) ) {
            cbx->setEditable( false );
            cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
            model.setData( model.index( index.row(), c_isCounting ), false, Qt::EditRole );
            model.setData( model.index( index.row(), c_isCounting ), Qt::Unchecked, Qt::CheckStateRole );
        }

    }

    if ( index.row() == model_->rowCount() - 1 ) {
        auto data = model_->index( index.row(), c_formula ).data( Qt::EditRole );
        if ( !data.isNull() && !data.toString().isEmpty() ) {
            model_->insertRow( index.row() + 1 );
            model_->setData( model_->index( index.row() + 1, c_protocol ), -1 );
        }
    }
}

void
CompoundsTable::handleContextMenu( const QPoint& )
{
    QMenu menu;
#if 0
    typedef std::pair< QAction *, std::function< void() > > action_type;

    std::vector< action_type > actions;
    actions.push_back( std::make_pair( menu.addAction( "Enable all" ), [=] (){ enable_all( true ); } ) );
    actions.push_back( std::make_pair( menu.addAction( "Disable all" ), [=] (){ enable_all( false ); } ) );

    if ( QAction * selected = menu.exec( mapToGlobal( pt ) ) ) {
        auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){
                return t.first == selected;
            });
        if ( it != actions.end() )
            (it->second)();
    }
#endif
}

bool
CompoundsTable::getContents( adcontrols::QuanCompounds& c )
{
    QStandardItemModel& model = *model_;

    c.clear();
    for ( int row = 0; row < model.rowCount(); ++row ) {
        adcontrols::QuanCompound a;
        a.setFormula( model.index( row, c_formula ).data().toString().toStdString().c_str() );
        if ( std::string( a.formula() ).empty() )
            continue;
        a.setProtocol( model.index( row, c_protocol ).data().toInt() );
        a.setDescription( model.index( row, c_description ).data().toString().toStdWString().c_str() );
        a.setMass( model.index( row, c_mass ).data().toDouble() );
        a.set_tR( model.index( row, c_tR ).data().toDouble() ); // sec
        a.setIsCounting( model.index( row, c_isCounting ).data().toBool() );
        a.setIsISTD( model.index( row, c_isISTD ).data().toBool() );
        a.setIdISTD( model.index( row, c_idISTD ).data().toInt() );
        a.setIsLKMSRef( model.index( row, c_isLKMSReference ).data().toBool() );
        a.setIsTimeRef( model.index( row, c_isTimeReference ).data().toBool() );

        std::vector< double > amounts;
        for ( int i = c_level_0; i <= c_level_last; ++i ) {
            auto data = model.index( row, i ).data();
            if ( !data.isNull() )
                amounts.push_back( data.toDouble() );
        }
        a.setAmounts( amounts.data(), amounts.size() );
        c << a;
    }
    return false;
}

bool
CompoundsTable::setContents( const adcontrols::QuanCompounds& c )
{
    QStandardItemModel& model = *model_;
    model.setRowCount( int( c.size() ) + 1 ); // add last empty line

    if ( auto lkms = QuanDocument::instance()->getm< adcontrols::MSLockMethod >() ) {
        setColumnHidden( c_isLKMSReference, !lkms->enabled() );
    }

    int row = 0;
    for ( auto& comp: c ) {
        std::string formula = comp.formula();
        model.setData( model.index( row, c_formula ), QString::fromStdString( formula ) );
        model.setData( model.index( row, c_protocol), comp.protocol() );
        model.setData( model.index( row, c_mass ), comp.mass() );
        model.setData( model.index( row, c_tR ), comp.tR() ); // sec
        model.setData( model.index( row, c_description ), QString::fromStdWString( comp.description() ) );
        model.setData( model.index( row, c_idISTD ), comp.idISTD() );

        model.item( row, c_mass )->setEditable( false );

        if ( auto cbx = model.itemFromIndex( model.index( row, c_isCounting ) ) ) {
            cbx->setEditable( false );
            cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
            model.setData( model.index( row, c_isCounting ), comp.isCounting(), Qt::EditRole );
            model.setData( model.index( row, c_isCounting ), comp.isCounting() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

        if ( auto cbx = model.itemFromIndex( model.index( row, c_isLKMSReference ) ) ) {
            cbx->setEditable( false );
            cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
            model.setData( model.index( row, c_isLKMSReference ), comp.isLKMSRef(), Qt::EditRole );
            model.setData( model.index( row, c_isLKMSReference ), comp.isLKMSRef() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

        if ( auto cbx = model.itemFromIndex( model.index( row, c_isTimeReference ) ) ) {
            cbx->setEditable( false );
            cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
            model.setData( model.index( row, c_isTimeReference ), comp.isTimeRef(), Qt::EditRole );
            model.setData( model.index( row, c_isTimeReference ), comp.isTimeRef() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

        if ( auto cbx = model.itemFromIndex( model.index( row, c_isISTD ) ) ) {
            cbx->setEditable( false );
            cbx->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | cbx->flags() );
            model.setData( model.index( row, c_isISTD ), comp.isISTD(), Qt::EditRole );
            model.setData( model.index( row, c_isISTD ), comp.isISTD() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

        model.setData( model.index( row, c_idISTD ), comp.idISTD() );
        const double * amounts = comp.amounts();
        size_t i;
        for ( i = 0; i < comp.levels(); ++i )
            model.setData( model.index( row, int(c_level_0 + i) ), amounts[ i ] );

        ++row;
    }

    return false;
}

void
CompoundsTable::handleQuanMethod( const adcontrols::QuanMethod& qm )
{
    levels_ = qm.levels();

    if ( !qm.isChromatogram() ) {
        setColumnHidden( c_tR, true );
        setColumnHidden( c_isTimeReference, true );
    }
    else {
        setColumnHidden( c_tR, false );
        setColumnHidden( c_isTimeReference, false );
    }

    if ( !qm.isInternalStandard() ) {
        setColumnHidden( c_isISTD, true );
        setColumnHidden( c_idISTD, true );
    }
    else {
        setColumnHidden( c_isISTD, false );
        setColumnHidden( c_idISTD, false );
    }

    for ( int column = c_level_0; column <= c_level_last; ++column ) {
        uint32_t level = column - c_level_0;
        bool hidden = (level >= levels_) ? true : false;
        setColumnHidden( column, hidden );
    }
}

