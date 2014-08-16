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

#include "panelswidget.hpp"
#include "paneldata.hpp"
#include "quanconstants.hpp"

#include <utils/stylehelper.h>
#include <QPainter>
#include <QLabel>
#include <QIcon>

namespace quan {

    class RootWidget : public QWidget {
    public:
        RootWidget(QWidget *parent) : QWidget(parent) {
            setFocusPolicy(Qt::NoFocus);
        }
        void paintEvent(QPaintEvent * e) {
            QWidget::paintEvent(e);

            QPainter painter(this);
            QColor light = Utils::StyleHelper::mergedColors( palette().button().color(), Qt::white, 30);
            QColor dark = Utils::StyleHelper::mergedColors( palette().button().color(), Qt::black, 85);

            painter.setPen(light);
            painter.drawLine(rect().topRight(), rect().bottomRight());
            painter.setPen(dark);
            painter.drawLine(rect().topRight() - QPoint(1,0), rect().bottomRight() - QPoint(1,0));
        }
    };

    class OnePixelBlackLine : public QWidget
    {
    public:
        OnePixelBlackLine(QWidget *parent)  : QWidget(parent) {
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            setMinimumHeight(1);
            setMaximumHeight(1);
        }
        void paintEvent(QPaintEvent *e) {
            Q_UNUSED(e);
            QPainter p(this);
            QColor fillColor = Utils::StyleHelper::mergedColors( palette().button().color(), Qt::darkBlue, 80);
            p.fillRect(contentsRect(), fillColor);
        }
    };

}

using namespace quan;

PanelsWidget::PanelsWidget( QWidget * parent ) : QScrollArea( parent )
                                               , root_( new RootWidget( this ) )
{
    // We want a 900px wide widget with and the scrollbar at the
    // side of the screen.
    //root_->setMaximumWidth( 900 );
    root_->setContentsMargins( 0, 0, 40, 0 );

    QPalette pal;
    QColor background = Utils::StyleHelper::mergedColors( palette().window().color(), Qt::white, 85);
    pal.setColor(QPalette::All, QPalette::Window, background.darker(102));
    setPalette(pal);
    pal.setColor(QPalette::All, QPalette::Window, background);
    root_->setPalette(pal);

    // The layout holding the individual panels:
    QVBoxLayout *topLayout = new QVBoxLayout(root_);
    topLayout->setMargin(0);
    topLayout->setSpacing(0);

    layout_ = new QGridLayout;
    layout_->setColumnMinimumWidth( 0, Constants::ICON_SIZE + 4 );
    layout_->setSpacing( 0 );
    topLayout->addLayout( layout_ );
    //topLayout->addStretch( 1 );
    setWidget(root_);
    setFrameStyle(QFrame::NoFrame);
    setWidgetResizable(true);
    setFocusPolicy(Qt::NoFocus);
}

PanelsWidget::~PanelsWidget()
{
}

// /*
//  * Add a widget with heading information into the grid
//  * layout of the PanelsWidget.
//  *
//  *     ...
//  * +--------+-------------------------------------------+ ABOVE_HEADING_MARGIN
//  * | icon   | name                                      |
//  * +        +-------------------------------------------+
//  * |        | line                                      |
//  * +        +-------------------------------------------+ ABOVE_CONTENTS_MARGIN
//  * |        | widget (with contentsmargins adjusted!)   |
//  * +--------+-------------------------------------------+ BELOW_CONTENTS_MARGIN
//  */
void
PanelsWidget::addPanel( std::shared_ptr< PanelData >& panel )
{
    addPanel( panel.get() );
}

void
PanelsWidget::addPanel( PanelData *panel )
{
    const int headerRow = layout_->rowCount();

    // icon:
    if (!panel->icon().isNull()) {
        QLabel *iconLabel = new QLabel(root_);
        iconLabel->setPixmap( panel->icon().pixmap( Constants::ICON_SIZE, Constants::ICON_SIZE ) );
        iconLabel->setContentsMargins( 0, Constants::ABOVE_HEADING_MARGIN, 0, 0 );
        layout_->addWidget( iconLabel, headerRow, 0, /*rowSpan=*/3, /*colSpan=*/1, Qt::AlignTop | Qt::AlignHCenter );
    }

    // name:
    QLabel *nameLabel = new QLabel(root_);
    nameLabel->setText( panel->displayName() );
    QPalette palette = nameLabel->palette();
    for (int i = QPalette::Active; i < QPalette::NColorGroups; ++i ) {
        QColor foregroundColor = palette.color(QPalette::ColorGroup(i), QPalette::Foreground);
        foregroundColor.setAlpha(110);
        palette.setBrush(QPalette::ColorGroup(i), QPalette::Foreground, foregroundColor);
    }
    nameLabel->setPalette(palette);
    nameLabel->setContentsMargins(0, Constants::ABOVE_HEADING_MARGIN, 0, 0);
    QFont f = nameLabel->font();
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() * 1.6);
    nameLabel->setFont(f);
    layout_->addWidget(nameLabel, headerRow, 1, 1, 1, Qt::AlignVCenter | Qt::AlignLeft);

    // line:
    const int lineRow(headerRow + 1);
    QWidget *line = new OnePixelBlackLine(root_);
    layout_->addWidget(line, lineRow, 1, 1, -1, Qt::AlignTop);

    // add the widget:
    const int widgetRow(lineRow + 1);
    addPanelWidget(panel, widgetRow);
    layout_->setRowStretch( lineRow + 1, 100 );
}

void
PanelsWidget::addPanelWidget(PanelData *panel, int row)
{
    if ( QWidget *widget = panel->widget() ) {
        widget->setContentsMargins( Constants::PANEL_LEFT_MARGIN, Constants::ABOVE_CONTENTS_MARGIN, 0, Constants::BELOW_CONTENTS_MARGIN );
        widget->setParent( root_ );
        layout_->addWidget( widget, row, 0, 1, 2 );
    }
    panels_.push_back( panel->shared_from_this() );
}

void
PanelsWidget::commit()
{
    emit onCommit();
}
