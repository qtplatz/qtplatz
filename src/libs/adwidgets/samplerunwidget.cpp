/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <adportable/scoped_flag.hpp>
#include <adportable/date_string.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <qtwrapper/font.hpp>
#include <QAction>
#include <QBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QTextEdit>


namespace adwidgets {

    namespace sampleruntable {
        enum {
            c_item_name
            , c_item_value
            , c_description
        };

        class ItemDelegate : public QStyledItemDelegate {
        public:
            void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {

                if ( index.column() == 1 && index.row() == 0 ) { // method time
                    painter->drawText( option.rect, Qt::AlignRight | Qt::AlignVCenter, QString::number( index.data().toDouble(), 'f', 2 ) );
                } else {
                    QStyleOptionViewItem op( option );
                    initStyleOption( &op, index );
                    if ( index.column() == 0 || (index.column() == 1 && index.row() == 1) )
                        op.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
                    if ( index.row() == 2 && index.column() == 1 )
                        op.textElideMode = Qt::ElideLeft;
                    QStyledItemDelegate::paint( painter, op, index );
                }
            }
        };

    }
    
    class SampleRunTable : public TableView {
        Q_OBJECT
    public:
        explicit SampleRunTable( QWidget *parent = 0) : TableView( parent )
                                                      , model_( new QStandardItemModel )
                                                      , inProgress_( false ) {
            setModel( model_ );
            setItemDelegate( new sampleruntable::ItemDelegate );

            connect( model_, SIGNAL( dataChanged(const QModelIndex&, const QModelIndex& ) )
                     , this, SLOT( handleDataChanged( const QModelIndex&, const QModelIndex& ) ) );
        }

        ~SampleRunTable() {
        }

        void onInitialUpdate() {
            adportable::scoped_flag scope_lock( inProgress_ );
            QStandardItemModel& model = *model_;
            static const QPair< QString, QString > headings[] = {
                { tr( "Method time (seconds):" ), tr( "Methond run length" ) }
                , { tr( "Replicates:" ), tr( "Number of samples to be acquired" ) }
                , { tr( "Data save in:" ), tr( "Data directory where data to be stored" ) }
                , { tr( "Filename:" ), tr( "Initial filename for data, name to be incremented" ) }
            };

            model.setColumnCount( 3 );
            model.setHeaderData( 0, Qt::Horizontal, QObject::tr( "parameter" ) );
            model.setHeaderData( 1, Qt::Horizontal, QObject::tr( "value" ) );
            model.setHeaderData( 2, Qt::Horizontal, QObject::tr( "description" ) );
            model.setRowCount( sizeof( headings ) / sizeof( headings[ 0 ] ) );

            for ( int row = 0; row < model.rowCount(); ++row ) {
                model.setData( model.index( row, 0 ), headings[row].first );
                model.setData( model.index( row, 2 ), headings[row].second );
            }
            setContents( adcontrols::SampleRun() );
        }

        bool setContents( const adcontrols::SampleRun& t ) {

            adportable::scoped_flag scope_lock( inProgress_ );
            QStandardItemModel& model = *model_;

            //model.setData( model.index( 0, 1 ), t.methodTime() / 60.0 ); // shows in minutes
            model.setData( model.index( 0, 1 ), t.methodTime() ); // shows in seconds
            model.setData( model.index( 1, 1 ), int( t.replicates() ) );
            model.setData( model.index( 2, 1 ), QString::fromStdWString( t.dataDirectory() ) );
            model.setData( model.index( 3, 1 ), QString::fromStdWString( t.filePrefix() ) );
            resizeColumnsToContents();
            resizeRowsToContents();

            return true;
        }

        bool getContents( adcontrols::SampleRun& t ) const {
            QStandardItemModel& model = *model_;
            //t.methodTime( model.index( 0, 1 ).data().toDouble() * 60.0 ); // stored in seconds
            t.methodTime( model.index( 0, 1 ).data().toDouble() ); // stored in seconds
            t.replicates( model.index( 1, 1 ).data().toInt() );
            t.setDataDirectory( model.index( 2, 1 ).data().toString().toStdWString() );
            t.setFilePrefix( model.index( 3, 1 ).data().toString().toStdWString() );
            return true;
        }

    private:
        QStandardItemModel * model_;
        bool inProgress_;
        // QModelIndex menuIndex_;

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
            if ( inProgress_ )
                return;
            using namespace sampleruntable;
            QStandardItemModel& model = *model_;
            for ( int row = topLeft.row(); row <= bottomRight.row(); ++row )
                model.itemFromIndex( model.index( row, c_item_value ) )->setBackground( QColor( Qt::yellow ) );
        }

        void findDirectory( const QModelIndex& menuIndex ) {
            if ( menuIndex.isValid() && menuIndex.column() == 1 && menuIndex.row() == 2 ) {
                QString dir = menuIndex.data().toString();
                dir = QFileDialog::getExistingDirectory( this, "Data save in:", dir );
                model_->setData( menuIndex, dir );
            }
            // menuIndex_ = QModelIndex();
        }

        void setDefault() {
            adportable::scoped_flag lock( inProgress_ );
            
            boost::filesystem::path path( adportable::profile::user_data_dir< char >() );
            path /= "data";
            path /= adportable::date_string::string( boost::posix_time::second_clock::local_time().date() );
            model_->setData( model_->index( 2, 1 ), QString::fromStdWString( path.wstring() ) );
            // menuIndex_ = QModelIndex();

            ADDEBUG() << "setDefault(" << path.string() << ")";
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
            if ( auto button = new QPushButton( tr("Apply") ) ) {
                vLayout->addWidget( button );
                connect( button, &QPushButton::released, this, [this](){ emit apply(); } );
            }
            if ( auto button = new QPushButton( tr("Default folder") ) ) {
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
    const char * t = a.type().name();

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
    if ( auto table = findChild< SampleRunTable * >() )
        table->getContents( t );
    if ( auto edit = findChild< QTextEdit * >() )
        t.description( edit->toHtml().toStdString().c_str() );
}


#include "samplerunwidget.moc"
