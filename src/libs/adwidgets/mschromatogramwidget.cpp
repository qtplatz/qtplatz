/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mschromatogramwidget.hpp"
#include "mschromatogramform.hpp"
#include "moltableview.hpp"
#include "targetingadducts.hpp"
#if HAVE_RDKit
# include <adchem/drawing.hpp>
# include <adchem/mol.hpp>
#endif
#include <adcontrols/processmethod.hpp>
#include <adcontrols/mschromatogrammethod.hpp>
#include <adcontrols/moltable.hpp>
#include <adportable/debug.hpp>
#include <adportable/is_type.hpp>
#include <boost/exception/all.hpp>
#include <QSplitter>
#include <QStandardItemModel>
#include <QBoxLayout>
#include <QMenu>
#include <fstream>

namespace adwidgets {

    enum {
        c_formula
        , c_adducts
        , c_mass
        , c_lockmass
        , c_protocol
        , c_synonym
        , c_memo
        , c_svg
        , c_smiles
        , column_count
    };

    struct MSChromatogramWidget::helper {
        static bool readRow( int row, adcontrols::moltable::value_type&, const QStandardItemModel& model );
        static bool setRow( int row, const adcontrols::moltable::value_type&, QStandardItemModel& model );
    };
}

using namespace adwidgets;

MSChromatogramWidget::MSChromatogramWidget(QWidget *parent) : QWidget(parent)
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin(0);
        layout->setSpacing(2);
        
        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new MSChromatogramForm ) );
            auto table = new MolTableView();
            setup( table );
            splitter->addWidget( table );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 3 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }

    if ( auto form = findChild< MSChromatogramForm * >() ) {

        if ( auto table = findChild< MolTableView *>() )
            connect( form, &MSChromatogramForm::onEnableLockMass
                     , [table]( bool enable ) { table->setColumnHidden( c_lockmass, !enable ); } );

        connect( form, &MSChromatogramForm::triggerProcess, [this] { run(); } );
    }
}

MSChromatogramWidget::~MSChromatogramWidget()
{
}

QWidget *
MSChromatogramWidget::create( QWidget * parent )
{
    return new MSChromatogramWidget( parent );
}

void
MSChromatogramWidget::OnCreate( const adportable::Configuration& )
{
}

void
MSChromatogramWidget::OnInitialUpdate()
{
    if ( auto form = findChild< MSChromatogramForm * >() ) 
        form->OnInitialUpdate();

    if ( auto table = findChild< MolTableView *>() )
        table->onInitialUpdate();
}

void
MSChromatogramWidget::onUpdate( boost::any&& )
{
}

void
MSChromatogramWidget::OnFinalClose()
{
}

bool
MSChromatogramWidget::getContents( boost::any& a ) const
{
    if ( auto pm = boost::any_cast<adcontrols::ProcessMethod *>( a ) ) {
        adcontrols::MSChromatogramMethod m;
        getContents( m );
        (*pm) *= m;
        return true;
    }
    return false;
}

bool
MSChromatogramWidget::setContents( boost::any&& a )
{
    if ( adportable::a_type< adcontrols::ProcessMethod >::is_a( a ) ) {
        
        const adcontrols::ProcessMethod& pm = boost::any_cast<adcontrols::ProcessMethod&>( a );
        
        if ( auto m = pm.find< adcontrols::MSChromatogramMethod >() ) {
            setContents( *m );
            return true;
        }
    }
    return false;
}

void
MSChromatogramWidget::setContents( const adcontrols::MSChromatogramMethod& m )
{
    if ( auto form = findChild< MSChromatogramForm * >() )
        form->setContents( m );

    if ( auto table = findChild< MolTableView * >() )
        table->setColumnHidden( c_lockmass, !m.lockmass() );

    size_t size = m.molecules().data().size();

    if ( size > model_->rowCount() )
        model_->setRowCount( m.molecules().data().size() );
    else
        model_->removeRows( size - 1, model_->rowCount() - size );
    
    int row( 0 );
    for ( auto& mol: m.molecules().data() )
        helper::setRow( row++, mol, *model_ );

    addRow();
}

