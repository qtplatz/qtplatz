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

#include "moltable.hpp"
#include "delegatehelper.hpp"
#include <adprot/digestedpeptides.hpp>
#include <adprot/peptides.hpp>
#include <adprot/peptide.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isotopecluster.hpp>
#include <adcontrols/moltable.hpp>
#include <adcontrols/molecule.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adportable/float.hpp>
#include <adportable/debug.hpp>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QSvgRenderer>
#include <QUrl>
#include <sstream>

#if defined HAVE_RDKit && HAVE_RDKit
#if defined _MSC_VER
# pragma warning( disable: 4267 4018 )
#endif
#include <adchem/drawing.hpp>
#include <RDGeneral/Invariant.h>
#include <GraphMol/Depictor/RDDepictor.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Substruct/SubstructMatch.h>
#include <GraphMol/FileParsers/FileParsers.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <RDGeneral/RDLog.h>
#endif

#include <boost/format.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <functional>

using namespace adwidgets;

namespace adwidgets {

    namespace ac = adcontrols;

    static double computeMass( const QString& formula, const QString& adducts, QString& stdFormula )
    {
        std::string stdformula = formula.toStdString();
        std::string stdadducts = adducts.toStdString();
        auto v = adcontrols::ChemicalFormula::standardFormulae( stdformula, stdadducts );
        if ( v.empty() )
            return 0;

        stdFormula = std::accumulate( v.begin(), v.end(), QString(), []( const QString& a, const std::string& b ){
                return a.isEmpty() ? QString::fromStdString( b ) : a + "\n" + QString::fromStdString( b );
            });
        ///////////////////////
        // ADDEBUG() << "computeMass(" << v[0] << " <- " << stdformula << ")=" << adcontrols::ChemicalFormula().getMonoIsotopicMass( v[0] );
        ///////////////////////
        return adcontrols::ChemicalFormula().getMonoIsotopicMass( v[0] ); // handle first molecule
    }

    static QVector<double> computeMasses( const QString& formula, const QString& adducts, QString& stdFormula )
    {
        QVector<double> masses;
        std::string stdformula = formula.toStdString();
        std::string stdadducts = adducts.toStdString();
        auto v = adcontrols::ChemicalFormula::standardFormulae( stdformula, stdadducts );

        std::for_each( v.begin(), v.end(), [&]( const std::string& f ){ masses << adcontrols::ChemicalFormula().getMonoIsotopicMass( f ); } );

        stdFormula = std::accumulate( v.begin(), v.end(), QString(), []( const QString& a, const std::string& b ){
                return a.isEmpty() ? QString::fromStdString( b ) : a + "\n" + QString::fromStdString( b );
            });

        return masses;
    }

    /////////////////

    class MolTable::delegate : public QStyledItemDelegate {
        
        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                
            QStyleOptionViewItem opt(option);
            initStyleOption( &opt, index );
            opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;

