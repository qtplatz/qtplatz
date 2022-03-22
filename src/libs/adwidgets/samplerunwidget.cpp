/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "samplerunwidget.hpp"
#include "tableview.hpp"
#include <adcontrols/samplerun.hpp>
#ifndef Q_MOC_RUN
#include <adportable/is_type.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#endif
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/scoped_flag.hpp>
#include <qtwrapper/font.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTextEdit>
#include <QDebug>

namespace adwidgets {

    namespace {
        enum {
            c_item_name
            , c_item_value
            , c_description
        };

        enum {
            r_method_time
            , r_replicates
            , r_ionization
            , r_directory
            , r_filename
        };

        class ItemDelegate : public QStyledItemDelegate {
        public:
            void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
                QStyleOptionViewItem op( option );
                initStyleOption( &op, index );
                if ( index.column() == 1 && index.row() == 0 ) { // method time
                    if ( auto item = qobject_cast< const QStandardItemModel * >(index.model())->itemFromIndex( index ) ) {
                        painter->setBrush( item->background() );
                        painter->drawRect( option.rect );
                    }
                    painter->drawText( option.rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( index.data().toDouble(), 'f', 1 ) );
                } else {
                    if ( index.column() == c_item_name || (index.column() == c_item_value && index.row() == r_method_time) )
                        op.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    if ( index.row() == r_directory && index.column() == c_item_value )
                        op.textElideMode = Qt::ElideLeft;
                    QStyledItemDelegate::paint( painter, op, index );
                }
            }

            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
                if ( index.row() == r_ionization && index.column() == c_item_value ) {
                    if ( auto combo = qobject_cast<QComboBox *>( editor ) )
                        model->setData( index, combo->currentText(), Qt::EditRole );
                } else {
                    QStyledItemDelegate::setModelData( editor, model, index );
                }
            }

            QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override {
                if ( index.row() == r_ionization && index.column() == c_item_value ) {
                    auto combo = new QComboBox( parent );
                    combo->addItems( { "ESI(+)", "ESI(-)", "PTR(+)", "PTR(-)" } );
                    return combo;
                } else {
                    return QStyledItemDelegate::createEditor( parent, option, index );
                }
            }

        };

    }

    class SampleRunTable : public TableView {
        Q_OBJECT
    public:
        explicit SampleRunTable( QWidget *parent = 0) : TableView( parent )
                                                      , model_( new QStandardItemModel )
                                                      , blockBackgroundColor_ ( false ) {
            setModel( model_ );
            setItemDelegate( new ItemDelegate );

            connect( model_, SIGNAL( dataChanged(const QModelIndex&, const QModelIndex& ) )
                     , this, SLOT( handleDataChanged( const QModelIndex&, const QModelIndex& ) ) );
        }

        ~SampleRunTable() {
        }

        void onInitialUpdate() {
            QStandardItemModel& model = *model_;
            static const QPair< QString, QString > headings[] = {
                { tr( "Method time (seconds):" ), tr( "Methond run length" ) }
                , { tr( "Replicates:" ),          tr( "Number of samples to be acquired" ) }
                , { tr( "Ionization:" ),          tr( "Ionization/polarity" ) }
                , { tr( "Data save in:" ),        tr( "Data directory where data to be stored" ) }
                , { tr( "Filename:" ),            tr( "Initial filename for data, name to be incremented" ) }
            };
            adportable::scoped_flag flag( blockBackgroundColor_ );
            model.setColumnCount( 3 );
            model.setHeaderData( c_item_name, Qt::Horizontal, QObject::tr( "parameter" ) );
            model.setHeaderData( c_item_value, Qt::Horizontal, QObject::tr( "value" ) );
            model.setHeaderData( c_description, Qt::Horizontal, QObject::tr( "description" ) );
            model.setRowCount( sizeof( headings ) / sizeof( headings[ 0 ] ) );

            for ( int row = 0; row < model.rowCount(); ++row ) {
                model.setData( model.index( row, 0 ), headings[row].first );
                model.setData( model.index( row, 2 ), headings[row].second );
            }
            setContents( adcontrols::SampleRun() );
        }

        bool setContents( const adcontrols::SampleRun& t ) {
            QSignalBlocker block( model_ );
            QStandardItemModel& model = *model_;

            model.setData( model.index( r_method_time, 1 ), t.methodTime() ); // shows in seconds
            model.setData( model.index( r_replicates, 1 ), int( t.replicates() ) );
            model.setData( model.index( r_directory, 1 ), QString::fromStdWString( t.dataDirectory() ) );
            model.setData( model.index( r_filename, 1 ), QString::fromStdWString( t.filePrefix() ) );
            model.setData( model.index( r_ionization, 1 ), "ESI(+)" );

            resizeColumnsToContents();
            resizeRowsToContents();

            return true;
        }

        bool getContents( adcontrols::SampleRun& t ) const {
            QStandardItemModel& model = *model_;

            t.methodTime( model.index( r_method_time, 1 ).data().toDouble() ); // stored in seconds
            t.replicates( model.index( r_replicates, 1 ).data().toInt() );
            t.setDataDirectory( model.index( r_directory, 1 ).data().toString().toStdWString() );
            t.setFilePrefix( model.index( r_filename, 1 ).data().toString().toStdWString() );
            t.setIonization( model.index( r_ionization, 1 ).data().toString().toStdString() );
            t.setPolarityPositive( model.index( r_ionization, 1 ).data().toString().contains("(+)") );

            // QSignalBlocker block( model_ );
            adportable::scoped_flag flag( blockBackgroundColor_ );
            for ( int row = 0; row < model.rowCount(); ++row ) {
                model.itemFromIndex( model.index( row, c_item_value ) )->setBackground( QColor( Qt::white ) );
            }
            return true;
        }

    private:
        QStandardItemModel * model_;
        mutable bool blockBackgroundColor_;

        // TableView
        void addActionsToContextMenu( QMenu& menu, const QPoint& pt ) const override {
            auto menuIndex = indexAt( pt );
            menu.addAction( tr( "Set default" ), this, SLOT( setDefault() ) );
            auto action = menu.addAction( tr( "Find directory" ), this, SLOT( findDirectory( QModelIndex& menuIndex ) ) );
            if ( !(menuIndex.column() == 1 && menuIndex.row() == 2) )
                action->setEnabled( false );
            TableView::addActionsToContextMenu( menu, pt );
        }

    signals:

    public slots:
        void handleDataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight ) {
            if ( !blockBackgroundColor_ ) {
                for ( int row = topLeft.row(); row <= bottomRight.row(); ++row ) {
                    if ( topLeft.column() <= c_item_value && c_item_value <= bottomRight.column() ) {
                        model_->itemFromIndex( model_->index( row, c_item_value ) )->setBackground( QColor( Qt::yellow ) );
                    }
                }
            }
        }

        void findDirectory( const QModelIndex& menuIndex ) {
            if ( menuIndex.isValid() && menuIndex.column() == 1 && menuIndex.row() == 2 ) {
                QString dir = menuIndex.data().toString();
                dir = QFileDialog::getExistingDirectory( this, "Data save in:", dir );
                model_->setData( menuIndex, dir );
            }
        }

        void setDefault() {
            boost::filesystem::path path( adportable::profile::user_data_dir< char >() );
            path /= "data";
            path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
            model_->setData( model_->index( r_directory, c_item_value ), QString::fromStdWString( path.wstring() ) );
        }

    };

}

