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

#include "doctree.hpp"
#include "document.hpp"
#include <xmlparser/pugixml.hpp>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QMessageBox>

namespace adpublisher {
    namespace detail {
        
        class docItemDelegate : public QStyledItemDelegate {
        public:
            
        };

        static const char* node_types[] = {
            "null", "document", "element", "pcdata", "cdata", "comment", "pi", "declaration"
        };

        class model_writer {
            QStandardItemModel& model;
        public:
            model_writer( QStandardItemModel& _model ) : model( _model ) {
            }

            void operator()( const pugi::xpath_node& node, QModelIndex& parent ) const {

                int row = model.rowCount( parent );

                model.insertRow( row, parent );
                model.setData( model.index( row, 0, parent ), node.node().name() );
                model.setData( model.index( row, 1, parent ), node.node().value() );

                for ( auto child : node.node().select_nodes( "./*" ) ) {
                    model.itemFromIndex( model.index( row, 0, parent ) )->setColumnCount( 1 );
                    (*this)(child, model.index( row, 0, parent ));
                }

            }
        };

    }
}

using namespace adpublisher;

docTree::~docTree()
{
}

docTree::docTree(QWidget *parent) : QTreeView(parent)
                                      , model_( new QStandardItemModel )
                                      , doc_( std::make_shared< adpublisher::document >() )
{
    setModel( model_.get() );
    setItemDelegate( new detail::docItemDelegate );
}

void
docTree::setDocument( std::shared_ptr< adpublisher::document >& t )
{
    doc_ = t;
    repaint( *(doc_->xml_document()) );
}

std::shared_ptr< adpublisher::document > 
docTree::document()
{
    return doc_;
}

void
docTree::repaint( const pugi::xml_document& doc )
{
    QStandardItemModel& model = *model_;

    std::string xml;
    doc_->save( xml );

    model.clear();
    model.setColumnCount( 1 );

    try {
        const pugi::xpath_node node = doc.select_single_node( "/article|/book" );
        
        detail::model_writer writer( *model_ );
        writer(node, QModelIndex());
        
        expandAll();

    } catch ( pugi::xpath_exception& ex ) {
        QMessageBox::warning( this, "adpublisher::docTree", ex.what() );
    }

}