bool
MSChromatogramWidget::getContents( adcontrols::MSChromatogramMethod& m ) const
{
    if ( auto form = findChild< MSChromatogramForm * >() )
        form->getContents( m );

    m.molecules().data().clear();

    for ( int row = 0; row < model_->rowCount(); ++row ) {
        adcontrols::moltable::value_type value;
        helper::readRow( row, value, *model_ );
        if ( !value.formula().empty() )
            m.molecules() << value;
    }

    return true;
}

void
MSChromatogramWidget::handleContextMenu( QMenu& menu, const QPoint& pt )
{
    menu.addAction( "Run generate chromatograms", this, SLOT( run() ) );
}

void
MSChromatogramWidget::run()
{
    emit triggerProcess( "MSChromatogramWidget" );
}

void
MSChromatogramWidget::setup( MolTableView * table )
{
    model_ = std::make_unique< QStandardItemModel >();
    model_->setColumnCount( column_count );

    model_->setHeaderData( c_formula,  Qt::Horizontal, QObject::tr( "Formula" ) );
    model_->setHeaderData( c_adducts,  Qt::Horizontal, QObject::tr( "adduct/lose" ) );
    model_->setHeaderData( c_mass,     Qt::Horizontal, QObject::tr( "<i>m/z<i>" ) );
    model_->setHeaderData( c_lockmass, Qt::Horizontal, QObject::tr( "lock mass" ) );
    model_->setHeaderData( c_protocol, Qt::Horizontal, QObject::tr( "protocol#" ) );
    model_->setHeaderData( c_synonym,  Qt::Horizontal, QObject::tr( "synonym" ) );
    model_->setHeaderData( c_memo,     Qt::Horizontal, QObject::tr( "memo" ) );
    model_->setHeaderData( c_svg,      Qt::Horizontal, QObject::tr( "structure" ) );
    model_->setHeaderData( c_smiles,   Qt::Horizontal, QObject::tr( "SMILES" ) );

    table->setModel( model_.get() );

    //                                                          editable, checkable
    table->setColumnField( c_formula, ColumnState::f_formula,     true,  true );
    table->setColumnField( c_adducts, ColumnState::f_adducts,     true,  false );
    table->setColumnField( c_mass,    ColumnState::f_mass,        false, false ); 
    table->setColumnField( c_synonym, ColumnState::f_synonym,     false, false );
    table->setColumnField( c_memo,    ColumnState::f_description, false, false );
    table->setColumnField( c_protocol, ColumnState::f_protocol,   true,  false );
    table->setColumnField( c_svg,     ColumnState::f_svg );
    table->setColumnField( c_smiles,  ColumnState::f_smiles );

    table->setContextMenuHandler( [&]( const QPoint& pt ){
            QMenu menu;
            menu.addAction( tr( "Add row" ), this, SLOT( addRow() ) );
            if ( auto table = findChild< MolTableView * >() )
                menu.exec( table->mapToGlobal( pt ) );
        });
    connect( model_.get(), &QAbstractItemModel::dataChanged, this, &MSChromatogramWidget::handleDataChanged );
    addRow();
}

void
MSChromatogramWidget::handleDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
    QSignalBlocker block( model_.get() );
    
    if ( ( topLeft.column() <= c_formula && c_formula <= bottomRight.column() ) ||
         ( topLeft.column() <= c_adducts && c_adducts <= bottomRight.column() ) ) {

        for ( auto row = topLeft.row(); row <= bottomRight.row(); ++row ) {

            double mass = MolTableView::getMonoIsotopicMass( model_->index( row, c_formula ).data( Qt::EditRole ).toString()
                                                             , model_->index( row, c_adducts ).data( Qt::EditRole ).toString() );

            model_->setData( model_->index( row, c_mass ), mass, Qt::EditRole );
        }
    }
    
