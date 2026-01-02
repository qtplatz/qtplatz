// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2023 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "sdfviewer.hpp"
#include "sdfview.hpp"
#include "sdfviewerdocument.hpp"
#include "constants.hpp"
#include <adportable/debug.hpp>
#include <adwidgets/tableview.hpp>
#include <coreplugin/icore.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/command.h>

#include <utils/filepath.h>
#include <utils/qtcassert.h>
#include <utils/utilsicons.h>
#include <utils/styledbar.h>

#include <QAction>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImageReader>
#include <QLabel>
#include <QMap>
#include <QMenu>
#include <QSpacerItem>
#include <QToolBar>
#include <QWidget>
#include <QToolButton>

using namespace Core;
using namespace Utils;

namespace sdfviewer {

    struct SDFViewer::impl {
        impl() : file_( std::make_unique< SDFViewerDocument >() ) {
        }
        QString displayName;
        std::unique_ptr< SDFViewerDocument > file_;
        std::unique_ptr< SDFView > view_;
        QWidget *toolbar;

        QToolButton *shareButton;
#if 0
        CommandAction *actionExportImage;
        CommandAction *actionMultiExportImages;
        CommandAction *actionButtonCopyDataUrl;
        CommandAction *actionBackground;
        CommandAction *actionOutline;
        CommandAction *actionFitToScreen;
        CommandAction *actionOriginalSize;
        CommandAction *actionZoomIn;
        CommandAction *actionZoomOut;
        CommandAction *actionPlayPause;
#endif
        QLabel *labelImageSize;
        QLabel *labelInfo;
    };
}

namespace {
    QToolButton *
    toolButton( QAction * action, const QString& objectName )    {
        if ( auto button = new QToolButton ) {
            button->setDefaultAction( action );
            if ( !objectName.isEmpty() ) {
                button->setObjectName( objectName );
                button->setCheckable( true );
            }
            return button;
        }
        return {};
    }

}

using namespace sdfviewer;

SDFViewer::SDFViewer() : impl_( std::make_unique< impl >() )
{
    ctor();
    ADDEBUG() << "## SDFViewer::ctor() returning ##";
}

SDFViewer::SDFViewer( std::unique_ptr< SDFViewerDocument >&& document) : impl_(std::make_unique< impl >() )
{
    impl_->file_ = std::move( document );
    ctor();
}

void
SDFViewer::ctor()
{
    ADDEBUG() << "## SDFViewer::ctor() ##";

    setContext( Core::Context(constants::SDFVIEWER_ID) );
    setWidget( new adwidgets::TableView );

    if ( auto toolBar = new Utils::StyledBar ) {
        // toolbar
        toolBar->setProperty( "topBorder", true );
        if ( auto toolBarLayout = new QHBoxLayout( toolBar ) ) {
            toolBarLayout->setContentsMargins( {} );
            toolBarLayout->setSpacing( 2 );
            if ( auto am = Core::ActionManager::instance() ) {
                Core::Context ctx( constants::SDFVIEWER_ID );
                if ( auto p = new QAction(constants::ACTION_EXPORT_IMAGE) ) {
                    connect( p, &QAction::triggered, [&](){ /* todo */ });
                    am->registerAction( p, "sdfviewer.exportimage", ctx );
                    toolBarLayout->addWidget( toolButton( p, "export" ) );
                }
            }
        }
    }
#if 0
    impl_->actionExportImage = new CommandAction(constants::ACTION_EXPORT_IMAGE, impl_->toolbar);
    impl_->actionMultiExportImages = new CommandAction(constants::ACTION_EXPORT_MULTI_IMAGES,
                                                   impl_->toolbar);
    impl_->actionButtonCopyDataUrl = new CommandAction(constants::ACTION_COPY_DATA_URL, impl_->toolbar);

    impl_->shareButton = new QToolButton;
    impl_->shareButton->setToolTip(QObject::tr("Export"));
    impl_->shareButton->setPopupMode(QToolButton::InstantPopup);
    impl_->shareButton->setIcon(Icons::EXPORTFILE_TOOLBAR.icon());
    impl_->shareButton->setProperty("noArrow", true);
    auto shareMenu = new QMenu(impl_->shareButton);
    shareMenu->addAction(impl_->actionExportImage);
    shareMenu->addAction(impl_->actionMultiExportImages);
    shareMenu->addAction(impl_->actionButtonCopyDataUrl);
    impl_->shareButton->setMenu(shareMenu);

    impl_->actionBackground = new CommandAction(constants::ACTION_BACKGROUND, impl_->toolbar);
    impl_->actionOutline = new CommandAction(constants::ACTION_OUTLINE, impl_->toolbar);
    impl_->actionFitToScreen = new CommandAction(constants::ACTION_FIT_TO_SCREEN, impl_->toolbar);
    impl_->actionOriginalSize = new CommandAction(Core::Constants::ZOOM_RESET, impl_->toolbar);
    impl_->actionZoomIn = new CommandAction(Core::Constants::ZOOM_IN, impl_->toolbar);
    impl_->actionZoomOut = new CommandAction(Core::Constants::ZOOM_OUT, impl_->toolbar);
    impl_->actionPlayPause = new CommandAction(constants::ACTION_TOGGLE_ANIMATION, impl_->toolbar);

    impl_->labelImageSize = new QLabel;
    impl_->labelInfo = new QLabel;

    auto bar = new QToolBar;
    bar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    bar->addWidget(impl_->shareButton);
    bar->addSeparator();
    bar->addAction(impl_->actionOriginalSize);
    bar->addAction(impl_->actionZoomIn);
    bar->addAction(impl_->actionZoomOut);
    bar->addAction(impl_->actionPlayPause);
    bar->addAction(impl_->actionPlayPause);
    bar->addSeparator();
    bar->addAction(impl_->actionBackground);
    bar->addAction(impl_->actionOutline);
    bar->addAction(impl_->actionFitToScreen);
    // bar->addAction(setAsDefault);

    auto horizontalLayout = new QHBoxLayout(impl_->toolbar);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->addWidget(bar);
    horizontalLayout->addItem(
        new QSpacerItem(315, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    horizontalLayout->addWidget(new StyledSeparator);
    horizontalLayout->addWidget(impl_->labelImageSize);
    horizontalLayout->addWidget(new StyledSeparator);
    horizontalLayout->addWidget(impl_->labelInfo);
#endif
}

SDFViewer::~SDFViewer()
{
}

IDocument *
SDFViewer::document() const
{
    return impl_->file_.get();
}

QWidget *
SDFViewer::toolBar()
{
    return impl_->toolbar;
}
