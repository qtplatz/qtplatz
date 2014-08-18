/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quanreportwidget.hpp"
#include "quanconnection.hpp"
#include "quanconstants.hpp"
#include "quandocument.hpp"
#include "quanmethodcomplex.hpp"
#include "quanquery.hpp"
#include "quanqueryform.hpp"
#include "quanresulttable.hpp"
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/profile.hpp>
#include <adportable/xml_serializer.hpp>
#include <adpublisher/doceditor.hpp>
#include <adpublisher/document.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <xmlparser/pugixml.hpp>
#include <xmlparser/xmlhelper.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <utils/styledbar.h>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <fstream>
#include <algorithm>

#ifdef Q_OS_MAC
const QString qrcpath = ":/adpublisher/images/mac";
#else
const QString qrcpath = ":/adpublisher/images/win";
#endif

namespace quan {
    namespace detail {

        struct append_process_method : public boost::static_visitor<bool> {
            pugi::xml_node& node;
            append_process_method( pugi::xml_node& n ) : node( n ){}
            template<class T> bool operator()( const T& data ) const {
                if ( auto child = node.append_child( "dataClass" ) ) {
                    child.append_attribute( "declType" ) = typeid(data).name();
                    pugi::xmlhelper helper( data );
                    child.append_copy( helper.doc().select_single_node( "/boost_serialization/class" ).node() );
                    return true;
                }
                return false;
            }
        };
    }
}

using namespace quan;

QuanReportWidget::~QuanReportWidget()
{
}

QuanReportWidget::QuanReportWidget(QWidget *parent) : QWidget(parent)
                                                    , layout_( new QVBoxLayout( this ) )
                                                    , docEditor_( new adpublisher::docEditor )
{
    if ( auto am = Core::ActionManager::instance() ) {

        if ( auto menuContainer = am->createMenu( Constants::PUBLISHER_FILE_MENU ) ) {
            menuContainer->menu()->setTitle( tr( "Publisher" ) );
            // docEditor_->setupFileActions( menuContainer->menu() );
            setupFileActions( menuContainer->menu() );
            am->actionContainer( Core::Constants::M_FILE )->addMenu( menuContainer );
        }

        if ( auto menuContainer = am->createMenu( Constants::PUBLISHER_EDIT_MENU ) ) {
            menuContainer->menu()->setTitle( tr( "Publisher" ) );
            docEditor_->setupEditActions( menuContainer->menu() );
            am->actionContainer( Core::Constants::M_EDIT )->addMenu( menuContainer );
        }

        if ( auto menuContainer = am->createMenu( Constants::PUBLISHER_TEXT_MENU ) ) {
            menuContainer->menu()->setTitle( tr( "Format" ) );
            docEditor_->setupTextActions( menuContainer->menu() );
            am->actionContainer( Core::Constants::MENU_BAR )->addMenu( menuContainer, Core::Constants::G_VIEW );
        }
        docEditor_->onInitialUpdate();
    }

    layout_->addWidget( docEditor_.get() );
    layout_->setStretch( 1, 10 );
    QSizePolicy policy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    docEditor_->setSizePolicy( policy );
}

