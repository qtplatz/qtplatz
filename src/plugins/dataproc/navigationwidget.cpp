//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "navigationwidget.h"
#include "navigationdelegate.h"
#include "dataprocessor.h"
#include "sessionmanager.h"
#include <adcontrols/datafile.h>
#include <portfolio/portfolio.h>
#include <portfolio/folder.h>
#include <portfolio/folium.h>
#include <qtwrapper/qstring.h>
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

    // connections
    connect( pModel_.get(), SIGNAL( modelReset() ), this, SLOT( initView() ) );
    // connect( pTreeView_.get(), SIGNAL(activated(const QModelIndex&)), this, SLOT(openItem(const QModelIndex&)));
    connect( pTreeView_.get(), SIGNAL(activated(const QModelIndex&)), this, SLOT(handle_activated(const QModelIndex&)));
    connect( pTreeView_.get(), SIGNAL(clicked(const QModelIndex&)), this, SLOT(handle_clicked(const QModelIndex&)));
    connect( pTreeView_.get(), SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(handle_doubleClicked(const QModelIndex&)));
    connect( pTreeView_.get(), SIGNAL(entered(const QModelIndex&)), this, SLOT(handle_entered(const QModelIndex&)));
    connect( pTreeView_.get(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(handle_currentChanged(const QModelIndex&, const QModelIndex&)));

    connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) ), this, SLOT( handleSessionAdded( Dataprocessor * ) ) );

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
NavigationWidget::handleSessionAdded( Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );

    QStandardItemModel& model = *pModel_;

    QStandardItem * item = StandardItemHelper::appendRow( model, qVariantFromValue( processor ) );
    item->setEditable( false );
    item->setToolTip( filename );

    /*
    if ( processor->getLCMSDataset() ) {
        QStandardItem * chro = StandardItemHelper::appendRow( *item, "Chromatograms" );
        StandardItemHelper::appendRow( *chro, "TIC" );
        StandardItemHelper::appendRow( *item, "Spectra" );
    }
    */

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

