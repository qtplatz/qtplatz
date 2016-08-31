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

#include "moltableview.hpp"
#include "delegatehelper.hpp"
#include "htmlheaderview.hpp"
#include "moldraw.hpp"
#include <adchem/drawing.hpp>
#include <adportable/float.hpp>
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
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <QSvgRenderer>
#include <QUrl>
#include <sstream>

#if defined HAVE_RDKit && HAVE_RDKit
#if defined _MSC_VER
# pragma warning( disable: 4267 4018 )
#endif
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
#include <qtwrapper/font.hpp>
#include <functional>

using namespace adwidgets;

namespace adwidgets {

    namespace ac = adcontrols;

    class MolTableView::impl  {

        MolTableView * this_;

    public:
        impl( MolTableView * p ) : this_( p ) {
        }

        ~impl() {
        }

        inline MolTableView * _this() { return this_; }

        std::function<void(const QPoint& )> handleContextMenu_;

        //-------------------------------------------------
        
        std::map< int, ColumnState > columnStates_;

        inline const ColumnState& state( int column ) { return columnStates_[ column ]; }

        inline ColumnState::fields field( int column ) { return columnStates_[ column ].field; }

        inline int findColumn( ColumnState::fields field ) const {
            auto it = std::find_if( columnStates_.begin(), columnStates_.end()
                                    , [field]( const std::pair< int, ColumnState >& c ){ return c.second.field == field; });
            if ( it != columnStates_.end() )
                return it->first;
            return (-1);
        }

        QAbstractItemModel * model() { return this_->model(); }

        inline void handleEditorValueChanged( const QModelIndex& index, double value ) {
            emit this_->valueChanged( index, value );
        }

    };

    //-------------------------- delegate ---------------
    class MolTableView::delegate : public QStyledItemDelegate {

        MolTableView::impl * impl_;

    public:

        ///////////////////////
        struct paint_f_precision {
            void operator()( const ColumnState& state
                             , QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const {
                painter->drawText( option.rect
                                   , option.displayAlignment
                                   , QString::number( index.data( Qt::EditRole ).toDouble(), 'f', state.precision  ) );
            }
        };
        
        ///////////////////////
        struct paint_f_mass {

            int cformula;
            int cadducts;
            paint_f_mass( int _cformula, int _cadducts ) : cformula( _cformula ), cadducts( _cadducts ) {}

            void operator()( const ColumnState& state, QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) {
                painter->save();
                if ( cformula >= 0 ) {
                    auto expr = index.model()->index( index.row(), cformula ).data( Qt::EditRole ).toString();
                    
                    if ( cadducts >= 0 )
                        expr += " " + index.model()->index( index.row(), cadducts ).data( Qt::EditRole ).toString();

                    double exactMass = ac::ChemicalFormula().getMonoIsotopicMass( ac::ChemicalFormula::split( expr.toStdString() ) );
                    if ( exactMass > 0.7 ) {  // Any 'chemical formula' mass should be > 1.0 (Hydrogen := 1.007825)
                        double mass = index.data( Qt::EditRole ).toDouble();
                        if ( !( adportable::compare<double>::approximatelyEqual( mass, 0 ) ||
                                adportable::compare<double>::approximatelyEqual( exactMass, mass ) ) )
                            painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x40 ) ); // tomato
                    }
                    paint_f_precision()( state, painter, option, index );
                }
                painter->restore();                
            }
        };
        
        ///////////////////////
        struct paint_f_formula {
            void operator()( const ColumnState& state, QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const {

                std::string formula = ac::ChemicalFormula::formatFormulae( index.data().toString().toStdString() );
                DelegateHelper::render_html2( painter, option, QString::fromStdString( formula ) );
            }
        };

        ///////////////////////
        struct paint_f_abundance {

            void operator()( const ColumnState& state, QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const {
                painter->save();
                if ( index.data().toDouble() <= 0.0001 ) {
                    painter->fillRect( option.rect, QColor( 0xff, 0x63, 0x47, 0x80 ) ); // tomato
                }
                painter->restore();                
                paint_f_precision()( state, painter, option, index );
            }
        };

        ///////////////////////
        struct paint_f_svg {

            void operator()( const ColumnState&, QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const {
                painter->save();
                QSvgRenderer renderer( index.data().toByteArray() );
                painter->translate( option.rect.x(), option.rect.y() );
                QRectF viewport = painter->viewport();
                painter->scale( 1.0, 1.0 );
                QRect target( 0, 0, option.rect.width(), option.rect.height() );
                renderer.render( painter, target );
                painter->restore();
            }
        };

        /////////////////////////