void
QuanReportWidget::importDocTemplate()
{
    QString name = QFileDialog::getOpenFileName( this
                                                 , tr( "Import ReportFormat..." )
                                                 , QuanDocument::instance()->lastMethodDir()
                                                 , tr( "Quan Method Files(*.qmth);;XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        QuanMethodComplex m;
        QuanDocument::instance()->load( name.toStdWString(), m );
        auto ptr = m.docTemplate();
        QuanDocument::instance()->method( ptr );
    }
}

void
QuanReportWidget::exportDocTemplate()
{
    boost::filesystem::path path( QuanDocument::instance()->lastMethodDir().toStdWString() );
    path.remove_filename();
    path /= "reportTemplate.xml";

    QString name = QFileDialog::getSaveFileName( this
                                                 , tr( "Export doc template..." )
                                                 , QString::fromStdWString( path.wstring() )
                                                 , tr( "XML Files(*.xml)" ) );
    if ( !name.isEmpty() ) {
        auto doc = docEditor_->document();
        doc->save_file( name.toUtf8() );
    }
}

void
QuanReportWidget::setupFileActions( QMenu * menu )
{
    QToolBar *tb = new QToolBar( docEditor_.get() );
    tb->setWindowTitle(tr("File Actions"));
    docEditor_->addToolBar( tb );

    QAction *a;

    a = new QAction( QIcon( ":/quan/images/run.png" ), tr( "Publish" ), this );
    a->setPriority(QAction::LowPriority);
    connect(a, &QAction::triggered, this, &QuanReportWidget::filePublish );
    tb->addAction(a);
    menu->addAction( a );

    QIcon newIcon = QIcon::fromTheme("document-new", QIcon(qrcpath + "/filenew.png"));
    a = new QAction( newIcon, tr("&New"), docEditor_.get());
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(fileNew()));
    tb->addAction(a);
    menu->addAction( a );

    a = new QAction(QIcon::fromTheme("document-open", QIcon(qrcpath + "/fileopen.png")), tr("&Open..."), docEditor_.get());
    a->setShortcut(QKeySequence::Open);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(fileOpen()));
    tb->addAction(a);
    menu->addAction( a );

    menu->addSeparator();

    a = new QAction( QIcon::fromTheme( "document-save", QIcon( qrcpath + "/filesave.png" ) ), tr( "&Save" ), docEditor_.get() );
    docEditor_->setAction( adpublisher::docEditor::idActionSave, a );
    a->setShortcut(QKeySequence::Save);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(fileSave()));
    a->setEnabled(false);
    tb->addAction(a);
    menu->addAction( a );

    a = new QAction(tr("Save &As..."), docEditor_.get());
    a->setPriority(QAction::LowPriority);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(fileSaveAs()));
    menu->addAction(a);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    a = new QAction(QIcon::fromTheme("document-print", QIcon(qrcpath + "/fileprint.png")),  tr("&Print..."), docEditor_.get());
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(filePrint()));
    tb->addAction(a);
    menu->addAction( a );

    a = new QAction(QIcon::fromTheme("fileprint", QIcon(qrcpath + "/fileprint.png")), tr("Print Preview..."), docEditor_.get());
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(filePrintPreview()));
    menu->addAction( a );

    a = new QAction(QIcon::fromTheme("exportpdf", QIcon(qrcpath + "/exportpdf.png")), tr("&Export PDF..."), docEditor_.get());
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(a, SIGNAL(triggered()), docEditor_.get(), SLOT(filePrintPdf()));
    tb->addAction(a);
    menu->addAction( a );
#endif

}

void
QuanReportWidget::filePublish()
{
    auto conn = QuanDocument::instance()->connection()->shared_from_this();
    if ( !conn )
        return;

    auto xmldoc = std::make_shared< pugi::xml_document >();
    if ( auto comment = xmldoc->append_child( pugi::node_comment ) )
        comment.set_value( "Copyright(C) 2010-2014, MS-Cheminformatics LLC, All rights reserved." );


    if ( auto doc = xmldoc->append_child( "qtplatz_document" ) ) {
        doc.append_attribute( "creator" ) = "Quan.qtplatzplugin.ms-cheminfo.com";
        if ( auto node = doc.append_child( "publish" ) ) {

            adcontrols::idAudit idAudit;

            pugi::xmlhelper helper( idAudit );
            node.append_copy( helper.doc().select_single_node( "/boost_serialization/class" ).node() );
            node.append_attribute( "datasource" ) = pugi::as_utf8( conn->filepath().c_str() ).c_str();
        }

        if ( auto node = doc.append_child("SampleSequence") ) {

            if ( auto p = conn->quanSequence() ) {
                pugi::xmlhelper helper(*p);
                if ( auto xnode = node.append_child( "dataClass" ) ) {
                    xnode.append_attribute( "declType" ) = typeid(*p).name();
                    xnode.append_copy( helper.doc().select_single_node( "/boost_serialization/class" ).node() );
                }
            }

        }
        if ( auto node = doc.append_child("ProcessMethod") ) {
            if ( auto pm = conn->processMethod() ) {
                
                if ( auto xnode = node.append_child( "dataClass" ) ) {
                    xnode.append_attribute( "declType" ) = typeid(*pm).name();
                    pugi::xmlhelper helper( pm->ident() ); // idAudit
                    xnode.append_copy( helper.doc().select_single_node( "/boost_serialization/class" ).node() );

                    for ( auto& m: *pm )
                        boost::apply_visitor( detail::append_process_method( xnode ), m );
                }
            }
        }

        boost::filesystem::path path( QuanDocument::instance()->lastDataDir().toStdWString() );
        path.remove_filename();
        path /= "published.xml";

        xmldoc->save_file( path.wstring().c_str() );
    }
    
    //auto doc = docEditor_->document()->xml_document();
    //auto nodes = doc->select_nodes( "//table|//figure[@dataClass]" );
}