#if HAVE_RDKit
    if ( topLeft.column() <= c_smiles && c_smiles <= bottomRight.column() ) {

        for ( auto row = topLeft.row(); row <= bottomRight.row(); ++row ) {
            auto smiles = model_->index( row, c_smiles ).data( Qt::EditRole ).toString().toStdString();
            adchem::mol mol( smiles, adchem::mol::SMILES );
            if ( smiles.empty() ) {
                if ( auto item = model_->item( row, c_formula ) )
                    item->setEditable( true );
                model_->setData( model_->index( row, c_svg ), QByteArray() );
            } else {
                std::string svg = adchem::drawing::toSVG( *static_cast< RDKit::ROMol *>(mol) );
                QString formula = QString::fromStdString( mol.formula() );
                double mass = MolTableView::getMonoIsotopicMass( formula, model_->index( row, c_adducts ).data( Qt::EditRole ).toString() );            
                
                model_->setData( model_->index( row, c_svg ), QByteArray( svg.data(), svg.size() ));
                model_->setData( model_->index( row, c_formula ), formula, Qt::EditRole );
                model_->setData( model_->index( row, c_mass ), mass, Qt::EditRole );
                if ( auto item = model_->item( row, c_formula ) )
                    item->setEditable( false );
            }
        }
        
    }
#endif
}

void
MSChromatogramWidget::addRow()
{
    int row = model_->rowCount();
    model_->setRowCount( row + 1 );

    helper::setRow( row, adcontrols::moltable::value_type(), *model_ );
}

bool
MSChromatogramWidget::helper::readRow( int row, adcontrols::moltable::value_type& mol, const QStandardItemModel& model )
{
    mol.formula() = model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdString();
    mol.enable()  = model.index( row, c_formula ).data( Qt::CheckStateRole ).toBool();
    mol.mass()    = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
    mol.abundance() = 1.0;
    mol.adducts() = model.index( row, c_adducts ).data( Qt::EditRole ).toString().toStdString();
    mol.synonym() = model.index( row, c_synonym ).data( Qt::EditRole ).toString().toStdString();
    mol.description() = model.index( row, c_memo ).data( Qt::EditRole ).toString().toStdWString();
    mol.setIsMSRef( model.index( row, c_lockmass ).data( Qt::CheckStateRole ).toBool() );
    mol.smiles()  = model.index( row, c_smiles ).data( Qt::EditRole ).toString().toStdString();
    int protocol = model.index( row, c_protocol ).data( Qt::EditRole ).toInt();
    mol.setProtocol( protocol >= 0 ? boost::optional< int32_t >( protocol ) : boost::none );
}

bool
MSChromatogramWidget::helper::setRow( int row, const adcontrols::moltable::value_type& mol, QStandardItemModel& model )
{
    {
        QSignalBlocker block( &model );
        
        if ( row <= model.rowCount() )
            model.setRowCount( row + 1 );
        
        model.setData( model.index( row, c_formula ), QString::fromStdString( mol.formula() ) );
        if ( auto item = model.item( row, c_formula ) ) {
            item->setEditable( true );
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            model.setData( model.index( row, c_formula ), mol.enable() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }
        
        model.setData( model.index( row, c_adducts ), QString::fromStdString( mol.adducts() ) );
        
        model.setData( model.index( row, c_mass ), mol.mass() );
        
        model.setData( model.index( row, c_lockmass ), mol.isMSRef() );
        if ( auto item = model.item( row, c_lockmass ) ) {
            item->setEditable( false );
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
            model.setData( model.index( row, c_lockmass ), mol.isMSRef() ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
        }

        model.setData( model.index( row, c_protocol ), mol.protocol() ? mol.protocol().get() : -1 );

        model.setData( model.index( row, c_synonym ), QString::fromStdString( mol.synonym() ) );
        model.setData( model.index( row, c_memo ), QString::fromStdWString( mol.description() ) );
    }

    model.setData( model.index( row, c_smiles ), QString::fromStdString( mol.smiles() ) );
    
    return true;
}
