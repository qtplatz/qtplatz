// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
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
#include "dataprocplugin.hpp"
#include "dataprocessor.hpp"
#include "document.hpp"
#include "sessionmanager.hpp"
#include "actionmanager.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adutils/processeddata.hpp>
#include <adutils/fsio2.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <qtwrapper/qstring.hpp>
#include <qtwrapper/qfiledialog.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/modemanager.h>
#include <QDataStream>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iomanip>
#include <array>

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
            if (v.canConvert< T >() ) {
                if ( v.value< T >() == value )
                    return item;
            }
        }
        return 0;
    }

	static QStandardItem * findFolium( QStandardItem * item, const std::wstring& id ) {

		for ( int i = 0; i < item->rowCount(); ++i ) {
			QStandardItem * child = item->child( i );
			QVariant v = child->data( Qt::UserRole );
			if ( v.canConvert< portfolio::Folium >() ) {
                if ( v.value< portfolio::Folium >().id() == id )
					return child;
			} else if ( v.canConvert< portfolio::Folder >() && child->hasChildren() ) {
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

			if ( v.canConvert< portfolio::Folder >() ) {
                if ( v.value< portfolio::Folder >().name() == name ) {
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
        while ( parent.isValid() && !parent.data( Qt::UserRole ).canConvert< dataproc::Dataprocessor * >() )
			parent = parent.parent();
		if ( parent.isValid() )
            return parent.data( Qt::UserRole ).value< dataproc::Dataprocessor * >();
		return 0;
	}
};


class PortfolioHelper {

public:

    static void appendAttachment( QStandardItem& parent, const portfolio::Folium& folium ) {
		QStandardItem * item = StandardItemHelper::appendRow( parent, folium, false );
		item->setToolTip( QString::fromStdWString( folium.name() ) );
    }
    
    static void appendFolium( QStandardItem& parent, portfolio::Folium& folium ) {
        
		QStandardItem * item = StandardItemHelper::appendRow( parent, folium, true, folium.attribute( L"isChecked" ) == L"true" );
		item->setToolTip( QString::fromStdWString( folium.name() ) );
        
        auto atts = folium.attachments();
        for ( auto& att: atts )
            appendAttachment( *item, att );
    }

    static void appendFolder( QStandardItem& parent, portfolio::Folder& folder ) {

        QStandardItem * item = StandardItemHelper::appendRow( parent, folder, false );
        item->setEditable( false );

        std::vector< portfolio::Folder > folders = folder.folders();
        for ( std::vector< portfolio::Folder >::iterator it = folders.begin(); it != folders.end(); ++it )
            appendFolder( *item, *it );

        portfolio::Folio folio = folder.folio();
        for ( portfolio::Folio::iterator it = folio.begin(); it != folio.end(); ++it ) {
            appendFolium( *item, *it );
        }
    }
};

QDataStream &operator<<(QDataStream& out, const portfolio::Folium& folium )
{
    out << QString::fromStdWString( folium.id() );
    return out;
}

QDataStream &operator>>(QDataStream& in, portfolio::Folium& folium )
{
    QString id;
    in >> id;
    folium.id( id.toStdWString() );
    return in;
}

QDataStream &operator<<(QDataStream& out, const portfolio::Folder& folder )
{
    out << QString::fromStdWString( folder.id() );
    return out;
}

QDataStream &operator>>(QDataStream& in, portfolio::Folder& folder )
{
    QString id;
    in >> id;
    folder.id( id.toStdWString() );
    return in;
}

using namespace dataproc;

NavigationWidget::~NavigationWidget()
{
    delete pDelegate_;
    delete pModel_;
    delete pTreeView_;
}

NavigationWidget::NavigationWidget(QWidget *parent) : QWidget(parent)
                                                    , pTreeView_( new QTreeView( this ) )
                                                    , pModel_( new QStandardItemModel )
                                                    , pDelegate_( new NavigationDelegate ) 
{
    pTreeView_->setModel( pModel_ );
    pTreeView_->setItemDelegate( pDelegate_ );
	pTreeView_->setDragEnabled( true );
    pTreeView_->setTextElideMode(Qt::ElideMiddle);

    setStyleSheet(
        "QTreeView {"
        " show-decoration-selected: 1;"
        " font-size: 9pt;"
        "}"
        "QTreeView::item:selected {"
        " border: 1px solid #567dbc;"
        "}"        
        "QTreeView::item:open {"
        " background-color: #c5ebfb;"
        " color: blue;"
        "}"
#if ! defined Q_OS_MAC
        "QTreeView::branch {"
        " background: palette(base);"
        "}"
        "QTreeView::branch:open {"
        " image: url(:/dataproc/image/control-270-small.png);"
        "}"
        "QTreeView::branch:closed:has-children {"
        " image: url(:/dataproc/image/control-000-small.png);"
        "}"
#endif        
        );


    // pTreeView_->setDragDropMode( QAbstractItemView::DragOnly );

    qRegisterMetaTypeStreamOperators< portfolio::Folium >( "portfolio::Folium" );
    qRegisterMetaTypeStreamOperators< portfolio::Folder >( "portfolio::Folder" );

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

    connect( pTreeView_, &QTreeView::activated, this, &NavigationWidget::handle_activated );
    connect( pTreeView_, &QTreeView::clicked, this, &NavigationWidget::handle_clicked );
    connect( pTreeView_, &QTreeView::doubleClicked, this, &NavigationWidget::handle_doubleClicked );
    connect( pTreeView_, &QTreeView::entered, this, &NavigationWidget::handle_entered );

    pTreeView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( pTreeView_, &QTreeView::customContextMenuRequested, this, &NavigationWidget::handleContextMenuRequested );

    if ( SessionManager * mgr = SessionManager::instance() ) {

        connect( mgr, &SessionManager::onSessionRemoved, this, &NavigationWidget::handleRemoveSession );

        connect( mgr, &SessionManager::signalAddSession, this, &NavigationWidget::handleAddSession );

        connect( mgr, &SessionManager::onSessionUpdated, this, [&] ( Dataprocessor* dp, const QString& id ){ handleSessionUpdated( dp, id ); } );

        connect( mgr, &SessionManager::onFolderChanged, this, &NavigationWidget::handleFolderChanged );
        
        connect( pModel_, &QStandardItemModel::itemChanged, this, &NavigationWidget::handleItemChanged );

        connect( mgr, &SessionManager::foliumChanged, this, &NavigationWidget::handleFoliumChanged );
    }

    setAutoSynchronization(true);
    setAcceptDrops( true );

    if ( auto am = DataprocPlugin::instance()->actionManager() ) {
        am->connect_navigation_pointer( this );
    }
}

void
NavigationWidget::setAutoSynchronization( bool sync )
{
    if ( autoSync_ == sync )
        return;
    autoSync_ = sync;
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

    if ( data.canConvert< portfolio::Folium >() ) {

		Qt::CheckState state = static_cast< Qt::CheckState >( item->data( Qt::CheckStateRole ).toUInt() );
        portfolio::Folium folium = data.value< portfolio::Folium >();
        folium.setAttribute( L"isChecked", state == Qt::Checked ? L"true" : L"false" );

		if ( Dataprocessor * dp = StandardItemHelper::findDataprocessor( item->index() ) )
            SessionManager::instance()->checkStateChanged( dp, folium, state == Qt::Checked );

    }
    
}

void
NavigationWidget::invalidateSession( Dataprocessor * processor )
{
    QString filename( processor->qfilename() );
    QStandardItemModel& model = *pModel_;

    if ( QStandardItem * item = StandardItemHelper::findRow( model, processor ) ) {
        model.removeRows( 0, item->rowCount(), item->index() );
        portfolio::Portfolio portfolio = processor->getPortfolio();
        for ( auto folder: portfolio.folders() )
            PortfolioHelper::appendFolder( *item, folder );
    }
}

// add child node when process applied (such as Centroid)
// this is responcible to add/redraw attributes under specified folium
void
NavigationWidget::handleFoliumChanged( Dataprocessor * processor, const portfolio::Folium& folium )
{
    ADDEBUG() << "handleFoliumChanged: " << folium.name();

    if ( auto top = StandardItemHelper::findRow< Dataprocessor * >( *pModel_, processor ) ) {
        if ( auto folder = StandardItemHelper::findFolder( top, folium.parentFolder().name() ) ) {
            if ( auto item = StandardItemHelper::findFolium( folder, folium.id() ) ) {
                item->setData( qVariantFromValue< portfolio::Folium >( folium ), Qt::UserRole );
                for ( auto& att: folium.attachments() ) {
                    if ( StandardItemHelper::findFolium( item, att.id() ) == nullptr )
                        PortfolioHelper::appendAttachment( *item, att );
                }
            }
        }
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
    QString filename = processor->qfilename();

    QStandardItemModel& model = *pModel_;

    if ( QStandardItem * processorItem = StandardItemHelper::findRow< Dataprocessor * >( model, processor ) ) {

        if ( QStandardItem * folderItem
             = StandardItemHelper::findFolder( processorItem, folium.parentFolder().name() ) ) {
            
            if ( QStandardItem * item = StandardItemHelper::findFolium( processorItem, folium.id() ) ) {
                // replace existing
                item->setData( qVariantFromValue< portfolio::Folium >( folium ), Qt::UserRole );
            } else {
                PortfolioHelper::appendFolium( *folderItem, folium );
            }

        } else {
            portfolio::Folder parent = folium.parentFolder();
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
    adcontrols::datafile * file = processor->file();
    QString filename = processor->qfilename();

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
}

void
NavigationWidget::handle_activated( const QModelIndex& index )
{
    if ( index.isValid() ) {

        QVariant data = index.data( Qt::UserRole );

		if ( data.canConvert< portfolio::Folder >() ) {
			// folder (Spectra|Chromatograms)
            qtwrapper::waitCursor wait;
			portfolio::Folder folder = data.value< portfolio::Folder >();
			Dataprocessor * processor = StandardItemHelper::findDataprocessor( index );
			processor->setCurrentSelection( folder );

		} else if ( data.canConvert< portfolio::Folium >() ) {

            portfolio::Folium folium = data.value< portfolio::Folium >();

			Dataprocessor * processor = StandardItemHelper::findDataprocessor( index );
			if ( processor ) {
                qtwrapper::waitCursor wait;
				std::string tname = static_cast<boost::any&>( folium ).type().name();
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

	struct export_chromatogram {
		static bool write( std::ostream& o, const adcontrols::Chromatogram& c ) {
            for ( size_t n = 0; n < c.size(); ++n ) {
                o << std::scientific << std::setprecision( 15 ) << c.time( n )
                    << "\t"
                    << std::fixed << std::setprecision( 13 ) << c.intensity( n ) << std::endl;
            }
			return true;
		}
	};

    enum ActionType { checkAll, unCheckAll, asProfile, asCentroid, doCalibration, removedChecked, asDFTProfile };

    struct CheckAllFunctor {
        bool check;
        QStandardItemModel& model;
        QModelIndex index;
        CheckAllFunctor( bool f, QStandardItemModel& m, QModelIndex& idx ) : check( f ), model( m ), index( idx ) {};
        void operator()() {
            auto parent = model.itemFromIndex( index ); // ex. Spectra
            for ( int row = 0; row < parent->rowCount(); ++row ) {
                if ( auto item = model.itemFromIndex( model.index( row, 0, parent->index() ) ) ) {
                    if ( item->isCheckable() )
                        item->setCheckState( check ? Qt::Checked : Qt::Unchecked );
                }
            }
        }
    };

    struct SaveSpectrumAs {
        ActionType idAction;
        portfolio::Folium folium;
        Dataprocessor * processor;
        SaveSpectrumAs( ActionType id, portfolio::Folium& f, Dataprocessor * p ) : idAction( id ), folium( f ), processor( p ) {}

        void operator()() {

            boost::filesystem::path path( processor->file()->filename() );
            std::wstring defaultname = path.stem().wstring() + L"_" + folium.name();
            while ( !boost::filesystem::is_directory( path ) )
                path = path.branch_path();
            
            QString filename = qtwrapper::QFileDialog::getSaveFileName( 0
                                                                        , QObject::tr( "Save spectrum" )
                                                                        , QString::fromStdWString( path.wstring() )
                                                                        , QString::fromStdWString( folium.name() ) 
                                                                        , QObject::tr( "qtplatz (*.adfs);;Text files (*.txt)" ) );
            
            boost::filesystem::path dstfile( filename.toStdWString() );
            
            if ( dstfile.extension() == ".adfs" ) {
                adutils::fsio2::appendOnFile( dstfile.wstring(), folium, *processor->file() );
            } else {
                boost::filesystem::ofstream of( dstfile );

                auto ms = portfolio::get< adcontrols::MassSpectrumPtr >( folium );
                export_spectrum::write( of, *ms );
            }
        }        
    };

    struct SaveChromatogramAs {
        portfolio::Folium folium;
        Dataprocessor * processor;
        
        SaveChromatogramAs( portfolio::Folium& f, Dataprocessor * p ) : folium( f ), processor( p )
            {}
        void operator()() {
            
            boost::filesystem::path path( processor->file()->filename() );
            std::wstring defaultname = path.stem().wstring() + L"_" + folium.name();
            while ( !boost::filesystem::is_directory( path ) )
                path = path.branch_path();

            QString filename = qtwrapper::QFileDialog::getSaveFileName( 0
                                                                        , QObject::tr( "Save Chromatogarm" )
                                                                        , QString::fromStdWString( path.wstring() )
                                                                        , QString::fromStdWString( folium.name() )
                                                                        , QObject::tr( "Text files (*.txt);;qtplatz (*.adfs)" ) );
            
            boost::filesystem::path dstfile( filename.toStdWString() );
            
            if ( dstfile.extension() == ".adfs" ) {
                adutils::fsio2::appendOnFile( dstfile.wstring(), folium, *processor->file() );
            } else {
                boost::filesystem::ofstream of( dstfile );
                if ( auto c = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( folium ) )
                    export_chromatogram::write( of, *c );
            }
        }        
    };
    
    struct CalibrationAction {
        Dataprocessor * processor;
        CalibrationAction( Dataprocessor * p ) : processor( p ) {}

        void operator()() {
            for ( auto& session : *SessionManager::instance() )
                processor->sendCheckedSpectraToCalibration( session.processor() );
        }
    };

    struct BackgroundSubtraction {
        portfolio::Folium background;
        portfolio::Folium foreground;
        Dataprocessor * processor;
        BackgroundSubtraction( portfolio::Folium& back, portfolio::Folium& fore, Dataprocessor * p ) : background( back ), foreground( fore ), processor( p ) {}
        void operator()() {
            processor->subtract( foreground, background );
        }
    };
}

void
NavigationWidget::handleContextMenuRequested( const QPoint& pos )
{
	QPoint globalPos = pTreeView_->mapToGlobal(pos);
	QModelIndex index = pTreeView_->currentIndex();

    QMenu menu;

    if ( index.isValid() ) {

        portfolio::Folium active_folium;
        QString active_spectrum;
        if ( auto activeProcessor = SessionManager::instance()->getActiveDataprocessor() ) {
            if ( (active_folium = activeProcessor->currentSelection()) ) {
                if ( active_folium.parentFolder().name() == L"Spectra" )
                    active_spectrum = QString::fromStdWString( active_folium.name() );
            }
        }
        
        if ( Dataprocessor * processor = StandardItemHelper::findDataprocessor( index ) ) {
            // this indicates menu requested on folium|folder node

            QVariant data = pModel_->data( index, Qt::UserRole );

            if ( data.canConvert< portfolio::Folder >() ) {

                if ( auto folder = data.value< portfolio::Folder >() ) {
                    menu.addAction( QString( tr("Uncheck all for %1") ).arg( index.data( Qt::EditRole ).toString() ), CheckAllFunctor( false, *pModel_, index ) );
                    menu.addAction( QString( tr("Check all for %1") ).arg( index.data( Qt::EditRole ).toString() ), CheckAllFunctor( true, *pModel_, index ) );
                }
                
            } else if ( data.canConvert< portfolio::Folium >() ) { // an item of [Spectrum|Chrmatogram] selected

                portfolio::Folium folium = data.value< portfolio::Folium >();
                
                if ( (folium.parentFolder().name() == L"Spectra") ||
                     (folium.parentFolder().name() == L"MSCalibration") )  {

                    QString selected_spectrum = QString::fromStdWString( folium.name() );

                    if ( folium.empty() )
                        processor->fetch( folium );

                    if ( bool isSpectrum = portfolio::is_type< adutils::MassSpectrumPtr >( folium ) ) {
                        portfolio::Folio atts = folium.attachments();
                        auto itCentroid = std::find_if( atts.begin(), atts.end(), [] ( const portfolio::Folium& a ){
                                return a.name() == Constants::F_CENTROID_SPECTRUM;
                            } );
                        bool hasCentroid = itCentroid != atts.end();

                        auto itFiltered = std::find_if( atts.begin(), atts.end(), [] ( const portfolio::Folium& a ){
                                return a.name() == Constants::F_DFT_FILTERD;
                            } );
                        bool hasFilterd = itFiltered != atts.end();

                        if ( auto a = menu.addAction( tr("Save profile spectrum as..."), SaveSpectrumAs( asProfile, folium, processor ) ) ) 
                            a->setEnabled( isSpectrum );
                        
                        portfolio::Folium centroid = itCentroid != atts.end() ? *itCentroid : portfolio::Folium();

                        if ( auto a = menu.addAction( tr("Save centroid spectrum as..."), SaveSpectrumAs( asCentroid, centroid, processor ) ) )
                            a->setEnabled( hasCentroid );
                        
                        portfolio::Folium filtered = itFiltered != atts.end() ? *itFiltered : portfolio::Folium();
                        if( auto a = menu.addAction( tr("Save DFT filtered spectrum as..."), SaveSpectrumAs( asDFTProfile, filtered, processor ) ) )
                            a->setEnabled( hasFilterd );

                        menu.addAction( tr("Send checked spectra to calibration folder"), CalibrationAction( processor ) );

                        menu.addSeparator();

                        if ( auto a = menu.addAction( QString( tr("Subtract background '%1' from '%2'") ).arg( selected_spectrum, active_spectrum )
                                                      , BackgroundSubtraction( active_folium, folium, processor ) ) )
                            a->setEnabled( !active_spectrum.isEmpty() );

                        menu.addAction( QString( tr("Remove '%1'") ).arg( QString::fromStdWString( folium.name() ) ), [=](){
                                processor->remove( folium );
                                invalidateSession( processor );
                            } );

                    }
                }
                
                if ( folium.parentFolder().name() == L"Chromatograms" ) {

                    menu.addAction( tr( "Create Spectrogram" ), [processor] () { processor->createSpectrogram(); } );
                    menu.addAction( tr( "Save Chromatogram as..."), SaveChromatogramAs( folium, processor ) );
                    menu.addSeparator();                    
                    menu.addAction( QString( tr("Remove '%1'") ).arg( QString::fromStdWString( folium.name() ) ), [&](){
                            processor->remove( folium );
                            invalidateSession( processor );
                        } );
                }
                
                if ( folium.parentFolder().name() == L"Spectrograms" ) {
                    menu.addAction( tr("Apply lock mass"), [processor,folium](){
                            if ( auto v = portfolio::get< std::shared_ptr< adcontrols::MassSpectra > >( folium ) )
                                processor->applyLockMass( v );
                        } );
                    menu.addAction( tr("Export matched masses..."), [processor,folium](){
                            if ( auto v = portfolio::get< std::shared_ptr< adcontrols::MassSpectra > >( folium ) )
                                processor->exportMatchedMasses( v, folium.id() );
                        } );
                }

                processor->addContextMenu( adprocessor::ContextMenuOnNavigator, menu, folium );
            }
            
            menu.addSeparator();
            menu.addAction( tr( "Export data tree to XML" ), [processor] () { processor->exportXML(); } );

            menu.exec( globalPos );

        } // if dataprocessor
    } // if index.isValid
}

void
NavigationWidget::handleAllCheckState( bool checked, const QString& node )
{
    QStandardItemModel& model = *pModel_;

    for ( int row = 0; row < model.rowCount(); ++row ) {
        auto parent = model.itemFromIndex( model.index( row, 0 ) );
        for ( int n = 0; n < parent->rowCount(); ++n ) {
            if ( model.data( model.index( n, 0, parent->index() ) ).toString() == node ) {
                auto sp = model.itemFromIndex( model.index( n, 0, parent->index() ) );
                for ( int isp = 0; isp < sp->rowCount(); ++isp ) {
                    if ( auto item = model.itemFromIndex(model.index(isp, 0, sp->index())) ) {
                        if ( item->isCheckable() )
                            item->setCheckState( checked ? Qt::Checked : Qt::Unchecked );
                    }
                }
            }
        }
    }
}

void
NavigationWidget::handleUncheckAllSpectra()
{
    handleAllCheckState( false, "Spectra" );
}

void
NavigationWidget::handleCheckAllSpectra()
{
    handleAllCheckState( true, "Spectra" );
}



Q_DECLARE_METATYPE( portfolio::Folium )
Q_DECLARE_METATYPE( portfolio::Folder )
Q_DECLARE_METATYPE( dataproc::Dataprocessor * )