            if ( index.column() == MolTable::c_formula ) {
                
                std::string formula = ac::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                DelegateHelper::render_html2( painter, opt, QString::fromStdString( formula ) );

            } else if ( index.column() == MolTable::c_abundance ) {
                
                if ( index.data().toDouble() <= 0.002 ) {
                    painter->save();
                    painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x80 ) ); // tomato
                    QStyledItemDelegate::paint( painter, opt, index );
                    painter->restore();
                }
                QStyledItemDelegate::paint( painter, opt, index );

            } else if ( index.column() == MolTable::c_mass ) {
                    
                painter->save();
                QString stdFormulae;
                double exactMass = computeMass(
                    index.model()->index( index.row(), MolTable::c_formula ).data( Qt::EditRole ).toString()
                    , index.model()->index( index.row(), MolTable::c_adducts ).data( Qt::EditRole ).toString(), stdFormulae );

                double mass = index.data( Qt::EditRole ).toDouble();
                if ( adportable::compare<double>::approximatelyEqual( exactMass, mass ) )
                    painter->fillRect( option.rect, QColor( 0xf0, 0xf8, 0xff, 0x80 ) ); // AliceBlue                    
                else if ( mass < 0.9 )
                    painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x80 ) ); // tomato
                else 
                    painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x40 ) ); // tomato

                painter->drawText( option.rect
                                   , option.displayAlignment
                                   , QString::number( index.data( Qt::EditRole ).toDouble(), 'f', 8  ) );
                // QStyledItemDelegate::paint( painter, opt, index );
                painter->restore();

            } else if ( index.column() == MolTable::c_svg ) {
                painter->save();
                    
                QSvgRenderer renderer( index.data().toByteArray() );

                painter->translate( option.rect.x(), option.rect.y() );
                //QRectF viewport = painter->viewport();
                painter->scale( 1.0, 1.0 );

                QRect target( 0, 0, option.rect.width(), option.rect.height() );
                renderer.render( painter, target );

                painter->restore();
                    
            } else {

                QStyledItemDelegate::paint( painter, opt, index );

            }
        }

        void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
            QStyledItemDelegate::setModelData( editor, model, index );
            if ( valueChanged_ )
                valueChanged_( index );
        }

        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {
            if ( index.column() == MolTable::c_mass ) {
                auto widget = new QDoubleSpinBox( parent );
                widget->setMinimum( 0 ); widget->setMaximum( 5000 ); widget->setSingleStep( 0.0001 ); widget->setDecimals( 7 );
                widget->setValue( index.data( Qt::EditRole ).toDouble() );
                return widget;
            } else {
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
        }
        QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            if ( index.column() == MolTable::c_svg && !index.data( Qt::EditRole ).toByteArray().isEmpty() ) {
                return QSize( 80, 80 );
            } else if ( index.column() == MolTable::c_formula ) {
                return DelegateHelper::html_size_hint( option, index );
            } else {
                return QStyledItemDelegate::sizeHint( option, index );
            }
        }
    public:
        delegate( std::function< void( const QModelIndex& ) > f ) : valueChanged_( f ) {
        }
    private:
        std::function< void( const QModelIndex& ) > valueChanged_;
    };

    class MolTable::impl  {
    public:
        impl() {
            std::fill( editable_.begin(), editable_.end(), false );
            for ( auto& col : { c_formula, c_adducts, c_abundance, c_synonym, c_description, c_smiles } )
                editable_[ col ] = true;
        }

        ~impl() {
        }

        std::array< bool, nbrColums > editable_;

        void  setData( MolTable& table
                       , int row
                       , const QString& formula
                       , const QString& adducts
                       , const QString& smiles
                       , const QByteArray& svg
                       , const QString& synonym = QString()
                       , const QString& description = QString()
                       , double mass = 0.0, double abundance = 1.0, bool enable = true, bool msref = false ) {
            
            auto& model = table.model();

            model.setData( model.index( row, c_svg ), svg );

            if ( svg.isEmpty() && !smiles.isEmpty() ) {
#if HAVE_RDKit
                if ( auto mol = std::unique_ptr< RDKit::ROMol >( RDKit::SmilesToMol( smiles.toStdString(), 0, false ) ) ) {
                    mol->updatePropertyCache( false );
                    auto xsvg = adchem::drawing::toSVG( *mol );
                    model.setData( model.index( row, c_svg ), QByteArray( xsvg.data(), int( xsvg.size() ) ) );
                }
#endif                
            }

            QString stdFormula;
            if ( mass < 0.9 && !formula.isEmpty() )
                mass = computeMass( formula, adducts, stdFormula );
            
            model.setData( model.index( row, c_smiles ), smiles );
            model.setData( model.index( row, c_formula ), formula );
            model.setData( model.index( row, c_adducts ), adducts );
            model.setData( model.index( row, c_synonym ), synonym );
            model.setData( model.index( row, c_abundance ), abundance );
            model.setData( model.index( row, c_mass ), mass );
            model.setData( model.index( row, c_msref ), msref );
            model.setData( model.index( row, c_description ), description );

            if ( auto item = model.item( row, c_formula ) ) {
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                model.setData( model.index( row, c_formula ), enable ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
            }
            
            if ( auto item = model.item( row, c_msref ) ) {
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
                model.setData( model.index( row, c_msref ), msref ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
            }

            if ( !smiles.isEmpty() ) {
                if ( auto item = model.item( row, c_svg ) ) // has structure data
                    item->setEditable( false );
                if ( auto item = model.item( row, c_formula ) )
                    item->setEditable( false );
            }
            
            if ( auto item = model.item( row, c_mass ) )
                item->setEditable( editable_[ c_mass ] );
        }
    };
}

MolTable::MolTable(QWidget *parent) : TableView(parent)
                                    , impl_( new impl() )
                                    , model_( new QStandardItemModel() )
{
    setModel( model_ );
	setItemDelegate( new delegate( [this]( const QModelIndex& index ){
                if ( ! signalsBlocked() )
                    handleValueChanged( index );
            } ) );
    setSortingEnabled( true );
    setAcceptDrops( true );

    connect( this, &TableView::rowsDeleted, [this]() {
            if ( model_->rowCount() == 0 )
                model_->setRowCount( 1 );                
        });

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &QTableView::customContextMenuRequested, this, &MolTable::handleContextMenu );

    model_->setColumnCount( nbrColums );
    model_->setRowCount( 1 );
    //setColumnHidden( c_smiles, true );
}

