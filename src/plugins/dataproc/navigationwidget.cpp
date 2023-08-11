// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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
#include "export_chromatogram.hpp"
#include "export_spectrum.hpp"
#include "sessionmanager.hpp"
#include "actionmanager.hpp"
#include "utility.hpp"
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adutils/processeddata.hpp>
#include <adutils/fsio2.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adprocessor/generator_property.hpp>
#include <qtwrapper/qfiledialog.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/modemanager.h>
#include <QApplication>
#include <QClipboard>
#include <QDataStream>
#include <QDebug>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/json.hpp>
#include <boost/json/value_from.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <iomanip>
#include <array>

class StandardItemHelper {
public:
    StandardItemHelper();

    template<class T> static QStandardItem * appendRow( QStandardItemModel& model, const T& value ) {
        // top level row that should be dataset (file on disk)
        QStandardItem * item = new QStandardItem;
        item->setData( QVariant::fromValue<T>( value ), Qt::UserRole );
        model.appendRow( item );
        return item;
    }

    template<class T> static QStandardItem * appendRow( QStandardItem& parent, const T& value
                                                        , bool isCheckable, bool isChecked = false ) {
        QStandardItemModel& model = *parent.model();
		int row = parent.rowCount();
		parent.insertRow( row, new QStandardItem );
		QStandardItem * item = model.itemFromIndex( model.index( row, 0, parent.index() ) );
		//
        if ( isCheckable ) {
			item->setCheckable( true );
            item->setEditable( true );
			item->setData( isChecked ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
			item->setData( QString::fromStdWString( value.name() ), Qt::EditRole );
			item->setData( QVariant::fromValue<T>( value ), Qt::UserRole );
        } else {
			item->setData( QString::fromStdWString( value.name() ), Qt::EditRole );
			item->setData( QVariant::fromValue<T>( value ), Qt::UserRole );
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
		item->setToolTip( QString::fromStdWString( folium.name<wchar_t>() ) );
    }

    static void appendFolium( QStandardItem& parent, portfolio::Folium& folium ) {
		QStandardItem * item = StandardItemHelper::appendRow( parent
                                                              , folium
                                                              , true
                                                              , folium.attribute( L"isChecked" ) == L"true" );
		item->setToolTip( QString::fromStdWString( folium.name<wchar_t>() ) );
        // set sort value
        QRegularExpression
            re( R"__(m\/z[ ]+([0-9\.]+).*;tR=([0-9\.]+).*$)__" // m/z 440.267(W 50.0mDa),AVG,p0;tR=38.5(3.7) S[1,2]
                R"__(|m\/z[ ]+([0-9\.]+).*$)__"                // m/z 1011.715(W 75.0mDa),AVG,p0 C[3]
                R"__(|[A-Za-z ]+([0-9\.]+)-([0-9\.]+)s)__"     // AVG 24.679-25.485s S[4,5]
                );
        auto match = re.match( item->data( Qt::EditRole ).toString() );
        std::ostringstream o;
        if ( match.hasMatch() ) {
            if ( !match.captured( 1 ).isEmpty() && !match.captured( 2 ).isEmpty() ) {
                // spectra
                o << "S";
                o << std::setw(10) << std::setfill('0') << int( match.captured( 2 ).toDouble() * 1000 ); // tR ms
                o << ",";
                o << std::setw(10) << std::setfill('0') << int( match.captured( 1 ).toDouble() * 1000 ); // mass mDa
            } else if ( !match.captured( 3 ).isEmpty() ) {
                // chromatograms
                o << "C";
                o << std::setw(10) << std::setfill('0') << int( match.captured( 3 ).toDouble() * 1000 ); // mass mDa
            } else if ( !match.captured( 4 ).isEmpty() && !match.captured( 5 ).isEmpty() ) {
                // spectra
                o << "_S";
                o << std::setw(10) << std::setfill('0') << int( match.captured( 4 ).toDouble() * 1000 ); // mass t1
                o << ",";
                o << std::setw(10) << std::setfill('0') << int( match.captured( 5 ).toDouble() * 1000 ); // mass t2
            }
        } else {
            qDebug() << match << "\t" << item->data( Qt::EditRole ).toString();
            o << item->data( Qt::EditRole ).toString().toStdString();
        }
        item->setData( QString::fromStdString( o.str() ), Qt::UserRole + 1 );

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
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    qRegisterMetaTypeStreamOperators< portfolio::Folium >( "portfolio::Folium" );
    qRegisterMetaTypeStreamOperators< portfolio::Folder >( "portfolio::Folder" );
#endif

    pTreeView_->setModel( pModel_ );
    pTreeView_->setItemDelegate( pDelegate_ );
	pTreeView_->setDragEnabled( false );
    pTreeView_->setTextElideMode(Qt::ElideMiddle);

    setStyleSheet(
        "QTreeView {"
        " show-decoration-selected: 1;"
#if defined Q_OS_MAC
        " font-size: 11pt;"
#else
        " font-size: 9pt;"
#endif
        "}"
        "QTreeView::item:selected {"
        " border: 1px solid #567dbc;"
        " color: blue;"
        "}"
        "QTreeView::item:open {"
        // " background-color: #c5ebfb;"
        " color: blue;"
        "}"
        );


    pTreeView_->setSelectionMode( QAbstractItemView::ExtendedSelection );
    setFocusProxy( pTreeView_ );
    initView();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( pTreeView_ );
    layout->setSpacing( 0 );
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );

    // connections
    connect( pModel_, SIGNAL( modelReset() ), this, SLOT( initView() ) );

    connect( pTreeView_, &QTreeView::activated, this, &NavigationWidget::handle_activated );
    connect( pTreeView_, &QTreeView::clicked, this, &NavigationWidget::handle_clicked );
    connect( pTreeView_, &QTreeView::doubleClicked, this, &NavigationWidget::handle_doubleClicked );
    connect( pTreeView_, &QTreeView::entered, this, &NavigationWidget::handle_entered );

    pTreeView_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect( pTreeView_, &QTreeView::customContextMenuRequested, this, &NavigationWidget::handleContextMenuRequested );

    if ( SessionManager * mgr = SessionManager::instance() ) {
        connect( mgr, &SessionManager::onRemoveSession, this, &NavigationWidget::handleRemoveSession );
        connect( mgr, &SessionManager::signalAddSession, this, &NavigationWidget::handleAddSession );
        connect( mgr, &SessionManager::onSessionUpdated, this
                 , [&] ( Dataprocessor* dp, const QString& id ){ handleSessionUpdated( dp, id ); } );
        connect( mgr, &SessionManager::onFolderChanged, this, &NavigationWidget::handleFolderChanged );
        connect( pModel_, &QStandardItemModel::itemChanged, this, &NavigationWidget::handleItemChanged );
        connect( mgr, &SessionManager::foliumChanged, this, &NavigationWidget::handleFoliumChanged );
    }

    setAcceptDrops( true );

    if ( auto am = DataprocPlugin::instance()->actionManager() ) {
        am->connect_navigation_pointer( this );
    }

    pTreeView_->installEventFilter( this );
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

    // expand top level projects
    for ( int i = 0; i < pModel_->rowCount(sessionIndex); ++i)
        view.expand( model.index(i, 0, sessionIndex));

    view.setMouseTracking( true );
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

		if ( Dataprocessor * dp = StandardItemHelper::findDataprocessor( item->index() ) ) {
            SessionManager::instance()->checkStateChanged( dp, folium, state == Qt::Checked );
        }
    }
}

void
NavigationWidget::invalidateSession( Dataprocessor * processor )
{
    QStandardItemModel& model = *pModel_;

    if ( QStandardItem * item = StandardItemHelper::findRow( model, processor ) ) {
        model.removeRows( 0, item->rowCount(), item->index() );

        portfolio::Portfolio portfolio = processor->getPortfolio();
        for ( auto folder: portfolio.folders() ) {
            ADDEBUG() << "adding folder: " << folder.name();
            PortfolioHelper::appendFolder( *item, folder );
        }

        // expanding top and 2nd levels
        // pTreeView_->expand( item->index() );
        // for ( int i = 0; i < item->rowCount(); ++i)
        //     pTreeView_->expand( model.index( i, 0, item->index()) );
        // pTreeView_->setUpdatesEnabled( true );
    }
}

void
NavigationWidget::handleInvalidateFolium( Dataprocessor * processor, portfolio::Folium folium )
{
    if ( auto top = StandardItemHelper::findRow< Dataprocessor * >( *pModel_, processor ) ) {
        if ( auto folder = StandardItemHelper::findFolder( top, folium.parentFolder().name() ) ) {
            if ( auto item = StandardItemHelper::findFolium( folder, folium.id() ) ) {
                pModel_->removeRows( 0, item->rowCount(), item->index() );
                for ( auto& att: folium.attachments() ) {
                    if ( StandardItemHelper::findFolium( item, att.id() ) == nullptr )
                        PortfolioHelper::appendAttachment( *item, att );
                }
            }
        }
    }
}

// add child node when process applied (such as Centroid)
// this is responcible to add/redraw attributes under specified folium
void
NavigationWidget::handleFoliumChanged( Dataprocessor * processor, const portfolio::Folium& folium )
{
    if ( auto top = StandardItemHelper::findRow< Dataprocessor * >( *pModel_, processor ) ) {
        if ( auto folder = StandardItemHelper::findFolder( top, folium.parentFolder().name() ) ) {
            if ( auto item = StandardItemHelper::findFolium( folder, folium.id() ) ) {
                item->setData( QVariant::fromValue< portfolio::Folium >( folium ), Qt::UserRole );
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
            pTreeView_->setUpdatesEnabled( false );
            for ( auto folium: folio ) {
                if ( QStandardItem * item = StandardItemHelper::findFolium( procItem, folium.id() ) ) {
                    item->setData( QVariant::fromValue< portfolio::Folium >( folium ), Qt::UserRole );
                } else {
                    PortfolioHelper::appendFolium( *folderItem, folium );
                }
            }
            pTreeView_->setUpdatesEnabled( true );
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
#if QTC_VERSION <= 0x03'02'81
    QString filename = processor->filePath();
#else
    QString filename = processor->filePath().toString();
#endif

    QStandardItemModel& model = *pModel_;

    if ( QStandardItem * processorItem = StandardItemHelper::findRow< Dataprocessor * >( model, processor ) ) {

        if ( QStandardItem * folderItem
             = StandardItemHelper::findFolder( processorItem, folium.parentFolder().name() ) ) {

            if ( QStandardItem * item = StandardItemHelper::findFolium( processorItem, folium.id() ) ) {
                // replace existing
                item->setData( QVariant::fromValue< portfolio::Folium >( folium ), Qt::UserRole );
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
    std::filesystem::path path( processor->filename() );

    if ( QStandardItem * item = StandardItemHelper::appendRow( *pModel_, processor ) ) {

        connect( processor, &Dataprocessor::invalidateFolium, this, &NavigationWidget::handleInvalidateFolium );

        auto ppath = path.parent_path().parent_path();
        item->setData( QString::fromStdString( std::filesystem::relative( path, ppath ).string() ), Qt::EditRole );
        item->setEditable( false );
        item->setToolTip( QString::fromStdString( path.string() ) ); // full path

        portfolio::Portfolio portfolio = processor->getPortfolio();

        for ( auto& folder: portfolio.folders() )
            PortfolioHelper::appendFolder( *item, folder );

        pTreeView_->expand( item->index() );
        // expand second levels (Chromatograms|Spectra|MSCalibration etc.)
        for ( int i = 0; i < item->rowCount(); ++i)
            pTreeView_->expand( pModel_->index( i, 0, item->index()) );

    }
}

void
NavigationWidget::handle_activated( const QModelIndex& index )
{
    if ( index.isValid() ) {

        QVariant data = index.data( Qt::UserRole );

		if ( data.canConvert< portfolio::Folder >() ) {
			// folder (Spectra|Chromatograms)
			portfolio::Folder folder = data.value< portfolio::Folder >();
            qtwrapper::waitCursor wait;
			Dataprocessor * processor = StandardItemHelper::findDataprocessor( index );
			processor->setCurrentSelection( folder );

		} else if ( data.canConvert< portfolio::Folium >() ) {

            portfolio::Folium folium = data.value< portfolio::Folium >();

			Dataprocessor * processor = StandardItemHelper::findDataprocessor( index );
			if ( processor ) {
				std::string tname = static_cast<boost::any&>( folium ).type().name();
                qtwrapper::waitCursor wait;
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
    // ADDEBUG() << "## " << __FUNCTION__ << " ## ";
    (void)index;
}

void
NavigationWidget::handle_entered( const QModelIndex& index )
{
    // ADDEBUG() << "## " << __FUNCTION__ << " ## ";
    (void)index;
}

void
NavigationWidget::handle_pressed( const QModelIndex& index )
{
    ADDEBUG() << "## " << __FUNCTION__ << " ## ";
    (void)index;
}

namespace { // anonymous

    //////////////// find_t< portfolio::Folium | portfolio::Folder > //////////////////
    template< typename T >
    struct find_t {
        T operator()( const QModelIndex& index ) const {
            QVariant data = index.model()->data( index, Qt::UserRole );
            if ( data.canConvert< T >() ) {
                return data.value< T >();
            }
            return {};
        }

        T operator()( const QStandardItem * item ) const {
            QVariant data = item->data( Qt::UserRole );
            if ( data.canConvert< T >() ) {
                return data.value< T >();
            }
            return {};
        }
    };

    template<> Dataprocessor * find_t< Dataprocessor * >::operator()( const QModelIndex& index ) const {
		QModelIndex t( index ); //  = index.parent();
        while ( t.isValid() && !t.data( Qt::UserRole ).canConvert< dataproc::Dataprocessor * >() )
			t = t.parent();
		if ( t.isValid() && t.data( Qt::UserRole ).canConvert< dataproc::Dataprocessor * >() )
            return t.data( Qt::UserRole ).value< dataproc::Dataprocessor * >();
		return nullptr;
    }

    template<> Dataprocessor * find_t< Dataprocessor * >::operator()( const QStandardItem * item ) const {
        return find_t< Dataprocessor * >()( item->index() );
    }

    //////////////////////////////// find_processor_t<> /////////////////////////////////
    template< typename T >
    struct find_processor_t {
        std::pair< Dataprocessor *, T > operator()( const QModelIndex& index ) const {
            return { find_t< Dataprocessor * >()( index ), find_t< T >()( index ) };
        }
    };

    //////////////////////////////// set_attribute /////////////////////////////////
    struct set_attribute {
        const QModelIndexList& rows_;
        set_attribute( const QModelIndexList& rows ) : rows_( rows )  { }

        void operator()( std::pair< std::string, std::string>&& keyValue ) const {
            for ( auto index: rows_ ) {
                auto [processor, folium] = find_processor_t< portfolio::Folium >()( index );
                if ( processor && folium )
                    processor->setAttribute( folium, std::move( keyValue ) );
            }
        }
    };

    //////////////////////////////// check selected /////////////////////////////////
    struct set_checked {
        const QModelIndexList& rows_;
        set_checked( const QModelIndexList& rows ) : rows_( rows )  { }
        void operator()( bool check ) const {
            for ( auto index: rows_ ) {
                if ( auto model = qobject_cast< const QStandardItemModel * >( index.model() ) ) {
                    if ( auto item = model->itemFromIndex( index ) ) {
                        if ( item->isCheckable() ) {
                            item->setCheckState( check ? Qt::Checked : Qt::Unchecked );
                            if ( auto folium = find_t< portfolio::Folium >()( item ) ) {
                                folium.setAttribute( "isChecked", check ? "true" : "false" );
                            }
                        }
                    }
                }
            }
        }
    };

    ///////////////////////////////////////////////////////////////////////////////
    struct check_all_in_folder {
        const QModelIndexList& rows_;
        check_all_in_folder( const QModelIndexList& rows ) : rows_( rows ) {}
        void operator()( bool check ) const {
            for ( auto index: rows_ ) {
                if ( auto model = qobject_cast< const QStandardItemModel * >( index.model() ) ) {
                    auto parent = model->itemFromIndex( index );
                    for ( int row = 0; row < parent->rowCount(); ++row ) {
                        if ( auto item = model->itemFromIndex( model->index( row, 0, parent->index() ) ) ) {
                            if ( item->isCheckable() ) {
                                item->setCheckState( check ? Qt::Checked : Qt::Unchecked );
                                if ( auto folium = find_t< portfolio::Folium >()( item ) ) {
                                    folium.setAttribute( "isChecked", check ? "true" : "false" );
                                }
                            }
                        }
                    }
                }
            }
        }
    };

    //////////////////////////////// spectra_from_chromatographic_peaks JCB2009 /////////////////////////////////
    struct spectra_from_chromatographic_peaks {
        const QModelIndex& index_;
        spectra_from_chromatographic_peaks( const QModelIndex& index ) : index_( index )  {
        }
        void operator()() const {
            std::vector< portfolio::Folium > list;
            if ( auto processor = find_t< Dataprocessor * >()( index_ ) ) {
                if ( auto folder = processor->portfolio().findFolder( L"Chromatograms" ) ) {
                    for ( const auto& folium: folder.folio() ) {
                        if ( folium.attribute( L"isChecked" ) == L"true" )
                            list.emplace_back( folium );
                    }
                }
                processor->handleSpectraFromChromatographicPeaks( std::move( list ) );
            }
        }
    };

    //////////////////////////////// remove duplicated chromatograms /////////////////////////////////
    struct remove_duplicated_chromatogram {
        const QModelIndexList& rows_;
        remove_duplicated_chromatogram( const QModelIndexList& rows ) : rows_( rows )  {}
        void operator()() const {
            std::set< Dataprocessor * > v;
            for ( auto index: rows_ ) {
                if ( auto processor = find_t< Dataprocessor * >()( index ) )
                    v.emplace( processor );
            }

            for ( auto& dp: v ) {
                if ( auto folder = dp->portfolio().findFolder( L"Chromatograms" ) ) {
                    std::vector< portfolio::Folium > list;
                    for ( auto folium: folder.folio() )
                        list.emplace_back( std::move( folium ) );
                    dp->handleRemoveDuplicatedChromatograms( std::move( list ) );
                }
            }
        }
    };

    //////////////////////////////// sort /////////////////////////////////

    struct sort_by_value {
        std::vector< QModelIndex >
        populate( const QModelIndexList& rows ) const {
            std::vector< QModelIndex > _;
            for ( auto index: rows ) {
                if ( (index.data().toString() == "Chromatograms")  ||
                     (index.data().toString() == "Spectra") )
                    _.emplace_back( index );
            }
            return _;
        }
        void operator()( const QModelIndexList& rows, QStandardItemModel * model ) const {
            model->setSortRole( Qt::UserRole + 1 );
            auto indecies = populate( rows );
            for ( auto index: rows ) {
                model->itemFromIndex( index )->sortChildren( 0, Qt::AscendingOrder );
            }
        }
    };

    //////////////////////////////// chromatogram_generator_list /////////////////////////////////
    struct copy_chromatogram_generator {
        QTreeView * p_;
        copy_chromatogram_generator( QTreeView * p ) : p_( p ) {}
        void operator()() const {
            std::vector< adprocessor::generator_property > gv;
            for ( auto index: p_->selectionModel()->selectedRows() ) {
                auto [processor, folium] = find_processor_t< portfolio::Folium >()( index );
                processor->fetch( folium );
                if ( auto v = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( folium ) ) {
                    gv.emplace_back( adprocessor::generator_property( *v ) );
                    gv.back().set_dataSource( { folium.name<char>(), folium.uuid() } );
                }
            }
            auto json = boost::json::serialize( boost::json::value_from( gv ) );
            if ( auto md = new QMimeData() ) {
                md->setData( "application/json", QByteArray( json.data(), json.size() ) );
                md->setData( "text/plain", QByteArray( json.data(), json.size() ) );
                QApplication::clipboard()->setMimeData( md );
            }
        };

        static std::vector< adprocessor::generator_property > fromClipboard() {
            if ( auto md = QApplication::clipboard()->mimeData() ) {
                auto data = md->data( "application/json" );
                if ( !data.isEmpty() ) {
                    auto jv = adportable::json_helper::parse( data.toStdString() );
                    if ( jv != boost::json::value{} )
                        return boost::json::value_to< std::vector< adprocessor::generator_property > >( jv );
                }
            }
            return {};
        }
    };

    //--------------------------------
    template< Qt::CheckState CheckState >
    struct set_attribute_all {
        std::set< QModelIndex > rows_;
        set_attribute_all( const QModelIndexList& rows ) {
            std::for_each( rows.begin(), rows.end(), [&](auto a){
                if ( a.data( Qt::EditRole ).toString() == "Chromatograms" ||
                     a.data( Qt::EditRole ).toString() == "Spectra" )
                    rows_.emplace( a );
            });
        }

        void operator()( std::pair< std::string, std::string >&& keyValue ) const {
            for ( auto parent: rows_ ) {
                if ( auto model = qobject_cast< const QStandardItemModel * >( parent.model() ) ) {
                    for ( int row = 0; row < parent.model()->rowCount( parent ); ++row ) {
                        if ( auto item = model->itemFromIndex( parent.model()->index( row, 0, parent ) ) ) {
                            if ( item->isCheckable() && ( item->checkState() == CheckState ) ) {
                                auto [processor, folium] = find_processor_t< portfolio::Folium >()( item->index() );
                                if ( processor && folium )
                                    processor->setAttribute( folium, std::move( keyValue ) );
                            }
                        }
                    }
                }
            }
        }
    };

    // --------------------------------
    struct delete_removed {
        std::set< QModelIndex > indecies_;
        QAbstractItemModel * model_;
        delete_removed( const QModelIndexList& rows, QAbstractItemModel * m ) : model_( m ) {
            for ( auto index: rows ) {
                while ( index.isValid() && index != index.model()->index(0,0,{}) )
                    index = index.parent();
                if ( index.isValid() )
                    indecies_.emplace( index );
            }
        }
        std::set< int > populate( const QModelIndex& index ) const {
            std::set< int > rows;
            for ( size_t row = 0; row < index.model()->rowCount( index ); ++row ) {
                auto t_index = index.model()->index( row, 0, index );
                if ( auto folium = find_t< portfolio::Folium >()( t_index ) ) {
                    if ( folium.attribute( L"remove" ) == L"true" )
                        rows.emplace( row );
                }
            }
            return rows;
        }

        void operator()() const {
            for ( auto index: indecies_ ) {
                auto model = index.model();
                if ( auto processor = find_t< Dataprocessor * >()( index ) ) {
                    for ( size_t i = 0; i < model->rowCount( index ); ++i ) {
                        auto p_index = model->index( i, 0, index );
                        auto rows = populate( p_index );
                        std::for_each( rows.rbegin(), rows.rend(),[&](int row){
                            model_->removeRow( row, p_index );
                        });
                    }
                    // delete data
                    processor->deleteRemovedItems();
                }
            }

        }
    };

    // --------------------------------
    struct fetch_t {
        static void fetch( const QModelIndex& index, portfolio::Folium folium ) {
            if ( folium.empty() ) {
                if ( auto processor = StandardItemHelper::findDataprocessor( index ) )
                    processor->fetch( folium );
            }
        }
    };

    // --------------------------------
    struct selected_folders {
        const QModelIndexList& indices_;
        size_t selFolderCounts_;
        size_t selFoliumCounts_;
        std::set< QString > selFolders_;
        selected_folders( const QModelIndexList& indices ) : indices_( indices )
                                                           , selFolderCounts_( 0 )
                                                           , selFoliumCounts_( 0 ) {
            for ( const auto& index: indices_ ) {
                if ( auto folder = find_t< portfolio::Folder >()( index ) ) {
                    selFolders_.insert( QString::fromStdWString( folder.name() ) );
                    selFolderCounts_++;
                } else if ( auto folium = find_t< portfolio::Folium >()( index ) ) {
                    selFolders_.insert( QString::fromStdWString( folium.parentFolder().name() ) );
                    selFoliumCounts_++;
                }
            }
        }
        const std::set< QString >& folders() const { return selFolders_; }
        size_t foliumCounts() const { return selFoliumCounts_; }
        size_t folderCounts() const { return selFolderCounts_; }
        bool contains( const QString& key ) const {
#if __cplusplus >= 202002L
            return selFolders_.contains( key );
#else
            return (selFolders_.find( key ) != selFolders_.end());
#endif
        }
    };

    // --------------------------------
    struct merge_selection {
        std::vector< QModelIndex > indices_;
        merge_selection( QModelIndexList& indices ) { // : indices_( indices ) {
            for ( auto index: indices ) {
                if ( auto folder = find_t< portfolio::Folder >()( index ) ) {
                    if ( folder.name() == L"Chromatograms" ) {
                        indices_.emplace_back( index );
                    }
                }
                if ( auto folium = find_t< portfolio::Folium >()( index ) ) {
                    if ( folium.dataClass() == L"Chromatogram" ) {
                        if ( std::find( indices.begin(), indices.end(), index.parent() ) == indices.end() ) {
                            indices_.emplace_back( index );
                        }
                    }
                }
            }
        }

        std::vector< portfolio::Folium >
        operator()() const {
            std::vector< portfolio::Folium > selected;
            for ( auto index: indices_ ) {
                Dataprocessor * dp = find_t< Dataprocessor * >()( index );
                if ( auto folium = find_t< portfolio::Folium >()( index ) ) {
                    dp->fetch( folium );
                    selected.emplace_back( folium );
                } else if ( auto folder = find_t< portfolio::Folder >()( index ) ) {
                    for ( auto folium: folder.folio() ) {
                        dp->fetch( folium );
                        selected.emplace_back( folium );
                    }
                }
                for ( auto folium: selected ) {
                    auto chro = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( folium );
                    if ( chro ) {
                        adprocessor::generator_property g( *chro );
                        ADDEBUG() << g.mass();
                    } else {
                        ADDEBUG() << "chro null ptr";
                    }
                }
            }
            return selected;
        }
    };

    // --------------------------------
    struct attachment_walker {
        const portfolio::Folium& folium_;
        std::tuple< portfolio::Folium    // centroid;
                    , portfolio::Folium  // filtered
                    > has_a_;

        attachment_walker( const portfolio::Folium& folium ) : folium_( folium ) {
            portfolio::Folio atts = folium.attachments();
            auto itCentroid = std::find_if( atts.begin(), atts.end()
                                            , [] ( const auto& a ){ return a.name() == Constants::F_CENTROID_SPECTRUM; });
            if ( itCentroid != atts.end() )
                std::get< 0 >( has_a_ ) = *itCentroid;

            auto itFiltered = std::find_if( atts.begin(), atts.end()
                                            , [] ( const auto& a ){ return a.name() == Constants::F_DFT_FILTERD; });
            if ( itFiltered != atts.end() ) {
                std::get< 1 >( has_a_ ) = *itFiltered;
            }
        }
    };
}

namespace {
    // if selected for multiple 'Chromatograms' folders
    struct collect_baselines_for_selected_folders {
        const QModelIndexList& indices_;
        collect_baselines_for_selected_folders( const QModelIndexList& indices ) : indices_( indices ) {};
        void operator()() const {
            for ( auto& index: indices_ ) {
                auto [ processor, folder ] = find_processor_t< portfolio::Folder >()( index );
                if ( processor && folder && ( folder.name() == L"Chromatograms" ) ) {
                    for ( auto folium: folder.folio() ) {
                        processor->fetch( folium );
                        processor->baselineCollection( folium );
                    }
                }
            }
        }
    };


    // collect baseline for selected folia
    struct collect_baseline_for_selected_folia {
        const QModelIndexList& indices_;
        collect_baseline_for_selected_folia( const QModelIndexList& indices ) : indices_( indices ) {};
        void operator()() const {
            for ( auto& index: indices_ ) {
                auto [processor, folium] = find_processor_t< portfolio::Folium >()( index );
                if ( processor && folium && folium.parentFolder().name() == L"Chromatograms" ) {
                    processor->fetch( folium );
                    processor->baselineCollection( folium );
                }
            }
        }
    };

    //-----------------
    enum ActionType { checkAll, unCheckAll, asProfile, asCentroid, doCalibration, removedChecked, asDFTProfile };

    struct SaveSpectrumAs {
        ActionType idAction;
        portfolio::Folium parent;
        portfolio::Folium folium;
        Dataprocessor * processor;
        SaveSpectrumAs( ActionType id, portfolio::Folium& pf, portfolio::Folium& f, const QModelIndex& index )
            : idAction( id ), parent( pf ), folium( f ), processor( find_t< Dataprocessor * >()( index ) ) {
        }

        void operator()() {
            if ( auto path = utility::save_spectrum_as()( parent, folium ) ) {
                if ( path->extension() == ".adfs" ) {
                    adutils::fsio2::appendOnFile( path->wstring(), folium, *processor->file() );
                } else {
                    boost::filesystem::ofstream of( *path );
                    auto ms = portfolio::get< adcontrols::MassSpectrumPtr >( folium );
                    export_spectrum::write( of, *ms );
                }
            }
        }
    };

    struct mslock_data {
        static std::shared_ptr< const adcontrols::lockmass::mslock >
        find( const portfolio::Folium& folium ) {
            if ( auto att = portfolio::find_first_of(
                     folium.attachments()
                     , []( const auto& a ){ return a.name() == Constants::F_MSLOCK; }) ) {
                return portfolio::get< std::shared_ptr< adcontrols::lockmass::mslock > >( att );
            }
            return {};
        }
    };

    struct ExportMSLock {
        portfolio::Folium folium;
        ExportMSLock( portfolio::Folium& f ) : folium( f ) {}
        void operator()() {
            if ( auto mslock = mslock_data::find( folium ) ) {
                if ( auto path = utility::export_mslock_as()( folium ) ) {
                    boost::filesystem::ofstream of( *path );
                    of << boost::json::value_from( *mslock ) << std::endl;
                }
            } else {
                ADDEBUG() << "------- mslock is null -----";
            }
        }
    };


    struct SaveChromatogramAs {
        portfolio::Folium folium;
        Dataprocessor * processor;

        SaveChromatogramAs( portfolio::Folium& f, Dataprocessor * p ) : folium( f ), processor( p )
            {}
        void operator()() {
            if ( auto path = utility::save_chromatogram_as()( folium ) ) {
                if ( path->extension() == ".adfs" ) {
                    adutils::fsio2::appendOnFile( path->wstring(), folium, *processor->file() );
                } else {
                    boost::filesystem::ofstream of( *path );
                    if ( auto c = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( folium ) )
                        export_chromatogram::write( of, *c );
                }
            }
        }
    };

    // reconstract mass spectrum form a list of checked chromatograms, which were generated by 'extract_by_peak_info' function.
    class make_spectrum_from_checked_chromatograms {
        QStandardItemModel& model;
        const QModelIndex& index;
        bool enable_;

        bool isValid( portfolio::Folium& folium, Dataprocessor * processor ) const {
            processor->fetch( folium );
            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                if ( auto chro = portfolio::get_shared_of< adcontrols::Chromatogram >()( folium.data() ) ) {
                    auto jv = adportable::json_helper::find( chro->generatorProperty(), "generator.extract_by_peak_info.pkinfo" );
                    return ! jv.is_null();
                }
            }
            return false;
        }

        std::pair< portfolio::Folder, QModelIndex > findFolder( QModelIndex index ) const {
            while ( index.isValid() ) {
                if ( auto folder = find_t< portfolio::Folder >()( index ) )
                    return { folder, index };
                index = index.parent();
            }
            return {};
        }
        QModelIndexList populate( QModelIndex parent, Qt::CheckState checkState ) const {
            QModelIndexList indices;
            auto pitem = model.itemFromIndex( parent );
            for ( int row = 0; row < pitem->rowCount(); ++row ) {
                if ( auto item = model.itemFromIndex( model.index( row, 0, parent ) ) ) {
                    if ( item->isCheckable() && item->checkState() == checkState ) {
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
                        indices.push_back( item->index() );
#else
                        indices.emplace_back( item->index() );
#endif
                    }
                }
            }
            return indices;
        }
    public:
        make_spectrum_from_checked_chromatograms( QStandardItemModel& m
                                                  , const QModelIndex& idx ) : model( m )
                                                                             , index( idx )
                                                                             , enable_( false ) {

            if ( auto processor = find_t< Dataprocessor * >()( index ) ) {   // <-------- folium node selected
                if ( auto folium = find_t< portfolio::Folium >()( index ) ) {
                    if (( enable_ = isValid( folium, processor ) ))
                        return;
                }
                if ( auto folder = find_t< portfolio::Folder >()( index ) ) { // <------- folder 'Chromatograms' selected
                    for ( auto folium: folder.folio() ) {
                        if (( enable_ = isValid( folium, processor ) ))
                            break;
                    }
                }
            }
        }

        bool enable() const { return enable_; }

        void operator()() const {
            if ( auto processor = find_t< Dataprocessor * >()( index ) ) {
                auto [folder, parent] = findFolder( index );
                if ( folder && parent.isValid() ) {
                    adcontrols::MSPeakInfo info;
                    auto list = populate( parent, Qt::Checked );
                    std::for_each( list.begin(), list.end()
                                   , [&](const auto& idx){
                                       if ( auto folium = find_t< portfolio::Folium >()( idx ) ) {
                                           processor->fetch( folium );
                                           if ( auto chro = portfolio::get_shared_of< adcontrols::Chromatogram >()( folium ) ) {
                                               auto jv = adportable::json_helper::find( chro->generatorProperty()
                                                                                        , "generator.extract_by_peak_info.pkinfo" );
                                               if ( ! jv.is_null() ) {
                                                   auto pk = boost::json::value_to< adcontrols::MSPeakInfoItem >( jv );
                                                   info << pk; // append to pkinfo
                                               }
                                           }
                                       }
                                   });
                    if ( info.size() ) {
                        processor->xicSelectedMassPeaks( std::move( info ) );
                    }
                }
            }
        }
    };

    struct XIC2MS {
        QStandardItemModel& model;
        const QModelIndex& index;
        XIC2MS( QStandardItemModel& m, const QModelIndex& idx ) : model( m ), index( idx )  {}
        void operator()() {
            QVariant data = model.data( index, Qt::UserRole ); // must be spectrum
            if ( data.canConvert< portfolio::Folium >() ) {
                auto folium = data.value< portfolio::Folium >();
                if ( auto processor = find_t< Dataprocessor * >()( index ) ) {
                    processor->fetch( folium );
                    if ( portfolio::is_type< adutils::MassSpectrumPtr >( folium ) ) {
                        processor->markupMassesFromChromatograms( std::move( folium ) );
                    }
                }
            }
        }
    };

    struct GlobalMSLock {
        Dataprocessor * processor_;
        portfolio::Folium folium_;
        GlobalMSLock( const QModelIndex& index ) : processor_( find_t< Dataprocessor * >()( index ) )
                                                 , folium_( find_t< portfolio::Folium >()( index ) ) {
        }
        void operator()() {
            if ( auto mslock = mslock_data::find( folium_ ) ) {
                if ( processor_ )
                    processor_->handleSetGlobalMSLock( folium_ );
            }
        }
    };

    struct RemoveGlobalMSLock {
        Dataprocessor * processor_;
        portfolio::Folium folium_;
        RemoveGlobalMSLock( const QModelIndex& index ) : processor_( find_t< Dataprocessor * >()( index ) )
                                                       , folium_( find_t< portfolio::Folium >()( index ) ) {
        }
        void operator()() {
            if ( processor_ )
                processor_->handleRemoveGlobalMSLock( folium_ );
        }
    };

    struct CalibrationAction {
        const QModelIndex& index_;
        CalibrationAction( const QModelIndex& index ) : index_( index ) {}

        void operator()() const {
            if ( auto processor = find_t< Dataprocessor * >()( index_ ) ) {
                for ( auto& session : *SessionManager::instance() )
                    processor->sendCheckedSpectraToCalibration( session.processor() );
            }
        }
    };

    struct BackgroundSubtraction {
        portfolio::Folium background;
        portfolio::Folium foreground;
        Dataprocessor * processor;
        BackgroundSubtraction( portfolio::Folium& back, portfolio::Folium& fore, Dataprocessor * p )
            : background( back ), foreground( fore ), processor( p ) {}
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
    auto selRows = pTreeView_->selectionModel()->selectedRows();
    selected_folders selFolders( selRows );

    if ( selRows.size() >= 1 ) {

        bool enable = selFolders.foliumCounts() > 0;
        set_attribute set_attr( selRows );

        menu.addAction( tr( "Remove"    ), [=](){ set_attr( { "remove", "true" } ); } )->setEnabled( enable );
        menu.addAction( tr( "Unremove"  ), [=](){ set_attr( { "remove", "false" } ); } )->setEnabled( enable );
        menu.addAction( tr( "Tag none"  ), [=](){ set_attr( { "tag",    "none"  } ); } )->setEnabled( enable );
        menu.addAction( tr( "Tag red"   ), [=](){ set_attr( { "tag",    "red"   } ); } )->setEnabled( enable );
        menu.addAction( tr( "Tag blue"  ), [=](){ set_attr( { "tag",    "blue"  } ); } )->setEnabled( enable );
        menu.addAction( tr( "Tag green" ), [=](){ set_attr( { "tag",    "green" } ); } )->setEnabled( enable );
        set_checked set_check( selRows );
        menu.addAction( tr( "Check selected" ), [=](){ set_check( true ); })->setEnabled( enable );
        menu.addAction( tr( "Uncheck selected" ), [=](){ set_check( false ); })->setEnabled( enable );
        menu.addSeparator();
    };

    do {
        // [Spectra,Chromatograms] selection
        auto name = std::accumulate( selFolders.folders().begin(), selFolders.folders().end(), QString()
                         , [&](const auto& a, const auto& b){ return a.isEmpty() ? b.toLower() : a + "," + b.toLower(); });

        // enable either Spectra, Chromatograms, or both
        bool enable = selFolders.folderCounts() > 0;

        menu.addAction( tr( "Sort" ), [=](){ sort_by_value()( selRows, pModel_ ); } )->setEnabled( enable );
        if ( enable ) {
            check_all_in_folder check_all( selRows );
            menu.addAction( QString( tr("Uncheck all %1") ).arg( name ), [=](){ check_all( false ); } )->setEnabled( enable );
            menu.addAction( QString( tr("Check all %1") ).arg( name ),   [=](){ check_all( true ); } )->setEnabled( enable );

            set_attribute_all< Qt::Unchecked > set_attr( selRows );
            menu.addAction( QString( tr("Remove all unchecked %1") ).arg( name )
                            , [=]{ set_attr( { "remove", "true" } ); } )->setEnabled( enable );
        }

        // enable only Chromatograms was selected
        enable = selFolders.contains( "Chromatograms" ) && selFolders.folders().size() == 1;

        menu.addAction( QString( tr("Collect all baseline for %1") ).arg( name )
                        , collect_baselines_for_selected_folders( selRows ) )->setEnabled( enable );

        make_spectrum_from_checked_chromatograms spectrum_from_chromatogram( *pModel_, index );
        enable = spectrum_from_chromatogram.enable();
        menu.addAction( QString( tr("Make mass spectrum from checked chromatograms") )
                        , spectrum_from_chromatogram )->setEnabled( enable );

    } while ( 0 );

    if ( selFolders.folders().size() == 1 ) { // Spectra | MSCalibration -- exclusively selected

        if ( selFolders.contains( "Spectra" ) ||
             selFolders.contains( "MSCalibration" ) ) {
            if ( selRows.size() == 1 ) { // single selection
                if ( auto folium = find_t< portfolio::Folium >()( index ) ) { // an item of [Spectrum|Chrmatogram] selected
                    fetch_t::fetch( index, folium );
                    if ( bool isSpectrum = portfolio::is_type< adutils::MassSpectrumPtr >( folium ) ) {
                        menu.addAction( tr("Export mass lock data..." )
                                        , ExportMSLock( folium ) )->setEnabled( folium.attribute( "mslock" ) == "true" );

                        menu.addAction( tr("Set as file global lock mass" )
                                        , GlobalMSLock( index ) )->setEnabled( folium.attribute( "mslock" ) == "true" );

                        attachment_walker attachments( folium );
                        if ( auto ms = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
                            if ( ms->isCentroid() && !ms->isHistogram() ) {
                                menu.addAction( tr("Save centroid spectrum as..."),
                                                SaveSpectrumAs( asCentroid, folium, folium, index ) );
                            } else {
                                menu.addAction( tr("Save profile spectrum as..."),
                                                SaveSpectrumAs( asProfile, folium, folium, index ) );
                            }
                        }
                        auto centroid = std::get< 0 >( attachments.has_a_ );
                        menu.addAction( tr("Save centroid spectrum as..."),
                                        SaveSpectrumAs( asCentroid, folium, centroid, index ) )->setEnabled( bool( centroid ) );

                        auto filtered = std::get< 1 >( attachments.has_a_ );
                        menu.addAction( tr("Save DFT filtered spectrum as..."),
                                        SaveSpectrumAs( asDFTProfile, folium, filtered, index ) )->setEnabled( bool( filtered ) );
                    }
                }

                menu.addAction( tr("Send checked spectra to calibration folder"), CalibrationAction( index ) );
                menu.addAction( tr("Mark masses from checked chromatograms"), XIC2MS( *pModel_, index ) );
                menu.addSeparator();
            }
        }
        if ( selFolders.contains( "MSLock" ) ) {
            if ( auto folium = find_t< portfolio::Folium >()( index ) ) { // an item of [Spectrum|Chrmatogram] selected
                if ( auto a = menu.addAction( tr("Remove from global lock mass" ), RemoveGlobalMSLock( index ) ) ) {
                    a->setEnabled( folium.attribute( "mslock" ) == "true" );
                }
                if ( auto a = menu.addAction( tr("Set as global lock mass" ), GlobalMSLock( index ) ) ) {
                    a->setEnabled( folium.attribute( "mslock" ) == "false" );
                }
            }
        }
    }

    if ( ( selFolders.folders().size() == 1 ) && selFolders.contains( "Chromatograms" ) ) {
        // Chromatograms -- exclusively selected

        copy_chromatogram_generator copy_generator( pTreeView_ );
        menu.addAction( tr( "Copy masses" ), [&](){ copy_generator(); } );

        remove_duplicated_chromatogram remover( selRows );
        menu.addAction( tr( "Remove duplicate" ), [=](){ remover(); } );

        spectra_from_chromatographic_peaks gen_spectra( index );
        menu.addAction( tr( "Create mass spectra from checked chromatograms (JCB 2009)" )
                        , [=](){ gen_spectra(); } );

        if ( Dataprocessor * processor = StandardItemHelper::findDataprocessor( index ) ) {
            auto props = copy_chromatogram_generator::fromClipboard();
            menu.addAction( tr("Create %1 extracted ion chromatograms from copied properties")
                            .arg( props.size() )
                            , [props,processor](){
                                processor->createChromatograms( props ); })->setEnabled( !props.empty() );

            if ( auto folium = find_t< portfolio::Folium >()( index ) ) {
                menu.addAction( tr( "Baseline collection" )
                                , [=] () { processor->baselineCollection( folium ); } )->setEnabled( selRows.size() == 1 );
                menu.addAction( tr( "Create Contour" )
                                , [processor] () { processor->createContour(); } )->setEnabled( selRows.size() == 1 );
                menu.addAction( tr( "Save Chromatogram as...")
                                , SaveChromatogramAs( folium, processor ) )->setEnabled( selRows.size() == 1 );
                menu.addSeparator();
            }
        }
    }

    if ( selFolders.contains( "Chromatograms" ) ) {
        merge_selection merger( selRows );
        menu.addAction( tr( "Merge selection" ), [merger,this]{
            emit document::instance()->onMergeSelection( merger() );
            });
    }

    if ( ( selFolders.folders().size() == 1 ) && selFolders.contains( "Spectrograms" ) ) { // Chromatograms -- exclusively selected
        if ( Dataprocessor * processor = StandardItemHelper::findDataprocessor( index ) ) {
            if ( auto folium = find_t< portfolio::Folium >()( index ) ) { // an item of [Spectrum|Chrmatogram] selected
                menu.addAction( tr("Apply lock mass"), [processor,folium](){
                    if ( auto v = portfolio::get< std::shared_ptr< adcontrols::MassSpectra > >( folium ) )
                        processor->applyLockMass( v );
                } );
                menu.addAction( tr("Export matched masses..."), [processor,folium](){
                    if ( auto v = portfolio::get< std::shared_ptr< adcontrols::MassSpectra > >( folium ) )
                        processor->exportMatchedMasses( v, folium.id() );
                } );
            }
        }
    }

    if ( selRows.size() == 1 ) {
        if ( auto processor = find_t< Dataprocessor * >()( index ) ) {
            menu.addAction( tr( "Export data tree to XML" ), [processor] () { processor->exportXML(); } );
            processor->addContextMenu( adprocessor::ContextMenuOnNavigator, menu, find_t< portfolio::Folium >()( index ) );
        }
    }

    // spectral subtraction using two selected spectra
    if ( selRows.size() == 2 ) {
        std::vector< std::tuple< QModelIndex
                                 , Dataprocessor *
                                 , portfolio::Folium
                                 , QString > > operand;
        for ( size_t i = 0; i < selRows.size(); ++i ) {
            auto [ processor, folium ] = find_processor_t< portfolio::Folium >()( selRows[ i ] );
            if ( ( processor && folium ) && folium.parentFolder().name() == L"Spectra" ) {
                operand.emplace_back( selRows[ i ], processor, folium, QString::fromStdWString( folium.name() ) );
            }
        }
        if ( operand.size() == 2 ) {
            int subtrahend = -1, minuend = -1;
            if ( auto active_processor = SessionManager::instance()->getActiveDataprocessor() ) {
                if ( auto active_folium = active_processor->currentSelection() ) {
                    minuend = std::get< 2 >( operand[ 0 ] ).name() == active_folium.name() ? 0 : 1;
                    subtrahend = minuend == 0 ? 1 : 0 ;
                    menu.addAction(
                        QString( tr("Subtract '%1' from '%2'") )
                        .arg( std::get< 3 >( operand[ subtrahend ] ), std::get< 3 >( operand[ minuend ] ) )
                        , BackgroundSubtraction( std::get< 2 >( operand[ subtrahend ] )
                                                 , std::get< 2 >( operand[ minuend ] )
                                                 , active_processor ) );
                }
            }
        }
    }

    menu.addSeparator();

    menu.addAction( tr( "Delete removed items"), [=]{ delete_removed{ selRows, pModel_ }(); } );
    menu.addAction( tr( "Collapse all"), [&]{ pTreeView_->collapseAll(); } );

    menu.exec( globalPos );
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
                        // ADDEBUG() << "-----> " << item->data( Qt::EditRole ).toString().toStdString();
                        if ( item->isCheckable() )
                            item->setCheckState( checked ? Qt::Checked : Qt::Unchecked );
                    }
                }
            }
        }
    }
}

void
NavigationWidget::handleAllCheckState( bool checked, const QString& node, const QString& exclude )
{
    QStandardItemModel& model = *pModel_;
    QRegularExpression re(exclude);

    for ( int row = 0; row < model.rowCount(); ++row ) {
        auto parent = model.itemFromIndex( model.index( row, 0 ) );
        for ( int n = 0; n < parent->rowCount(); ++n ) {
            if ( model.data( model.index( n, 0, parent->index() ) ).toString() == node ) {
                auto sp = model.itemFromIndex( model.index( n, 0, parent->index() ) );
                for ( int isp = 0; isp < sp->rowCount(); ++isp ) {
                    if ( auto item = model.itemFromIndex(model.index(isp, 0, sp->index())) ) {
                        auto m = re.match( item->data( Qt::EditRole ).toString() );
                        if ( item->isCheckable() && !m.hasMatch() )
                            item->setCheckState( checked ? Qt::Checked : Qt::Unchecked );
                    }
                }
            }
        }
    }
}

void
NavigationWidget::handleCheckAllSpectra()
{
    handleAllCheckState( true, "Spectra" );
}

void
NavigationWidget::handleUncheckAllSpectra()
{
    handleAllCheckState( false, "Spectra" );
}

void
NavigationWidget::handleCheckAllXICs()
{
    handleAllCheckState( true, "Chromatograms", ".*/TIC.[0-9]" );
}

void
NavigationWidget::handleUncheckAllXICs()
{
    handleAllCheckState( false, "Chromatograms" );
}

bool
NavigationWidget::eventFilter( QObject * obj, QEvent * ev )
{
    if ( ev->type() == QEvent::KeyPress ) {
        auto ke = static_cast< QKeyEvent * >( ev );
        if ( ke->matches( QKeySequence::Copy ) ) {
            auto selRows = pTreeView_->selectionModel()->selectedRows();
            selected_folders selFolders( selRows );
            if ( selFolders.contains( "Chromatograms" ) ) {
                copy_chromatogram_generator copy_generator( pTreeView_ );
                copy_generator();
                return true;
            }
        }
    }
    return QObject::eventFilter( obj, ev );
}

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

Q_DECLARE_METATYPE( portfolio::Folium )
Q_DECLARE_METATYPE( portfolio::Folder )
Q_DECLARE_METATYPE( dataproc::Dataprocessor * )
