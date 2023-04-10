/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "document.hpp"
#include "mspeakwidget.hpp"
#include "mspeaktree.hpp"
#include "mol.hpp"
#include <adportable/debug.hpp>
#include <QBoxLayout>
#include <QSplitter>
#include <QSvgWidget>
#include <QSvgRenderer>
#include <QByteArray>

using lipidid::MSPeakWidget;
using lipidid::MSPeakTree;

namespace lipidid {
    class MSPeakWidget::impl {
    public:
        void draw( const QString& key, QWidget * p ) {
            if ( auto svg = document::instance()->find_svg( key.toStdString() ) ) {
                if ( auto view = p->findChild< QSvgWidget * >() ) {
                    QByteArray ba( svg->data(), svg->size() );
                    view->load( ba );
                }
            }
        }
    };
}

MSPeakWidget::~MSPeakWidget()
{
}

MSPeakWidget::MSPeakWidget( QWidget * parent ) : QWidget( parent )
                                               , impl_( std::make_unique< impl >() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {
        layout->setContentsMargins( {} );
        layout->setSpacing(2);
        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( new MSPeakTree( this ) );
            splitter->addWidget( new QSvgWidget( this ) );
            splitter->setStretchFactor( 0, 2 );
            splitter->setStretchFactor( 1, 1 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
    if ( auto tree = findChild< MSPeakTree * >() ) {
        connect( tree, &MSPeakTree::inChIKeySelected, [&](const QString& key){
            impl_->draw( key, this );});
    }
}

void
MSPeakWidget::onInitialUpdate()
{
}

MSPeakTree *
MSPeakWidget::treeView()
{
    return findChild< MSPeakTree * >();
}