MolTable::~MolTable()
{
    delete model_;
}

QStandardItemModel&
MolTable::model()
{
    return *model_;
}

void
MolTable::onInitialUpdate()
{
    QStandardItemModel& model = *model_;

    horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Interactive );
    horizontalHeader()->setStretchLastSection( true );

    model.setColumnCount( nbrColums );
    model.setHeaderData( c_formula, Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_adducts, Qt::Horizontal, QObject::tr( "adduct/lose" ) );
    model.setHeaderData( c_mass, Qt::Horizontal, QObject::tr( "mass" ) );
    model.setHeaderData( c_msref, Qt::Horizontal, QObject::tr( "lock mass" ) );    
    model.setHeaderData( c_abundance, Qt::Horizontal, QObject::tr( "R. A. " ) );
    model.setHeaderData( c_synonym, Qt::Horizontal, QObject::tr( "synonym" ) );
    model.setHeaderData( c_svg, Qt::Horizontal, QObject::tr( "structure" ) );
    model.setHeaderData( c_smiles, Qt::Horizontal, QObject::tr( "SMILES" ) );    
    model.setHeaderData( c_description, Qt::Horizontal, QObject::tr( "memo" ) );
    // setEditTriggers( QAbstractItemView::AllEditTriggers );

    setColumnHidden( c_msref, true );
}

void
MolTable::setContents( const adcontrols::moltable& mols )
{
    QStandardItemModel& model = *model_;
    adcontrols::ChemicalFormula cformula;

    model.setRowCount( int( mols.data().size() + 1 ) ); // add one free line for add formula

    int row = 0;
    for ( auto& mol : mols.data() ) {
        
        impl_->setData( *this, row
                        , QString::fromStdString( mol.formula() )
                        , QString::fromStdString( mol.adducts() )                       
                        , QString::fromStdString( mol.smiles() )
                        , QByteArray()
                        , QString::fromStdString( mol.synonym() )
                        , QString::fromStdWString( mol.description() )
                        , mol.mass()
                        , mol.abundance()
                        , mol.enable()
                        , mol.isMSRef() );
        ++row;
    }
    resizeRowsToContents();
    resizeColumnsToContents();
}