        delegate( impl * p ) : impl_( p ) {
        }
        
        void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
            
            QStyleOptionViewItem opt(option);
            initStyleOption( &opt, index );
            opt.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
            auto& state = impl_->state( index.column() );
            auto field = impl_->field( index.column() );

            if ( state.isChoice() ) {

                int idx = index.data().toInt();
				if ( idx >= 0 && idx  < state.choice.size() )
                    painter->drawText( option.rect, Qt::AlignHCenter | Qt::AlignVCenter, state.choice[ idx ].first );

            } else if ( field == ColumnState::f_formula ) {
                
                paint_f_formula()( state, painter, opt, index );
                
            } else if ( field == ColumnState::f_abundance ) {

                paint_f_abundance()( state, painter, option, index );                

            } else if ( field == ColumnState::f_mass ) {

                paint_f_mass( impl_->findColumn( ColumnState::f_formula )
                              , impl_->findColumn( ColumnState::f_adducts ) )( state, painter, opt, index );

            } else if ( field == ColumnState::f_svg ) {

                paint_f_svg()( state, painter, opt, index );

            } else if ( impl_->state( index.column() ).precision && index.data( Qt::EditRole ).canConvert<double>() ) {

                paint_f_precision()( state, painter, opt, index );

            } else {
                QStyledItemDelegate::paint( painter, opt, index );
            }
        }

        void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {

			auto& state = impl_->state( index.column() );

			if ( state.isChoice() ) {
				if ( auto combo = qobject_cast<QComboBox *>( editor ) ) {
					int idx = combo->currentIndex();
					if ( idx >= 0 && idx < state.choice.size() )
						model->setData( index, impl_->state( index.column() ).choice[ combo->currentIndex() ].second, Qt::EditRole );
				}
            } else 
                QStyledItemDelegate::setModelData( editor, model, index );

            // impl_->onValueChanged( index );
        }

        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {

            auto& state = impl_->state( index.column() );

            if ( state.isChoice() ) {
                auto combo = new QComboBox( parent );
                for ( auto& x : state.choice )
                    combo->addItem( x.first );
                return combo;
            } else if ( state.precision && index.data( Qt::EditRole ).canConvert< double >() ) {
				auto spin = new QDoubleSpinBox( parent );
                spin->setDecimals( state.precision );
                spin->setMaximum( 100000 );
				spin->setSingleStep( std::pow( 10, -state.precision ) );
                connect( spin, static_cast< void( QDoubleSpinBox::* )(double) >(&QDoubleSpinBox::valueChanged)
                         , [=]( double value ){ impl_->handleEditorValueChanged( index, value ); });
                return spin;
            } else {
                return QStyledItemDelegate::createEditor( parent, option, index );
            }
        }

        bool editorEvent( QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem& option, const QModelIndex& index ) override {

            if ( event->type() == QEvent::MouseButtonRelease && model->flags(index) & Qt::ItemIsUserCheckable ) {

                const Qt::CheckState prev = index.data( Qt::CheckStateRole ).value< Qt::CheckState >();

                if ( QStyledItemDelegate::editorEvent( event, model, option, index ) ) {

                    if ( index.data( Qt::CheckStateRole ).value< Qt::CheckState >() != prev ) {
                        // check state has changed
                        emit impl_->_this()->stateChanged( index, index.data( Qt::CheckStateRole ).value< Qt::CheckState >() );
                    }
                    return true;
                }
            } else {
                return QStyledItemDelegate::editorEvent( event, model, option, index );
            }
        }
        
        QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {

            auto field = impl_->field( index.column() );

            if ( field == ColumnState::f_svg && !index.data( Qt::EditRole ).toByteArray().isEmpty() ) {
                return QSize( 80, 80 );
            } else if ( field == ColumnState::f_formula ) {
                return DelegateHelper::html_size_hint( option, index );
            } else {
                return QStyledItemDelegate::sizeHint( option, index );
            }
        }
        
    };
}



////////////////////////////////////////
MolTableView::MolTableView(QWidget *parent) : TableView(parent)
                                            , impl_( new impl( this ) )
{
    setHorizontalHeader( new HtmlHeaderView );
    setItemDelegate( new delegate( impl_.get() ) );

    setSortingEnabled( true );
    setAcceptDrops( true );

    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );

    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, &QTableView::customContextMenuRequested, [this]( const QPoint& pt ){
            if ( impl_->handleContextMenu_ )
                impl_->handleContextMenu_( pt );
        });
}

MolTableView::~MolTableView()
{
}

void
MolTableView::onInitialUpdate()
{
    horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
}