using namespace adwidgets;

SampleRunWidget::SampleRunWidget(QWidget *parent) :  QWidget(parent)
{
    if ( auto topLayout = new QVBoxLayout( this ) ) {

        if ( QSplitter * splitter = new QSplitter ) {

            splitter->addWidget( new SampleRunTable );
            splitter->addWidget( new QTextEdit );

            splitter->setOrientation( Qt::Horizontal );
            splitter->setStretchFactor( 0, 4 );
            splitter->setStretchFactor( 1, 1 );

            if ( QVBoxLayout * layout = new QVBoxLayout ) {
                layout->setMargin( 0 );
                layout->setSpacing( 0 );
                layout->addWidget( splitter );
                topLayout->addLayout( layout );
            }
        }

        if ( auto vLayout = new QHBoxLayout ) {
            if ( auto button = qtwrapper::make_widget<QPushButton>( "apply", tr("Apply") ) ) {
                vLayout->addWidget( button );
                connect( button, &QPushButton::released, this, [&](){ emit apply(); } );
            }
            if ( auto button = qtwrapper::make_widget< QPushButton >( "reset-folder", tr("Reset folder to default") ) ) {
                vLayout->addWidget( button );
                if ( auto table = findChild< SampleRunTable * >() )
                    connect( button, &QPushButton::released, table, &SampleRunTable::setDefault );
            }
            topLayout->addLayout( vLayout );
        }
    }
}

void
SampleRunWidget::OnCreate( const adportable::Configuration& )
{
}

void
SampleRunWidget::OnInitialUpdate()
{
    if ( auto table = findChild< SampleRunTable * >() )
        table->onInitialUpdate();
}

void
SampleRunWidget::OnFinalClose()
{
}

bool
SampleRunWidget::getContents( boost::any& a ) const
{
    if ( adportable::a_type< std::shared_ptr< adcontrols::SampleRun > >::is_a( a ) ) {
        if ( auto ptr = boost::any_cast<std::shared_ptr< adcontrols::SampleRun >>(a) ) {
            getSampleRun( *ptr );
            return true;
        }
    }
    else if ( adportable::a_type< adcontrols::SampleRun >::is_pointer( a ) ) {
        if ( auto ptr = boost::any_cast<adcontrols::SampleRun *>(a) ) {
            getSampleRun( *ptr );
            return true;
        }
    }
    return false;
}

bool
SampleRunWidget::setContents( boost::any&& a )
{
    if ( adportable::a_type< adcontrols::SampleRun >::is_const_pointer( a ) ) {

        auto p = boost::any_cast< const adcontrols::SampleRun * >( a );
        setSampleRun( *p );
        return true;

    } else if ( adportable::a_type< adcontrols::SampleRun >::is_pointer( a ) ) {

        auto p = boost::any_cast< adcontrols::SampleRun * >( a );
        setSampleRun( *p );
        return true;

    } else if ( adportable::a_type< adcontrols::SampleRun >::is_a( a ) ) {

        auto t = boost::any_cast<const adcontrols::SampleRun& >( a );
        setSampleRun( t );
        return true;

    }
    return false;
}

void
SampleRunWidget::setSampleRun( const adcontrols::SampleRun& t )
{
    if ( auto table = findChild< SampleRunTable * >() ) {
        table->setContents( t );
    }
    if ( auto edit = findChild< QTextEdit * >() ) {
        edit->setHtml( t.description() );
    }
}

void
SampleRunWidget::getSampleRun( adcontrols::SampleRun& t ) const
{
    if ( auto table = findChild< SampleRunTable * >() ) {
        table->getContents( t );
    }
    if ( auto edit = findChild< QTextEdit * >() )
        t.description( edit->toHtml().toStdString().c_str() );
}

void
SampleRunWidget::handleRunning( bool running )
{
    if ( auto button = findChild< QPushButton * >( "apply" ) ) {
        button->setEnabled( !running );
    }
}

#include "samplerunwidget.moc"
