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
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/processmethod.hpp>
#include <adportable/profile.hpp>
#include <adportable/xml_serializer.hpp>
#include <adpublisher/doceditor.hpp>
#include <adpublisher/document.hpp>
#include <adfs/sqlite3.h>
#include <qtwrapper/waitcursor.hpp>
#include <xmlparser/pugixml.hpp>
#include <xmlparser/xmlhelper.hpp>
#include <xmlparser/xmlencode.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/styledbar.h>
#include <QCoreApplication>
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/spirit/include/classic.hpp>
#include <fstream>
#include <algorithm>

#ifdef Q_OS_MAC
const QString qrcpath = ":/adpublisher/images/mac";
#else
const QString qrcpath = ":/adpublisher/images/win";
#endif

namespace quan {
    namespace detail {

        struct append_class {

            template< class T > pugi::xml_node operator()( pugi::xml_node& node, const T& data ) const {
                auto child = node.append_child( "classdata" );
                child.append_attribute( "decltype" ) = typeid(data).name();
                pugi::xmlhelper helper( data );
                child.append_copy( helper.doc().select_single_node( "/boost_serialization/class" ).node() );
                return child;
            }
        };

        struct append_process_method : public boost::static_visitor<bool> {
            pugi::xml_node& node;
            append_process_method( pugi::xml_node& n ) : node( n ){}
            template<class T> bool operator()( const T& data ) const {
                append_class()( node, data );
                return true;
            }
        };
        
        struct append_column {
            pugi::xml_node& row;
            append_column( pugi::xml_node& n ) : row( n ) {}

            template<typename T> pugi::xml_node operator()( const char * typnam, const char * name, const T& value ) const {
                auto node = row.append_child( "column" );
                node.append_attribute( "name" ) = name;
                node.append_attribute( "decltype" ) = typnam;
                node.text() = value;
                return node;
            }

            template<> pugi::xml_node operator()( const char * typnam, const char * name, const std::string& value ) const {
                auto node = row.append_child( "column" );
                node.append_attribute( "name" ) = name;
                node.append_attribute( "decltype" ) = typnam;
                node.text() = xmlparser::encode( value ).c_str();
                return node;
            }

