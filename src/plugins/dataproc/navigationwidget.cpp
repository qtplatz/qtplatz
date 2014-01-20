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
#include <adutils/fsio2.hpp>
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
        // top level row that should be dataset (file on disk)
        QStandardItem * item = new QStandardItem;
        item->setData( qVariantFromValue<T>( value ), Qt::UserRole );
        model.appendRow( item );
        return item;
    }

    template<class T> static QStandardItem * appendRow( QStandardItem& parent, const T& value, bool isCheckable, bool isChecked = false ) {
        QStandardItemModel& model = *parent.model();
		int row = parent.rowCount();
		parent.insertRow( row, new QStandardItem );
		QStandardItem * item = model.itemFromIndex( model.index( row, 0, parent.index() ) );
		//
        if ( isCheckable ) {
			item->setCheckable( true );
			item->setData( isChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
			item->setData( QString::fromStdWString( value.name() ), Qt::EditRole );
			item->setData( qVariantFromValue<T>( value ), Qt::UserRole );
        } else {
			item->setData( QString::fromStdWString( value.name() ), Qt::EditRole );
			item->setData( qVariantFromValue<T>( value ), Qt::UserRole );
		}
		
        return item;
    }

    template<class T> static QStandardItem * findRow( QStandardItemModel& model, const T& value ) {
        for ( int i = 0; i < model.rowCount(); ++i ) {
            QStandardItem * item = model.item( i );
            QVariant v = item->data( Qt::UserRole );
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
			QVariant v = child->data( Qt::UserRole );
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

	static QStandardItem * findFolder( QStandardItem * item, const std::wstring& name ) {

		for ( int i = 0; i < item->rowCount(); ++i ) {
			QStandardItem * child = item->child( i );
			QVariant v = child->data( Qt::UserRole );

			if ( qVariantCanConvert< portfolio::Folder >( v ) ) {
                if ( qVariantValue< portfolio::Folder >( v ).name() == name ) {
                    return child;
                } else if ( child->hasChildren() ) {
                    if ( QStandardItem * res = findFolder( child, name ) )
                        return res;
                }
			}
		}
		return 0;
	}

	static dataproc::Dataprocessor * findDataprocessor( const QModelIndex& index ) {
		QModelIndex parent = index.parent();
		while ( parent.isValid() && ! qVariantCanConvert< dataproc::Dataprocessor * >( parent.data( Qt::UserRole ) ) )
			parent = parent.parent();
		if ( parent.isValid() )
			return qVariantValue< dataproc::Dataprocessor * >( parent.data( Qt::UserRole ) );
		return 0;
	}
};


class PortfolioHelper {

public:

    static void appendFolium( QStandardItem& parent, portfolio::Folium& folium ) {
		QStandardItem * item = StandardItemHelper::appendRow( parent, folium, true, folium.attribute( L"isChecked" ) == L"true" );
		item->setToolTip( QString::fromStdWString( folium.name() ) );
    }

    static void appendFolder( QStandardItem& parent, portfolio::Folder& folder ) {

        QStandardItem * item = StandardItemHelper::appendRow( parent, folder, false );
        item->setEditable( false );

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

    if ( SessionManager * mgr = SessionManager::instance() ) {
        connect( mgr, SIGNAL( onSessionRemoved( Dataprocessor* ) ), this, SLOT( handleRemoveSession( Dataprocessor * ) ) );
        connect( mgr, SIGNAL( signalAddSession( Dataprocessor* ) ), this, SLOT( handleAddSession( Dataprocessor * ) ) );
        // connect( mgr, SIGNAL( signalSessionUpdated( Dataprocessor*, portfolio::Folium ) )
        //          , this, SLOT( handleSessionUpdated( Dataprocessor *, portfolio::Folium ) ) );
        connect( mgr, SIGNAL( onSessionUpdated( Dataprocessor*, const QString& ) ), this, SLOT( handleSessionUpdated( Dataprocessor *, const QString& ) ) );
        connect( mgr, SIGNAL( onFolderChanged( Dataprocessor*, const QString& ) )
                 , this, SLOT( handleFolderChanged( Dataprocessor *, const QString& ) ) );
        
        connect( pModel_, SIGNAL( itemChanged( QStandardItem *) ), this, SLOT( handleItemChanged( QStandardItem * ) ) );
    }

    setAutoSynchronization(true);
    setAcceptDrops( true );
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
NavigationWidget::handleItemChanged( QStandardItem * item )
{
    // handle checkbox on tree item
    QVariant data = item->data( Qt::UserRole );

    if ( qVariantCanConvert< portfolio::Folium >( data ) ) {

		Qt::CheckState state = static_cast< Qt::CheckState >( item->data( Qt::CheckStateRole ).toUInt() );
        portfolio::Folium folium = qVariantValue< portfolio::Folium >( data );
        folium.setAttribute( L"isChecked", state == Qt::Checked ? L"true" : L"false" );

		if ( Dataprocessor * dp = StandardItemHelper::findDataprocessor( item->index() ) )
            SessionManager::instance()->checkStateChanged( dp, folium, state == Qt::Checked );

    }
    
}

void
NavigationWidget::invalidateSession( Dataprocessor * processor )
{
    QString filename( QString::fromStdWString( processor->file().filename() ) );
    QStandardItemModel& model = *pModel_;

    if ( QStandardItem * item = StandardItemHelper::findRow( model, processor ) ) {
        model.removeRows( 0, item->rowCount(), item->index() );
        portfolio::Portfolio portfolio = processor->getPortfolio();
        for ( auto folder: portfolio.folders() )
            PortfolioHelper::appendFolder( *item, folder );
    }
}

void
NavigationWidget::handleFolderChanged( Dataprocessor * processor, const QString& foldername )
{
    portfolio::Portfolio portfolio = processor->getPortfolio();
    portfolio::Folder folder = portfolio.findFolder( foldername.toStdWString() );
    portfolio::Folio folio = folder.folio();

    if ( QStandardItem * procItem = StandardItemHelper::findRow< Dataprocessor * >( *pModel_, processor ) ) {
        if ( QStandardItem * folderItem = StandardItemHelper::findFolder( procItem, foldername.toStdWString() ) ) {
            for ( auto folium: folio ) {
                if ( QStandardItem * item = StandardItemHelper::findFolium( procItem, folium.id() ) ) {
                    item->setData( qVariantFromValue< portfolio::Folium >( folium ), Qt::UserRole );
                } else {
                    PortfolioHelper::appendFolium( *folderItem, folium );
                }
            }
        }
    }
}

void
NavigationWidget::handleSessionUpdated( Dataprocessor * processor, const QString& foliumId )
{
    portfolio::Folium folium = processor->portfolio().findFolium( foliumId.toStdWString() );
    handleSessionUpdated( processor, folium );
}

void
NavigationWidget::handleSessionUpdated( Dataprocessor * processor, portfolio::Folium& folium )
{
    QString filename( qtwrapper::qstring::copy( processor->file().filename() ) );

    QStandardItemModel& model = *pModel_;

    if ( QStandardItem * processorItem = StandardItemHelper::findRow< Dataprocessor * >( model, processor ) ) {

        if ( QStandardItem * folderItem
             = StandardItemHelper::findFolder( processorItem, folium.getParentFolder().name() ) ) {
            
            if ( QStandardItem * item = StandardItemHelper::findFolium( processorItem, folium.id() ) ) {
                // replace existing
                item->setData( qVariantFromValue< portfolio::Folium >( folium ), Qt::UserRole );
            } else {
                PortfolioHelper::appendFolium( *folderItem, folium );
            }

        } else {
            portfolio::Folder parent = folium.getParentFolder();
            PortfolioHelper::appendFolder( *processorItem, parent );
        }
    }

	// set selected
	if ( QStandardItem * item = StandardItemHelper::findRow( model, processor ) ) {
        if ( QStandardItem * leaf = StandardItemHelper::findFolium( item, folium.id() ) )
            pTreeView_->setCurrentIndex( leaf->index() );
        processor->setCurrentSelection( folium );
    }

}

void
NavigationWidget::handleRemoveSession( Dataprocessor * processor )
{
    QStandardItemModel& model = *pModel_;

    if ( QStandardItem * item = StandardItemHelper::findRow( model, processor ) ) {
		model.removeRow( item->row() );
    }
}

void
NavigationWidget::handleAddSession( Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );

    QStandardItemModel& model = *pModel_;

    QStandardItem * item = StandardItemHelper::appendRow( model, processor );
    item->setEditable( false );
    item->setToolTip( filename );

    portfolio::Portfolio portfolio = processor->getPortfolio();
    
	for ( auto& folder: portfolio.folders() )
        PortfolioHelper::appendFolder( *item, folder );

	pTreeView_->expand( item->index() );
	 // expand second levels (Chromatograms|Spectra|MSCalibration etc.)
	for ( int i = 0; i < item->rowCount(); ++i)
        pTreeView_->expand( model.index( i, 0, item->index()) );


    Core::ModeManager::instance()->activateMode( dataproc::Constants::C_DATAPROC_MODE );
    
}

void
NavigationWidget::handle_activated( const QModelIndex& index )
{
    if ( index.isValid() ) {

        QVariant data = index.data( Qt::UserRole );

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
				// adportable::debug(__FILE__, __LINE__)
				// 	<< "folium name: '" << folium.name()
				// 	<< "'\tfilename: " << processor->file().filename()
				// 	<< "\tfolium(type=" << tname << ", id=" << folium.id() << ")";
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
		static bool write( std::ostream& o, const adcontrols::MassSpectrum& _ms ) {
			adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( _ms );
			for ( auto& ms : segments ) { // size_t n = 0; n < segments.size(); ++n ) {
				//const adcontrols::MassSpectrum& ms = segments[ n ];
				for ( size_t n = 0; n < ms.size(); ++n ) {
					o << std::scientific << std::setprecision( 15 ) << ms.getTime( n ) << ",\t"
						<< std::fixed << std::setprecision( 13 ) << ms.getMass( n ) << ",\t"
						<< std::scientific << std::setprecision(7) << ms.getIntensity( n ) << std::endl;
				}
			}
			return true;
		}
	};
}

void
NavigationWidget::handleContextMenuRequested( const QPoint& pos )
{
	QPoint globalPos = pTreeView_->mapToGlobal(pos);
	QModelIndex index = pTreeView_->currentIndex();

    if ( index.isValid() ) {

        if ( Dataprocessor * processor = StandardItemHelper::findDataprocessor( index ) ) {

            QVariant data = pModel_->data( index, Qt::UserRole );
            if ( qVariantCanConvert< portfolio::Folium >( data ) ) {
            
                portfolio::Folium folium = qVariantValue< portfolio::Folium >( data );
            
                if ( processor && 
                     ( ( folium.getParentFolder().name() == L"Spectra" ) ||
                       ( folium.getParentFolder().name() == L"MSCalibration" ) ) ) {

					if ( folium.empty() )
						processor->fetch( folium );
                    
                    bool isSpectrum = portfolio::is_type< adutils::MassSpectrumPtr >( folium );
                    portfolio::Folio atts = folium.attachments();
                    auto it = std::find_if( atts.begin(), atts.end(), []( const portfolio::Folium& a ){
                            return a.name() == Constants::F_CENTROID_SPECTRUM;
                        });
                    bool hasCentroid = it != atts.end();
                
                    QMenu menu;
                    QAction * asProfile = 0; 
                    QAction * asCentroid = 0;
                    QAction * doCalibration = 0;
                    QAction * removeChecked = 0;
                    asProfile = menu.addAction( "Save profile spectrum as..." );
                    asCentroid = menu.addAction( "Save centroid spectrum as..." );
                    if ( ! isSpectrum )
                        asProfile->setEnabled( false );

                    if ( ! hasCentroid )
                        asCentroid->setEnabled( false );

                    doCalibration = menu.addAction( "Send checked spectra to calibration folder" );
					menu.addSeparator();
                    removeChecked = menu.addAction( "Remove unchecked items" );

                    QAction* selectedItem = menu.exec( globalPos );
                    if ( selectedItem ) {
                        if ( selectedItem == doCalibration ) {

                            for ( auto& session: *SessionManager::instance() )
								processor->sendCheckedSpectraToCalibration( session.processor() );

                        } else if ( selectedItem == removeChecked ) {
                            processor->removeCheckedItems();
                            invalidateSession( processor );
                        } else {
                            boost::filesystem::path path( processor->file().filename() );
                            while ( ! boost::filesystem::is_directory( path ) )
                                path = path.branch_path();
                            QString dir = qtwrapper::qstring::copy( path.wstring() );
                            QString name = qtwrapper::qstring::copy( folium.name() );
                            QString filename = 
                                QFileDialog::getSaveFileName( this, tr("Save spectrum")
                                                              , dir
                                                              , tr("qtplatz (*.adfs);;Text files (*.txt)") );

                            boost::filesystem::path dstfile( qtwrapper::wstring::copy( filename ) );

                            if ( dstfile.extension() == ".adfs" ) {
                                
                                adutils::fsio2::appendOnFile( dstfile.wstring(), folium, processor->file() );
                                
                            } else {
                                boost::filesystem::ofstream of( dstfile );
                                if ( selectedItem == asProfile ) {
                                    auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium );
                                    export_spectrum::write( of, *profile );
                                } else if ( selectedItem == asCentroid ) {
                                    auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *it );
                                    export_spectrum::write( of, *centroid );
                                }
                            }
                        }
                    }
                }
                if ( processor && folium.getParentFolder().name() == L"Chromatograms" ) {
                    QMenu menu;
                    QAction * doSpectrogram = 0; 
                    doSpectrogram = menu.addAction( "Creatge Spectrogram" );
                    if ( QAction* selectedItem = menu.exec( globalPos ) ) {
                        if ( doSpectrogram == selectedItem ) {
                            processor->createSpectrogram();
                        }
                    }
                }
            }
        }
    }
}