void
MolTable::getContents( adcontrols::moltable& m )
{
    QStandardItemModel& model = *model_;

    m.data().clear();

    for ( int row = 0; row < model.rowCount(); ++row ) {
        adcontrols::moltable::value_type mol;

        mol.formula() = model.index( row, c_formula ).data( Qt::EditRole ).toString().toStdString();

        if ( !mol.formula().empty() ) {
            
            mol.enable() = model.index( row, c_formula ).data( Qt::CheckStateRole ).toBool();
            mol.adducts() = model.index( row, c_adducts ).data( Qt::EditRole ).toString().toStdString();
            mol.description() = model.index( row, c_description ).data( Qt::EditRole ).toString().toStdWString();
            mol.mass() = model.index( row, c_mass ).data( Qt::EditRole ).toDouble();
            mol.abundance() = model.index( row, c_abundance ).data( Qt::EditRole ).toDouble();
            mol.synonym() = model.index( row, c_synonym ).data( Qt::EditRole ).toString().toStdString();
            mol.smiles() = model.index( row, c_smiles ).data( Qt::EditRole ).toString().toStdString();
            mol.setIsMSRef( model.index( row, c_msref ).data( Qt::CheckStateRole ).toBool() );

            m << mol;
        }
    }
}

void
MolTable::handleValueChanged( const QModelIndex& index )
{
    QString stdFormula;

    if ( index.column() == c_formula ) {

        auto formula = model_->index( index.row(), c_formula ).data( Qt::EditRole ).toString();
        auto adducts = model_->index( index.row(), c_adducts ).data( Qt::EditRole ).toString();
        if ( auto item = model_->item( index.row(), c_formula ) ) {
            if ( !( item->flags() & Qt::ItemIsUserCheckable ) )
                item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | item->flags() );
        }
        model_->setData( index, formula.isEmpty() ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole );
        model_->setData( model_->index( index.row(), c_mass ), computeMass( formula, adducts, stdFormula ) );

        if ( model_->index( index.row(), c_abundance ).data( Qt::EditRole ).isNull() )
            model_->setData( model_->index( index.row(), c_abundance ), 100.0 );
    }

    if ( index.column() == c_adducts ) {
        auto formula = model_->index( index.row(), c_formula ).data( Qt::EditRole ).toString();
        auto adducts = model_->index( index.row(), c_adducts ).data( Qt::EditRole ).toString();
        model_->setData( model_->index( index.row(), c_mass ), computeMass( formula, adducts, stdFormula ) );
    }
    if ( !stdFormula.isEmpty() )
        model_->setData( model_->index( index.row(), c_description ), stdFormula );

    if ( index.column() == c_smiles ) {
        auto smiles = index.data( Qt::EditRole ).toString();
        if ( smiles.isEmpty() ) {
            model_->setData( model_->index( index.row(), c_svg ), QByteArray() );
        } else {
#if HAVE_RDKit
            if ( auto mol = std::unique_ptr< RDKit::ROMol >( RDKit::SmilesToMol( smiles.toStdString(), 0, false ) ) ) {
                mol->updatePropertyCache( false );
                auto formula = QString::fromStdString( RDKit::Descriptors::calcMolFormula( *mol, true, false ) );
                //auto drawing = RDKit::Drawing::MolToDrawing( *mol );
                auto svg = adchem::drawing::toSVG( *mol ); //RDKit::Drawing::DrawingToSVG( drawing );
                auto adducts = model_->index( index.row(), c_adducts ).data( Qt::EditRole ).toString();
                auto synonym = model_->index( index.row(), c_synonym ).data( Qt::EditRole ).toString();
                auto description = model_->index( index.row(), c_description ).data( Qt::EditRole ).toString();
                impl_->setData( *this, index.row(), formula, adducts, smiles, QByteArray( svg.data(), int( svg.size() ) ), synonym, description );
            }
#endif            
        }

    }
    
    if ( index.row() == model_->rowCount() - 1 &&
         !model_->index( index.row(), c_formula ).data( Qt::EditRole ).toString().isEmpty() ) {
        model_->insertRow( index.row() + 1 );
    }

    emit onValueChanged();

    //resizeRowsToContents();
    //resizeColumnsToContents();
}

