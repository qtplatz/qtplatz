/**************************************************************************
** Copyright (C) 2010-2025 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2025 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#include "peakdwidget.hpp"
#include "tableview.hpp"
#include "create_widget.hpp"
#include <QBoxLayout>
#include <QSplitter>
#include <QStandardItemModel>
#include <QtCore/qtmetamacros.h>

class QStandardItemModel;
class QStandardItem;
class QModelIndex;

namespace adwidgets {

    namespace peakd {

        class PeakdForm : public QWidget {
            Q_OBJECT
        public:
            PeakdForm( QWidget * parent = nullptr ) : QWidget( parent ) {
                if ( auto layout = new QVBoxLayout( this ) ) {
                    layout->setContentsMargins( {} );
                }
            }
            ~PeakdForm() {}
            QBoxLayout * topLayout_;
        };

    } // namespace peakd

    class PeakdWidget::impl {
    public:
        adwidgets::TableView * tv_;
        impl() : tv_( nullptr ) {}
    };

}

using namespace adwidgets;

PeakdWidget::PeakdWidget( QWidget * parent ) : QWidget( parent )
                                             , impl_( std::make_unique< impl >() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setContentsMargins( {} );
        layout->setSpacing(2);

        if ( QSplitter * splitter = new QSplitter ) {
            if ( auto form = add_widget( splitter, create_widget< peakd::PeakdForm >( "peakdForm" ) ) ) {
            }
            if ( auto tv = add_widget( splitter, create_widget< adwidgets::TableView >( "Abudances" ) ) ) {
                auto model = new QStandardItemModel();
                model->setRowCount( 1 );
                model->setColumnCount( 1 );
                tv->setModel( model );
                impl_->tv_ = tv;
            }
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 2 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
    // if ( auto form = findChild< adwidgets::TargetingForm * >() ) {
    //     connect( form, &adwidgets::TargetingForm::triggerProcess, [&] { emit triggered(); } );
    // }
}

PeakdWidget::~PeakdWidget()
{
}

void
PeakdWidget::OnCreate( const adportable::Configuration& )
{
}

void
PeakdWidget::OnInitialUpdate()
{
}

void
PeakdWidget::OnFinalClose()
{
}

bool
PeakdWidget::getContents( boost::any& ) const
{
    return {};
}

bool
PeakdWidget::setContents( boost::any&& )
{
    return {};
}

#include "peakdwidget.moc"
