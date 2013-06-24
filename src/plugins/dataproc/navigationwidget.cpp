// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include <adcontrols/massspectrum.hpp>
#include <adutils/processeddata.hpp>
#include <adportable/debug.hpp>
#include <portfolio/portfolio.hpp>
#include <portfolio/folder.hpp>
#include <portfolio/folium.hpp>
#include <qtwrapper/qstring.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/modemanager.h>
#include <QLabel>
#include <QTreeView>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QMenu>
#include <QFileDialog>
#include <QDebug>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iomanip>

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

	static QStandardItem * findFolium( QStandardItem * item, const std::wstring& id ) {

		for ( int i = 0; i < item->rowCount(); ++i ) {
			QStandardItem * child = item->child( i );
			QVariant v = child->data( Qt::UserRole + 1 );
			if ( qVariantCanConvert< portfolio::Folium >( v ) ) {
				if ( qVariantValue< portfolio::Folium >( v ).id() == id )
					return child;
			} else if ( qVariantCanConvert< portfolio::Folder >( v ) && child->hasChildren() ) {
				QStandardItem * res = findFolium( child, id );
				if ( res )
					return res;
			}
		}
		return 0;
	}

	static dataproc::Dataprocessor * findDataprocessor( const QModelIndex& index ) {
		QModelIndex parent = index.parent();
		while ( parent.isValid() && ! qVariantCanConvert< dataproc::Dataprocessor * >( parent.data( Qt::UserRole + 1 ) ) )
			parent = parent.parent();
		if ( parent.isValid() )
			return qVariantValue< dataproc::Dataprocessor * >( parent.data( Qt::UserRole + 1 ) );
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

NavigationWidget::~NavigationWidget()
{
    delete pDelegate_;
    delete pModel_;
    delete pTreeView_;
}

NavigationWidget::NavigationWidget(QWidget *parent) : QWidget(parent)
                                                    , pTreeView_( new QTreeView(this) )
                                                    , pModel_( new QStandardItemModel )
                                                    , pDelegate_( new NavigationDelegate ) 
{
    pTreeView_->setModel( pModel_ );
    pTreeView_->setItemDelegate( pDelegate_ );
    setFocusProxy( pTreeView_ );
    initView();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( pTreeView_ );
    layout->setSpacing( 0 );
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );

    for ( auto& it: *SessionManager::instance() )
            handleAddSession( &(it.getDataprocessor()) );

    // connections
    connect( pModel_, SIGNAL( modelReset() ), this, SLOT( initView() ) );

    connect( pTreeView_, SIGNAL(activated(const QModelIndex&)), this, SLOT(handle_activated(const QModelIndex&)));
    connect( pTreeView_, SIGNAL(clicked(const QModelIndex&)), this, SLOT(handle_clicked(const QModelIndex&)));
    connect( pTreeView_, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(handle_doubleClicked(const QModelIndex&)));
    connect( pTreeView_, SIGNAL(entered(const QModelIndex&)), this, SLOT(handle_entered(const QModelIndex&)));

    pTreeView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( pTreeView_, SIGNAL(customContextMenuRequested( QPoint )), this, SLOT( handleContextMenuRequested( QPoint ) ) );

    connect( SessionManager::instance(), SIGNAL( signalAddSession( Dataprocessor* ) ), this, SLOT( handleAddSession( Dataprocessor * ) ) );
	connect( SessionManager::instance(), SIGNAL( signalSessionUpdated( Dataprocessor*, portfolio::Folium& ) ), this, SLOT( handleSessionUpdated( Dataprocessor *, portfolio::Folium& ) ) );

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
NavigationWidget::handleSessionUpdated( Dataprocessor * processor, portfolio::Folium& folium )
{
    QString filename( qtwrapper::qstring::copy( processor->file().filename() ) );

    QStandardItemModel& model = *pModel_;

    QStandardItem * item = StandardItemHelper::findRow( model, processor );
    if ( item ) {
        model.removeRows( 0, item->rowCount(), item->index() );
    
        portfolio::Portfolio portfolio = processor->getPortfolio();
        for ( auto folder: portfolio.folders() )
            PortfolioHelper::appendFolder( *item, folder );
    }
	// set selected
	if ( ( item = StandardItemHelper::findRow( model, processor ) ) ) {
		QStandardItem * leaf = StandardItemHelper::findFolium( item, folium.id() );
		if ( leaf )
			pTreeView_->setCurrentIndex( leaf->index() );
		processor->setCurrentSelection( folium );
	}
}

void
NavigationWidget::handleAddSession( Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );

    QStandardItemModel& model = *pModel_;

    QStandardItem * item = StandardItemHelper::appendRow( model, qVariantFromValue( processor ) );
    item->setEditable( false );
    item->setToolTip( filename );

    portfolio::Portfolio portfolio = processor->getPortfolio();
    
	for ( auto& folder: portfolio.folders() )
        PortfolioHelper::appendFolder( *item, folder );

    Core::ModeManager::instance()->activateMode( dataproc::Constants::C_DATAPROC_MODE );
    
}