void
MolTable::setColumnEditable( int column, bool hide )
{
    if ( column >= 0 && column < impl_->editable_.size() )
        impl_->editable_[ column ] = hide;
}

bool
MolTable::isColumnEditable( int column ) const
{
    if ( column >= 0 && column < impl_->editable_.size() )
        return impl_->editable_[ column ];
    return false;
}

void
MolTable::handleContextMenu( const QPoint& pt )
{
    QMenu menu;

    emit onContextMenu( menu, pt );
    
    typedef std::pair< QAction *, std::function< void() > > action_type;

    std::vector< action_type > actions;
    
    actions.push_back( std::make_pair( menu.addAction( "Enable all" ), [=](){ enable_all( true ); }) );
    actions.push_back( std::make_pair( menu.addAction( "Disable all" ), [=](){ enable_all( false ); }) );

    TableView::addActionsToMenu( menu, pt );

    if ( QAction * selected = menu.exec( mapToGlobal( pt ) ) ) {
        auto it = std::find_if( actions.begin(), actions.end(), [=]( const action_type& t ){
                return t.first == selected;
            });
        if ( it != actions.end() )
            (it->second)();
    }
}

void
MolTable::enable_all( bool enable )
{
    QStandardItemModel& model = *model_;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        if ( ! model.index( row, c_formula ).data().toString().isEmpty() )
            model_->setData( model.index( row, c_formula ), enable ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
    }

}

void
MolTable::dragEnterEvent( QDragEnterEvent * event )
{
	const QMimeData * mimeData = event->mimeData();

	if ( mimeData->hasUrls() ) {
		QList<QUrl> urlList = mimeData->urls();
        for ( auto& url: urlList ) {
            QFileInfo path( url.toLocalFile() );
            if ( path.suffix() == "sdf" || path.suffix() == "mol" ) {
                event->accept();
                return;
            }
        }
	}
}

void
MolTable::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
MolTable::dragLeaveEvent( QDragLeaveEvent * event )
{
	event->accept();
}

void
MolTable::dropEvent( QDropEvent * event )
{
	const QMimeData * mimeData = event->mimeData();
    
	if ( mimeData->hasUrls() ) {

        QSignalBlocker block( this );

        int row = model_->rowCount() == 0 ? 0 : model_->rowCount() - 1;
        
        QList<QUrl> urlList = mimeData->urls();
        for ( auto& url : urlList ) {

#if HAVE_RDKit
            std::string filename = url.toLocalFile().toStdString();
            std::cout << "dropEvent: " << filename << std::endl;
            
            if ( auto supplier = std::make_shared< RDKit::SDMolSupplier >( filename, false, false, false ) ) {
                
                model_->insertRows( row, supplier->length() );
                
                for ( size_t i = 0; i < supplier->length(); ++i ) {
                    if ( auto mol = std::unique_ptr< RDKit::ROMol >( ( *supplier )[ i ] ) ) {
                        mol->updatePropertyCache( false );
                        auto formula = QString::fromStdString( RDKit::Descriptors::calcMolFormula( *mol, true, false ) );
                        auto smiles = QString::fromStdString( RDKit::MolToSmiles( *mol ) );
                        // auto drawing = RDKit::Drawing::MolToDrawing( *mol );
                        auto svg = adchem::drawing::toSVG( *mol ); // RDKit::Drawing::DrawingToSVG( drawing );
                        impl_->setData( *this, row, formula, QString(), smiles, QByteArray( svg.data(), int( svg.size() ) ) );
                    }
                    ++row;
                }
            }
            resizeRowsToContents();
#endif            
        }
        event->accept();
	}
}