void
MolTableView::setChoice( int column, const std::vector< std::pair< QString, QVariant > >& choice )
{
    impl_->columnStates_[ column ].choice = choice;
}

void
MolTableView::setPrecision( int column, int prec )
{
    impl_->columnStates_[ column ].precision = prec;
}

void
MolTableView::setColumnEditable( int column, bool hide )
{
}

const ColumnState&
MolTableView::columnState( int column ) const
{
    auto it = impl_->columnStates_.find( column );
    if ( it != impl_->columnStates_.end() )
        return it->second;
    throw std::out_of_range( "MolTableView::columnState subscript out of range" );
}

bool
MolTableView::isColumnEditable( int column ) const
{
    auto it = impl_->columnStates_.find( column );
    if ( it != impl_->columnStates_.end() )
        return it->second.isEditable;
    throw std::out_of_range( "MolTableView::isColumnEditable subscript out of range" );
}

bool
MolTableView::isColumnCheckable( int column ) const
{
    auto it = impl_->columnStates_.find( column );
    if ( it != impl_->columnStates_.end() )
        return it->second.isCheckable;
    throw std::out_of_range( "MolTableView::isColumnCheckable subscript out of range" );
}

void
MolTableView::setContextMenuHandler( std::function<void(const QPoint& )> f )
{
    impl_->handleContextMenu_ = f;
}

void
MolTableView::handleContextMenu( const QPoint& pt )
{
    if ( impl_->handleContextMenu_ )
        impl_->handleContextMenu_( pt );
}

void
MolTableView::dragEnterEvent( QDragEnterEvent * event )
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
MolTableView::dragMoveEvent( QDragMoveEvent * event )
{
    event->accept();
}

void
MolTableView::dragLeaveEvent( QDragLeaveEvent * event )
{
	event->accept();
}

void
MolTableView::dropEvent( QDropEvent * event )
{
	const QMimeData * mimeData = event->mimeData();
    
	if ( mimeData->hasUrls() ) {

        QSignalBlocker block( this );
#if 0
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
                    }
                    ++row;
                }
            }
            resizeRowsToContents();
#endif            
        }
        event->accept();
#endif
	}
}

void
MolTableView::handleCopyToClipboard()
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

            auto t = prev.data( Qt::EditRole ).type();
            if ( !isColumnHidden( prev.column() ) && ( impl_->state( prev.column() ).field != ColumnState::f_svg ) ) {

                QString text = prev.data( Qt::EditRole ).toString();
                selected_text.append( text );

                if ( index.row() == prev.row() )
                    selected_text.append( '\t' );
            }

            switch( impl_->field( prev.column() ) ) {
            case ColumnState::f_formula: mol.formula() = prev.data( Qt::EditRole ).toString().toStdString(); break;
            case ColumnState::f_adducts: mol.adducts() = prev.data( Qt::EditRole ).toString().toStdString(); break;
            case ColumnState::f_mass: mol.mass() = prev.data( Qt::EditRole ).toDouble(); break;
            case ColumnState::f_abundance: mol.abundance() = prev.data( Qt::EditRole ).toDouble(); break;                
            case ColumnState::f_synonym: mol.synonym() = prev.data( Qt::EditRole ).toString().toStdString(); break;
            case ColumnState::f_description: mol.description() = prev.data( Qt::EditRole ).toString().toStdWString(); break;
            case ColumnState::f_smiles: mol.smiles() = prev.data( Qt::EditRole ).toString().toStdString(); break;
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
MolTableView::handlePaste()
{
    auto md = QApplication::clipboard()->mimeData();
    auto data = md->data( "application/moltable-xml" );
    if ( !data.isEmpty() ) {
        QString utf8( QString::fromUtf8( data ) );
        std::wistringstream is( utf8.toStdWString() );

        adcontrols::moltable molecules;
        if ( adcontrols::moltable::xml_restore( is, molecules ) ) {
#if 0
            impl_->model_->setRowCount( row + int( molecules.data().size() + 1 ) ); // add one free line for add formula

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
#endif
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

void
MolTableView::setColumnField( int column, ColumnState::fields f, bool editable, bool checkable )
{
    impl_->columnStates_[ column ] = ColumnState( f, editable, checkable );
	if ( f == ColumnState::f_mass )
        impl_->columnStates_[ column ].precision = 7;
}

// static
double
MolTableView::getMonoIsotopicMass( const QString& formula, const QString& adducts )
{
    auto expr = formula;

    if ( ! adducts.isEmpty() )
        expr += " " + adducts;

    double exactMass = ac::ChemicalFormula().getMonoIsotopicMass( ac::ChemicalFormula::split( expr.toStdString() ) );
    
    return exactMass;
}

