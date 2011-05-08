// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "navigationwidget.hpp"
#include "navigationdelegate.hpp"
#include "dataprocessor.hpp"
#include "sessionmanager.hpp"
#include <adcontrols/datafile.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <qtwrapper/qstring.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/filemanager.h>
#include <QLabel>
#include <QTreeView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QDebug>

class StandardItemHelper {
public:
    StandardItemHelper();

    template<class T> static QStandardItem * appendRow( QStandardItemModel& model, const T& value ) {
        QStandardItem * item = new QStandardItem;
        item->setData( qVariantFromValue<T>( value ) );
        model.appendRow( item );
        return item;
    }

    template<class T> static QStandardItem * appendRow( QStandardItem& parent, const T& value ) {
        QStandardItem * item = new QStandardItem;
        item->setData( qVariantFromValue<T>( value ) );
        parent.appendRow( item );
        return item;
    }

    template<class T> static QStandardItem * findRow( QStandardItemModel& model, const T& value ) {
        for ( int i = 0; i < model.rowCount(); ++i ) {
            QStandardItem * item = model.item( i );
            QVariant v = item->data( Qt::UserRole + 1 );
            if ( qVariantCanConvert< T >( v ) ) {
                if ( qVariantValue< T >( v ) == value )
                    return item;
            }
        }
        return 0;
    }
};


class PortfolioHelper {
public:
    static void appendFolium( QStandardItem& parent, portfolio::Folium& folium ) {
        StandardItemHelper::appendRow( parent, folium );
    }

    static void appendFolder( QStandardItem& parent, portfolio::Folder& folder ) {

        QStandardItem * item = StandardItemHelper::appendRow( parent, folder );

        std::vector< portfolio::Folder > folders = folder.folders();
        for ( std::vector< portfolio::Folder >::iterator it = folders.begin(); it != folders.end(); ++it )
            appendFolder( *item, *it );

        portfolio::Folio folio = folder.folio();
        for ( portfolio::Folio::iterator it = folio.begin(); it != folio.end(); ++it ) 
            appendFolium( *item, *it );
    }
};


using namespace dataproc;

NavigationWidget::NavigationWidget(QWidget *parent) : QWidget(parent)
                                                    , pTreeView_( new QTreeView(this) )
                                                    , pModel_( new QStandardItemModel )
                                                    , pDelegate_( new NavigationDelegate ) 
{
    pTreeView_->setModel( pModel_.get() );
    pTreeView_->setItemDelegate( pDelegate_.get() );
    setFocusProxy( pTreeView_.get() );
    initView();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( pTreeView_.get() );
    layout->setSpacing( 0 );
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );

    for ( SessionManager::vector_type::iterator it = 
        SessionManager::instance()->begin(); it != SessionManager::instance()->end(); ++it ) {
            handleSessionAdded( &(it->getDataprocessor()) );
    }

    // connections
    connect( pModel_.get(), SIGNAL( modelReset() ), this, SLOT( initView() ) );
    // connect( pTreeView_.get(), SIGNAL(activated(const QModelIndex&)), this, SLOT(openItem(const QModelIndex&)));
    connect( pTreeView_.get(), SIGNAL(activated(const QModelIndex&)), this, SLOT(handle_activated(const QModelIndex&)));
    connect( pTreeView_.get(), SIGNAL(clicked(const QModelIndex&)), this, SLOT(handle_clicked(const QModelIndex&)));
    connect( pTreeView_.get(), SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(handle_doubleClicked(const QModelIndex&)));
    connect( pTreeView_.get(), SIGNAL(entered(const QModelIndex&)), this, SLOT(handle_entered(const QModelIndex&)));

    connect( pTreeView_.get(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(handle_currentChanged(const QModelIndex&, const QModelIndex&)));

    pTreeView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( pTreeView_.get(), SIGNAL(customContextMenuRequested( QPoint )), this, SLOT( handleContextMenuRequested( QPoint ) ) );

    connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) ), this, SLOT( handleSessionAdded( Dataprocessor * ) ) );
    connect( SessionManager::instance(), SIGNAL( signalSessionUpdated( Dataprocessor* ) ), this, SLOT( handleSessionUpdated( Dataprocessor * ) ) );



    setAutoSynchronization(true);
}

