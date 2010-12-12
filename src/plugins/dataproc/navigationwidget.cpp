//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "navigationwidget.h"
#include "dataprocessor.h"
#include "sessionmanager.h"
#include <adcontrols/datafile.h>
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
        QStandardItem * item = new QStandardItem( value );
        model.appendRow( item );
        return item;
    }

    template<class T> static QStandardItem * appendRow( QStandardItem& parent, const T& value ) {
        QStandardItem * item = new QStandardItem( value );
        parent.appendRow( item );
        return item;
    }

    template<class T> static QStandardItem * appendRow( QStandardItem& parent, const T& value, const QString& text ) {
        QStandardItem * item = new QStandardItem( value );
        if ( parent ) {
            parent->appendRow( item );
            QStandardItemModel& model = *item->model();
            if ( parent->columnCount() <= ( item->column() + 1 ) ) 
                model.insertColumn( item->column() + 1, parent->index() );
            model.setData( model.index( item->row(), item->column() + 1, parent->index() ), text );
        }
        return item;
    }
};


using namespace dataproc;

NavigationWidget::NavigationWidget(QWidget *parent) : QWidget(parent)
                                                    , pTreeView_( new QTreeView(this) )
                                                    , pModel_( new QStandardItemModel )
{
    pTreeView_->setModel( pModel_.get() );
    setFocusProxy( pTreeView_.get() );
    initView();

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget( pTreeView_.get() );
    layout->setSpacing( 0 );
    layout->setContentsMargins( 0, 0, 0, 0 );
    setLayout( layout );

    // connections
    connect( pModel_.get(), SIGNAL( modelReset() ), this, SLOT( initView() ) );
    connect( pTreeView_.get(), SIGNAL(activated(const QModelIndex&)), this, SLOT(openItem(const QModelIndex&)));

    connect( SessionManager::instance(), SIGNAL( signalSessionAdded( Dataprocessor* ) ), this, SLOT( handleSessionAdded( Dataprocessor * ) ) );

    setAutoSynchronization(true);
}

void
NavigationWidget::openItem(const QModelIndex &index)
{
    if ( index.isValid() ) {
        QVariant data = pModel_->data( index );
        /*
        const QModelIndex srcIndex = m_filter->mapToSource(index);
        if (m_dirModel->isDir(srcIndex)) {
            m_view->setRootIndex(index);
            setCurrentTitle(QDir(m_dirModel->filePath(srcIndex)));
        } else {
            const QString filePath = m_dirModel->filePath(srcIndex);
            Core::EditorManager *editorManager = Core::EditorManager::instance();
            editorManager->openEditor(filePath);
            editorManager->ensureEditorManagerVisible();
        }
        */
    }
}

void
NavigationWidget::setAutoSynchronization( bool sync )
{
    if ( autoSync_ == sync )
        return;
    autoSync_ = sync;

    Core::FileManager *fileManager = Core::ICore::instance()->fileManager();
    if ( autoSync_ ) {
        connect(fileManager, SIGNAL(currentFileChanged(QString)), this, SLOT(setCurrentFile(QString)));
        setCurrentFile(fileManager->currentFile());
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
NavigationWidget::setCurrentFile( const QString& filepath )
{
    qDebug() << "FolderNavigationWidget::setCurrentFile(" << filepath << ")";
/*
    QString dir = QFileInfo(filePath).path();
    if (dir.isEmpty())
        dir = Utils::PathChooser::homePath();

    QModelIndex dirIndex = m_dirModel->index(dir);
    QModelIndex fileIndex = m_dirModel->index(filePath);

    m_view->setRootIndex(m_filter->mapFromSource(dirIndex));
    if (dirIndex.isValid()) {
        setCurrentTitle(QDir(m_dirModel->filePath(dirIndex)));
        if (fileIndex.isValid()) {
            QItemSelectionModel *selections = m_view->selectionModel();
            QModelIndex mainIndex = m_filter->mapFromSource(fileIndex);
            selections->setCurrentIndex(mainIndex, QItemSelectionModel::SelectCurrent
                                                 | QItemSelectionModel::Clear);
            m_view->scrollTo(mainIndex);
        }
    } else {
        setCurrentTitle(QDir());
    }
*/
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

    // setCurrentItem(m_explorer->currentNode(), m_explorer->currentProject());
}

void
NavigationWidget::handleSessionAdded( Dataprocessor * processor )
{
    adcontrols::datafile& file = processor->file();
    QString filename( qtwrapper::qstring::copy( file.filename() ) );

    QStandardItemModel& model = *pModel_;

    QStandardItem * item = StandardItemHelper::appendRow( model, filename );
    item->setEditable( false );
    item->setToolTip( filename );

    if ( processor->getLCMSDataset() ) {

        QStandardItem * chro = StandardItemHelper::appendRow( *item, "Chromatograms" );
        StandardItemHelper::appendRow( *chro, "TIC" );

        StandardItemHelper::appendRow( *item, "Spectra" );
    }
}