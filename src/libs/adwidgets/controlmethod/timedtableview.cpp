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

#include "timedtableview.hpp"
#include "delegatehelper.hpp"
#include "htmlheaderview.hpp"
#include <adportable/float.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/controlmethod/modulecap.hpp>
#include <adportable/float.hpp>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <QSvgRenderer>
#include <QUrl>
#include <sstream>

#include <boost/format.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <qtwrapper/font.hpp>
#include <functional>

using namespace adwidgets;

namespace adwidgets {

    namespace ac = adcontrols;

    class TimedTableView::impl  {

        TimedTableView * this_;

    public:
        impl( TimedTableView * p ) : this_( p ) {
        }

        ~impl() {
        }

        std::function<void(const QPoint& )> handleContextMenu_;

        //-------------------------------------------------
        std::vector < adcontrols::ControlMethod::ModuleCap > capList_;
        
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
    };

    //-------------------------- delegate ---------------
    class TimedTableView::delegate : public QStyledItemDelegate {

        TimedTableView::impl * impl_;

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
                    if ( exactMass > 0.7 ) {  // Any 'chemical formula' string will be > 1.0 (Hydrogen)
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
                auto formula = QString::fromStdString( ac::ChemicalFormula::formatFormulae( index.data().toString().toStdString() ) );
                DelegateHelper::render_html2( painter, option, ( formula.isEmpty() ? index.data().toString() : formula ) );
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
                
                paint_f_formula()( state, painter, option, index );
                
            } else if ( field == ColumnState::f_abundance ) {

                paint_f_abundance()( state, painter, option, index );                

            } else if ( field == ColumnState::f_mass ) {

                paint_f_mass( impl_->findColumn( ColumnState::f_formula )
                              , impl_->findColumn( ColumnState::f_adducts ) )( state, painter, option, index );

            } else if ( index.column() == ColumnState::f_svg ) {

                paint_f_svg()( state, painter, option, index );

            } else if ( impl_->state( index.column() ).precision && index.data( Qt::EditRole ).canConvert<double>() ) {

                paint_f_precision()( state, painter, option, index );

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
                return spin;
            } else {
                return QStyledItemDelegate::createEditor( parent, option, index );
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

TimedTableView::TimedTableView(QWidget *parent) : TableView(parent)
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

TimedTableView::~TimedTableView()
{
}

void
TimedTableView::onInitialUpdate()
{
    horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
}

void
TimedTableView::setChoice( int column, const std::vector< std::pair< QString, QVariant > >& choice )
{
    impl_->columnStates_[ column ].choice = choice;
}

void
TimedTableView::setPrecision( int column, int prec )
{
    impl_->columnStates_[ column ].precision = prec;
}

void
TimedTableView::setColumnEditable( int column, bool hide )
{
}

bool
TimedTableView::isColumnEditable( int column ) const
{
    return false;
}

void
TimedTableView::setContextMenuHandler( std::function<void(const QPoint& )> f )
{
    impl_->handleContextMenu_ = f;
}

void
TimedTableView::handleContextMenu( const QPoint& pt )
{
    if ( impl_->handleContextMenu_ )
        impl_->handleContextMenu_( pt );
}

void
TimedTableView::dragEnterEvent( QDragEnterEvent * event )
{
}

void
TimedTableView::dragMoveEvent( QDragMoveEvent * event )
{
}

void
TimedTableView::dragLeaveEvent( QDragLeaveEvent * event )
{
}

void
TimedTableView::dropEvent( QDropEvent * event )
{
}

void
TimedTableView::handleCopyToClipboard()
{
}

void
TimedTableView::handlePaste()
{
}

void
TimedTableView::setColumnField( int column, ColumnState::fields f, bool editable, bool checkable )
{
    impl_->columnStates_[ column ] = ColumnState( f, editable, checkable );
	if ( f == ColumnState::f_mass )
        impl_->columnStates_[ column ].precision = 7;
}

void
TimedTableView::addModuleCap( const std::vector< adcontrols::ControlMethod::ModuleCap >& cap )
{
    std::copy( cap.begin(), cap.end(), std::back_inserter( impl_->capList_ ) );
}

const std::vector< adcontrols::ControlMethod::ModuleCap >&
TimedTableView::moduleCap() const
{
    return impl_->capList_;
}