void
NavigationWidget::setAutoSynchronization( bool sync )
{
    if ( autoSync_ == sync )
        return;
    autoSync_ = sync;

    Core::FileManager *fileManager = Core::ICore::instance()->fileManager();
    if ( autoSync_ ) {
        // connect(fileManager, SIGNAL(currentFileChanged(QString)), this, SLOT(setCurrentFile(QString)));
        // setCurrentFile(fileManager->currentFile());
    } else {
        disconnect(fileManager, SIGNAL(currentFileChanged(QString)), this, SLOT(setCurrentFile(QString)));
    }
}

bool
NavigationWidget::autoSyncronization() const
{
    return autoSync_;
}

void
NavigationWidget::toggleAutoSynchronization()
{
    setAutoSynchronization( ! autoSync_ );
}

void
NavigationWidget::initView()
{
    QStandardItemModel& model = *pModel_;
    QTreeView& view = *pTreeView_;

    view.setHeaderHidden( true );

    QModelIndex sessionIndex = model.index(0, 0);

    // hide root folder
    view.setRootIndex(sessionIndex);

    while ( model.canFetchMore(sessionIndex))
        model.fetchMore(sessionIndex);

    // expand top level projects
    for ( int i = 0; i < pModel_->rowCount(sessionIndex); ++i)
        view.expand( model.index(i, 0, sessionIndex));

    view.setMouseTracking( true );

    // setCurrentItem(m_explorer->currentNode(), m_explorer->currentProject());
}

void
NavigationWidget::handleSessionUpdated( Dataprocessor * processor )
{
    QString filename( qtwrapper::qstring::copy( processor->file().filename() ) );

    QStandardItemModel& model = *pModel_;

    QStandardItem * item = StandardItemHelper::findRow( model, processor );
    if ( item ) {
        model.removeRows( 0, item->rowCount(), item->index() );
    
        portfolio::Portfolio portfolio = processor->getPortfolio();
        std::vector< portfolio::Folder > folders = portfolio.folders();
        for ( std::vector< portfolio::Folder >::iterator it = folders.begin(); it != folders.end(); ++it )
            PortfolioHelper::appendFolder( *item, *it );
    }
}

void
NavigationWidget::handleSessionAdded( Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );

    QStandardItemModel& model = *pModel_;

    QStandardItem * item = StandardItemHelper::appendRow( model, qVariantFromValue( processor ) );
    item->setEditable( false );
    item->setToolTip( filename );

    portfolio::Portfolio portfolio = processor->getPortfolio();
    
    std::vector< portfolio::Folder > folders = portfolio.folders();

    for ( std::vector< portfolio::Folder >::iterator it = folders.begin(); it != folders.end(); ++it ) {
        PortfolioHelper::appendFolder( *item, *it );
    }

}

void
NavigationWidget::handle_activated( const QModelIndex& index )
{
    qDebug() << "activated: " << index.data( Qt::UserRole + 1 );

    if ( index.isValid() ) {
        QVariant data = index.data( Qt::UserRole + 1 );
        if ( qVariantCanConvert< portfolio::Folium >( data ) ) {

            portfolio::Folium folium = qVariantValue< portfolio::Folium >( data );

            qDebug() << qtwrapper::qstring::copy(folium.name());

            Dataprocessor * processor = 0;
            QModelIndex& parent = index.parent();
            while ( parent.isValid() && ! qVariantCanConvert< Dataprocessor * >( parent.data( Qt::UserRole + 1 ) ) )
                parent = parent.parent();

            if ( parent.isValid() ) {
                if ( processor = qVariantValue< Dataprocessor * >( parent.data( Qt::UserRole + 1 ) ) ) {
                    qDebug() << "filename: " << qtwrapper::qstring( processor->file().filename() );
                    processor->setCurrentSelection( folium );
                }
            }
                        
        }
    }
}

void
NavigationWidget::handle_clicked( const QModelIndex& index )
{
    qDebug() << "clicked: " << index.data( Qt::UserRole + 1 );
    handle_activated( index );
}

void
NavigationWidget::handle_doubleClicked( const QModelIndex& index )
{
    qDebug() << "doubleClicked: " << index.data( Qt::UserRole + 1 );
}

void
NavigationWidget::handle_entered( const QModelIndex& index )
{
    qDebug() << "entered: " << index.data( Qt::UserRole + 1 );
}

void
NavigationWidget::handle_pressed( const QModelIndex& index )
{
    qDebug() << "pressed: " << index.data( Qt::UserRole + 1 );
}

void
NavigationWidget::handleContextMenuRequested( const QPoint& )
{
}