void
MolTable::handleCopyToClipboard()
{
	QModelIndexList indecies = selectionModel()->selectedIndexes();

    qSort( indecies );
    if ( indecies.size() < 1 )
        return;

    adcontrols::moltable molecules;
    
    QString selected_text;
    QModelIndex prev = indecies.first();
    QModelIndex last = indecies.last();

    indecies.removeFirst();

    adcontrols::moltable::value_type mol;

    for( int i = 0; i < indecies.size(); ++i ) {
        
        QModelIndex index = indecies.at( i );

        if ( !isRowHidden( prev.row() ) ) {

            //auto t = prev.data( Qt::EditRole ).type();
            if ( !isColumnHidden( prev.column() ) && ( prev.column() != MolTable::c_svg ) ) {

                QString text = prev.data( Qt::EditRole ).toString();
                selected_text.append( text );

                if ( index.row() == prev.row() )
                    selected_text.append( '\t' );
            }

            switch( prev.column() ) {
            case MolTable::c_formula: mol.formula() = prev.data( Qt::EditRole ).toString().toStdString(); break;
            case MolTable::c_adducts: mol.adducts() = prev.data( Qt::EditRole ).toString().toStdString(); break;
            case MolTable::c_mass: mol.mass() = prev.data( Qt::EditRole ).toDouble(); break;
            case MolTable::c_abundance: mol.abundance() = prev.data( Qt::EditRole ).toDouble(); break;                
            case MolTable::c_synonym: mol.synonym() = prev.data( Qt::EditRole ).toString().toStdString(); break;
            case MolTable::c_description: mol.description() = prev.data( Qt::EditRole ).toString().toStdWString(); break;
            case MolTable::c_smiles: mol.smiles() = prev.data( Qt::EditRole ).toString().toStdString(); break;
            }
            
            if ( index.row() != prev.row() ) {
                selected_text.append( '\n' );
                molecules << mol;
                mol = adcontrols::moltable::value_type();
            }
        }
        prev = index;
    }

    if ( !isRowHidden( last.row() ) && !isColumnHidden( last.column() ) )
        selected_text.append( last.data( Qt::EditRole ).toString() );

    QApplication::clipboard()->setText( selected_text );
    
    std::wostringstream o;
    try {
        if ( adcontrols::moltable::xml_archive( o, molecules ) ) {
            QString xml( QString::fromStdWString( o.str() ) );
            QMimeData * md = new QMimeData();
            md->setData( QLatin1String( "application/moltable-xml" ), xml.toUtf8() );
            md->setText( selected_text );
            QApplication::clipboard()->setMimeData( md, QClipboard::Clipboard );
        }
    } catch ( ... ) {
    }
}

void
MolTable::handlePaste()
{
    int row = model_->rowCount() - 1;

    auto md = QApplication::clipboard()->mimeData();
    auto data = md->data( "application/moltable-xml" );
    if ( !data.isEmpty() ) {
        QString utf8( QString::fromUtf8( data ) );
        std::wistringstream is( utf8.toStdWString() );

        adcontrols::moltable molecules;
        if ( adcontrols::moltable::xml_restore( is, molecules ) ) {

            model_->setRowCount( row + int( molecules.data().size() + 1 ) ); // add one free line for add formula

            for ( auto& mol : molecules.data() ) {

                impl_->setData( *this, row
                               , QString::fromStdString( mol.formula() )
                               , QString::fromStdString( mol.adducts() )
                               , QString::fromStdString( mol.smiles() )
                               , QByteArray()
                               , QString::fromStdString( mol.synonym() )
                               , QString::fromStdWString( mol.description() )
                               , mol.mass()
                               , mol.abundance()
                               , mol.enable() );
                ++row;
            }
            resizeRowsToContents();
            resizeColumnsToContents();
        }
    } else {
        QString pasted = QApplication::clipboard()->text();
        QStringList lines = pasted.split( "\n" );
        for ( auto line : lines ) {
            QStringList texts = line.split( "\t" );
            for ( auto& test : texts ) {
                (void)test;
                // TODO...
            }
        }
    }
}