            pugi::xml_node operator()( const adfs::stmt& sql, int nCol, bool dropNull = false ) const {

                if ( sql.is_null_column( nCol ) && dropNull )
                    return pugi::xml_node();

                switch ( sql.column_type( nCol ) ) {
                case SQLITE_INTEGER:
                    return (*this)("int64_t", sql.column_name( nCol ).c_str(), sql.get_column_value< int64_t >( nCol ) );
                    break;
                case SQLITE_FLOAT:
                    return (*this)("double", sql.column_name( nCol ).c_str(), sql.get_column_value< double >( nCol ) );
                    break;
                case SQLITE_TEXT:
                    return (*this)("text", sql.column_name( nCol ).c_str(), sql.get_column_value< std::string >( nCol ).c_str() );
                    break;
                case SQLITE_BLOB: {
                    try {
                        auto uuid = sql.get_column_value< boost::uuids::uuid >( nCol );
                        return (*this)("uuid", sql.column_name( nCol ).c_str(), boost::lexical_cast<std::string>(uuid).c_str() );
                    } catch ( boost::bad_lexical_cast& ) {
                    }
                    break;
                }
                case SQLITE_NULL:
                    return (*this)("null", sql.column_name( nCol ).c_str(), 0);
                }
                return pugi::xml_node();
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

    const QString apppath = QCoreApplication::applicationDirPath() + QLatin1String( "/../share/qtplatz/xslt" );  // next to translations
    boost::filesystem::path xsltpath = boost::filesystem::path( apppath.toStdWString() ).normalize();

    auto xmldoc = std::make_shared< pugi::xml_document >();
    if ( auto comment = xmldoc->append_child( pugi::node_comment ) )
        comment.set_value( "Copyright(C) 2010-2014, MS-Cheminformatics LLC, All rights reserved." );
    auto decl = xmldoc->prepend_child( pugi::node_declaration );
    decl.append_attribute("version") = "1.0";
    decl.append_attribute("encoding") = "UTF-8";
    decl.append_attribute( "standalone" ) = "no";

#if 0 // for debug
    if ( auto pi = xmldoc->append_child( pugi::node_pi ) ) {
        pi.set_name( "xml-stylesheet" );
        pi.set_value( (boost::format("type=\"text/xsl\" href=\"%1%\"") % (xsltpath / "quan-html.xsl").generic_string()).str().c_str() );
    }
#endif

    if ( auto doc = xmldoc->append_child( "qtplatz_document" ) ) {
        doc.append_attribute( "creator" ) = "Quan.qtplatzplugin.ms-cheminfo.com";
        adcontrols::idAudit id;
        detail::append_class()(doc, id);

        if ( auto node = doc.append_child( "SampleSequence" ) ) {

            if ( auto p = conn->quanSequence() ) {
                pugi::xmlhelper helper( *p );
                if ( auto xnode = node.append_child( "classdata" ) ) {
                    xnode.append_attribute( "decltype" ) = typeid(*p).name();
                    xnode.append_copy( helper.doc().select_single_node( "/boost_serialization/class" ).node() );
                }
            }

        }

        if ( auto node = doc.append_child( "ProcessMethod" ) ) {
            if ( auto pm = conn->processMethod() ) {

                if ( auto xnode = node.append_child( "classdata" ) ) {
                    xnode.append_attribute( "decltype" ) = typeid(*pm).name();
                    detail::append_class()(xnode, pm->ident()); // idAudit

                    for ( auto& m : *pm )
                        boost::apply_visitor( detail::append_process_method( xnode ), m );
                }
            }
        }

        if ( auto node = doc.append_child( "QuanResponse" ) ) {

            node.append_attribute( "sampleType" ) = "UNK";

            if ( auto cmpds = conn->processMethod()->find< adcontrols::QuanCompounds >() ) {
                for ( auto& cmpd : *cmpds ) {

                    adfs::stmt sql( conn->db() );
                    if ( sql.prepare( "\
SELECT QuanCompound.uuid as cmpid, QuanResponse.id, QuanSample.name, sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\", QuanResponse.mass , QuanCompound.mass - QuanResponse.mass AS 'error(Da)', intensity, QuanResponse.amount, QuanCompound.description, dataSource \
FROM QuanSample, QuanResponse, QuanCompound \
WHERE QuanCompound.uuid = ? AND sampleType = 0 AND QuanResponse.idCmpd = QuanCompound.uuid AND QuanSample.id = QuanResponse.idSample" ) ) {
                        sql.bind( 1 ) = cmpd.uuid();
                        int nSelected = 0;
                        while ( sql.step() == adfs::sqlite_row ) {
                            auto rnode = node.append_child( "row" );
                            ++nSelected;
                            detail::append_column append( rnode );
                            for ( int col = 0; col < sql.column_count(); ++col ) {
                                append( sql, col );
                                if ( sql.column_name( col ) == "formula" ) {
                                    auto text = adcontrols::ChemicalFormula::formatFormula( sql.get_column_value < std::string>( col ) );
                                    append( "richtext", "formula", text );
                                }
                            }
                            
                        }
                    }
                } // for
            } //if
        } //if

        if ( auto node = doc.append_child( "QuanResponse" ) ) {

            node.append_attribute( "sampleType" ) = "STD";

            if ( auto cmpds = conn->processMethod()->find< adcontrols::QuanCompounds >() ) {
                for ( auto& cmpd : *cmpds ) {

                    adfs::stmt sql( conn->db() );
                    if ( sql.prepare( "\
SELECT QuanCompound.uuid as cmpid, QuanResponse.id, QuanSample.name, sampleType, QuanCompound.formula, QuanCompound.mass AS \"exact mass\", QuanResponse.mass , QuanCompound.mass - QuanResponse.mass AS 'error(Da)', intensity, QuanSample.level, QuanResponse.amount, QuanCompound.description, dataSource \
FROM QuanSample, QuanResponse, QuanCompound \
WHERE QuanCompound.uuid = ? AND sampleType = 1 AND QuanResponse.idCmpd = QuanCompound.uuid AND QuanSample.id = QuanResponse.idSample ORDER BY QuanSample.level" ) ) {
                        sql.bind( 1 ) = cmpd.uuid();
                        int nSelected = 0;
                        while ( sql.step() == adfs::sqlite_row ) {
                            auto rnode = node.append_child( "row" );
                            ++nSelected;
                            detail::append_column append( rnode );
                            for ( int col = 0; col < sql.column_count(); ++col ) {
                                append( sql, col );
                                if ( sql.column_name( col ) == "formula" ) {
                                    auto text = adcontrols::ChemicalFormula::formatFormula( sql.get_column_value < std::string>( col ) );
                                    append( "richtext", "formula", text );
                                }
                            }
                        }
                    }
                } // for
            } //if
        } //if

        ///////////////////////////////////

        if ( auto node = doc.append_child( "QuanCalib" ) ) {

            if ( auto cmpds = conn->processMethod()->find< adcontrols::QuanCompounds >() ) {
                for ( auto& cmpd : *cmpds ) {

                    adfs::stmt sql( conn->db() );
                    if ( sql.prepare( "SELECT QuanCompound.uuid as cmpid, formula, description, n, a, b, c, d, e, f, min_x, max_x, date \
                                          FROM QuanCalib, QuanCompound WHERE QuanCompound.uuid = ? AND idCompound = QuanCompound.id" ) ) {
                        sql.bind( 1 ) = cmpd.uuid();
                        int nSelected = 0;
                        while ( sql.step() == adfs::sqlite_row ) {
                            auto rnode = node.append_child( "row" );
                            ++nSelected;
                            detail::append_column append( rnode );
                            for ( int col = 0; col < sql.column_count(); ++col ) {
                                append( sql, col, true );  // drop null column
                                if ( sql.column_name( col ) == "formula" ) {
                                    auto text = adcontrols::ChemicalFormula::formatFormula( sql.get_column_value < std::string>( col ) );
                                    append( "richtext", "formula", text );
                                }
                            }

                            adfs::stmt sql2( conn->db() );
                            if ( sql2.prepare("\
SELECT QuanAmount.level, QuanAmount.amount, QuanResponse.intensity, QuanResponse.formula, QuanSample.dataGuid \
FROM QuanCompound, QuanSample, QuanAmount, QuanResponse \
WHERE QuanCompound.uuid = :uuid AND QuanCompound.uuid = QuanAmount.idCmpd AND QuanCompound.uuid = QuanResponse.idCmpd \
AND sampleType = 1 AND QuanResponse.idSample = QuanSample.id AND QuanAmount.level = QuanSample.level \
ORDER BY QuanAmount.level" ) ) {

                                auto response_node = rnode.append_child( "response" );

                                sql2.bind( 1 ) = cmpd.uuid();

                                while ( sql2.step() == adfs::sqlite_row ) {
                                    auto row_node = response_node.append_child( "row" );
                                    detail::append_column append2( row_node );
                                    for ( int col = 0; col < sql2.column_count(); ++col )
                                        append2( sql2, col ); // don't drop null column
                                }
                            }
                        }
                    }
                } // for
            } //if
        } //if
    }

    boost::filesystem::path path( QuanDocument::instance()->lastDataDir().toStdWString() );
    path.replace_extension( ".published.xml" );
    xmldoc->save_file( path.wstring().c_str() );

    QString output;
    adpublisher::document::apply_template( path.string().c_str()
                                           , (xsltpath / "quan-html.xsl").string().c_str()
                                           , output );
    path.replace_extension( ".html" );
    boost::filesystem::ofstream o( path );
    o << output.toStdString();
}
