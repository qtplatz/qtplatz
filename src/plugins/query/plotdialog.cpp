#include "plotdialog.hpp"
#include "ui_plotdialog.h"
#include <adportable/debug.hpp>
#include <QMenu>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QStyleOptionViewItem>

namespace query {

    class PlotDialog::delegate : public QStyledItemDelegate {
        const QStringList& list_;
    public:
        delegate( const QStringList& list ) : list_( list )
            {}

        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {
            if ( index.parent() != QModelIndex() && index.column() == 1 ) {
                auto combo = new QComboBox( parent );
                combo->setGeometry( option.rect );
                combo->addItems( list_ );
                return combo;
            }
            return QStyledItemDelegate::createEditor( parent, option, index );
        }
    };
    
}

using namespace query;

PlotDialog::PlotDialog(QWidget *parent) : QDialog(parent)
                                        , ui(new Ui::PlotDialog)
{
    ui->setupUi(this);
    connect( ui->buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    connect( ui->buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

    ui->comboBox->addItems( QStringList() << "Histogram" << "Scatter" << "Line" );

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( ui->treeView, &QTreeView::customContextMenuRequested, this, &PlotDialog::handleContextMenuRequested );
}

PlotDialog::~PlotDialog()
{
    delete ui;
}

void
PlotDialog::setModel( QAbstractItemModel * record )
{
    for ( int col = 0; col < record->columnCount(); ++col )
        list_ << record->headerData( col, Qt::Horizontal ).toString();

    if ( list_.size() >= 2 ) {
        if ( auto model = new QStandardItemModel ) {
            model->setColumnCount( 2 );
            model->setRowCount( 1 );

            model->setData( model->index( 0, 0 ), "Plot 1", Qt::EditRole );

            // child item
            int plot = 0;
            auto parent = model->item( plot, 0 );
            for ( auto label: { "X", "Y" } )
                parent->appendRow( QList< QStandardItem * >() << new QStandardItem( label ) << new QStandardItem() );

            model->itemFromIndex( model->index( 0, 0, parent->index() ) )->setEditable( false ); // X (title)
            model->itemFromIndex( model->index( 1, 0, parent->index() ) )->setEditable( false ); // Y (title)
            
            model->setData( model->index( 0, 1, parent->index() ), list_[0], Qt::EditRole );
            model->setData( model->index( 1, 1, parent->index() ), (list_.size() > 2 ) ? list_[2] : list_[ 1 ], Qt::EditRole );

            ui->treeView->setModel( model );
            ui->treeView->setItemDelegate( new delegate( list_ ) );
            ui->treeView->header()->hide();
            ui->treeView->resizeColumnToContents( 0 );
            ui->treeView->resizeColumnToContents( 1 );
            ui->treeView->expandAll();
        }
    }
}

void
PlotDialog::handleContextMenuRequested( const QPoint& pos )
{
	QPoint globalPos = ui->treeView->mapToGlobal(pos);
	QModelIndex index = ui->treeView->currentIndex();

    if ( index.parent() != QModelIndex() && index.column() == 1 ) {
    
        QMenu menu;
        
        if ( index.isValid() ) {
            for ( auto text: list_ )
                menu.addAction( text );
        }
        
        if ( auto selected = menu.exec( globalPos ) ) {
            auto model = ui->treeView->model();

            auto text = selected->text();
            ADDEBUG() << text.toStdString();
            
            model->setData( index, selected->text(), Qt::EditRole );
        }
    }
}

std::vector< std::tuple< QString, int, int > >
PlotDialog::plots() const
{
    std::vector< std::tuple< QString, int, int > > v;
    
    if ( auto model = ui->treeView->model() ) {
        for ( int row = 0; row < model->rowCount(); ++row ) {
            auto parent = model->index( row, 0 );
            auto name = model->data( parent ).toString();
            auto X = model->data( model->index( 0, 1, parent ) ).toString();
            int iX = list_.indexOf( X );
            
            auto Y = model->data( model->index( 1, 1, parent ) ).toString();
            int iY = list_.indexOf( Y );

            v.emplace_back( name, iX, iY );
        }
    }
    return v;
}

QString
PlotDialog::chartType() const
{
    return ui->comboBox->currentText();
}

bool
PlotDialog::clearExisting() const
{
    return ui->checkBox->isChecked();
}

void
PlotDialog::setClearExisting( bool check )
{
    ADDEBUG() << "setClearExisting( " << check << ")";
    ui->checkBox->setChecked( check );
}

void
PlotDialog::setChartType( const QString& type )
{
    ADDEBUG() << "setChartType( " << type.toStdString() << ui->comboBox->currentIndex() << ")";
    ui->comboBox->setCurrentText( type );
    ADDEBUG() << "setChartType( " << ui->comboBox->currentIndex() << ")";
}

void
PlotDialog::setPlot( size_t row, const QString& title, int x , int y )
{
    ADDEBUG() << "setPlot( " << row << ", " << title.toStdString() << ", " << x << ", " << y << ")";
    
    if ( auto model = qobject_cast< QStandardItemModel * >( ui->treeView->model() ) ) {
        if ( row >= model->rowCount() )
            model->setRowCount( row + 1 );
        auto parent = model->index( row, 0 );

        // title
        model->setData( model->index( 0, 0 ), title, Qt::EditRole );

        // child
        model->setData( model->index( 0, 1, parent ), (list_.size() > x ) ? list_[x] : list_[ 0 ], Qt::EditRole );
        model->setData( model->index( 1, 1, parent ), (list_.size() > y ) ? list_[y] : list_[ 1 ], Qt::EditRole );
    }
}