void
NavigationWidget::handle_activated( const QModelIndex& index )
{
    qDebug() << "activated: " << index.data( Qt::UserRole + 1 );

    if ( index.isValid() ) {

        QVariant data = index.data( Qt::UserRole + 1 );

		if ( qVariantCanConvert< portfolio::Folder >( data ) ) {
			// folder (Spectra|Chromatograms)
			portfolio::Folder folder = qVariantValue< portfolio::Folder >( data );
			Dataprocessor * processor = StandardItemHelper::findDataprocessor( index );
			processor->setCurrentSelection( folder );

		} else if ( qVariantCanConvert< portfolio::Folium >( data ) ) {

            portfolio::Folium folium = qVariantValue< portfolio::Folium >( data );

			Dataprocessor * processor = StandardItemHelper::findDataprocessor( index );
			if ( processor ) {
				std::string tname = static_cast<boost::any&>( folium ).type().name();
				adportable::debug(__FILE__, __LINE__)
					<< "folium name: '" << folium.name()
					<< "'\tfilename: " << processor->file().filename()
					<< "\tfolium(type=" << tname << ", id=" << folium.id() << ")";
				processor->setCurrentSelection( folium );
			}
        }
    }
}

void
NavigationWidget::handle_clicked( const QModelIndex& index )
{
    handle_activated( index );
}

void
NavigationWidget::handle_doubleClicked( const QModelIndex& index )
{
    (void)index;
}

void
NavigationWidget::handle_entered( const QModelIndex& index )
{
    (void)index;
}

void
NavigationWidget::handle_pressed( const QModelIndex& index )
{
    (void)index;
}

namespace dataproc {

	struct export_spectrum {
		static bool write( std::ostream& o, const adcontrols::MassSpectrum& ms ) {
			const double * masses = ms.getMassArray();
			const double * intens = ms.getIntensityArray();
			for ( size_t n = 0; n < ms.size(); ++n ) {
				o << std::fixed << std::setprecision( 14 ) << *masses++ << ",\t"
					<< std::scientific << std::setprecision(7) << *intens++ << std::endl;
			}
			return true;
		}
	};
}

void
NavigationWidget::handleContextMenuRequested( const QPoint& pos )
{
	QPoint globalPos = pTreeView_->mapToGlobal(pos);
	// QPoint globalPos = this->pTreeView_->viewport()->mapToGlobal(pos);
    // for QAbstractScrollArea and derived classes you would use:
    // QPoint globalPos = myWidget->viewport()->mapToGlobal(pos); 

	QModelIndex index = pTreeView_->currentIndex();
	Dataprocessor * processor = StandardItemHelper::findDataprocessor( index );

	QVariant data = pModel_->data( index, Qt::UserRole + 1 );
	if ( qVariantCanConvert< portfolio::Folium >( data ) ) {
		portfolio::Folium folium = qVariantValue< portfolio::Folium >( data );
		if ( processor && folium.getParentFolder().name() == L"Spectra" ) {
			adutils::MassSpectrumPtr profile = boost::any_cast< adutils::MassSpectrumPtr >( folium );
			adutils::MassSpectrumPtr centroid;

			portfolio::Folio atts = folium.attachments();
			portfolio::Folio::iterator it = portfolio::Folium::find_first_of<adcontrols::MassSpectrumPtr>(atts.begin(), atts.end());

			if ( it != atts.end() ) {
				adutils::MassSpectrumPtr ptr = boost::any_cast< adutils::MassSpectrumPtr >( *it );
				if ( ptr && ptr->isCentroid() )
					centroid = ptr;
			}

			QMenu menu;
			QAction * asProfile = 0; 
			QAction * asCentroid = 0;
			QAction * doNext = 0;
			if ( profile )
				asProfile = menu.addAction( "Save profile spectrum as..." );
			if ( centroid ) {
				asCentroid = menu.addAction( "Save centroid spectrum as..." );
				doNext = menu.addAction( "Show next spectrum" );
			}
			QAction* selectedItem = menu.exec( globalPos );
			if ( selectedItem ) {
				if ( selectedItem == doNext ) {
                    // processor->createChromatograms( *centroid );
                    // targeting
				} else {
					boost::filesystem::path path( processor->file().filename() );
                    while ( ! boost::filesystem::is_directory( path ) )
                        path = path.branch_path();
                    QString dir = qtwrapper::qstring::copy( path.wstring() );
                    QString name = qtwrapper::qstring::copy( folium.name() );
                    QString filename = 
                        QFileDialog::getSaveFileName( this, tr("Save spectrum"), dir, tr("Documents (*.txt)") );
                    boost::filesystem::path dstfile( qtwrapper::wstring::copy( filename ) );
                    boost::filesystem::ofstream of( dstfile );

                    if ( selectedItem == asProfile ) {
                        export_spectrum::write( of, *profile );
                    } else if ( selectedItem == asCentroid ) {
                        export_spectrum::write( of, *centroid );
                    }
                }
			}
		}
	}
}
