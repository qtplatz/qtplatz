/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "progressbar.h"

#include <utils/stylehelper.h>

#include <QPropertyAnimation>
#include <QPainter>
#include <QFont>
#include <QColor>
#include <QMouseEvent>

using namespace Core;
using namespace Core::Internal;

static const int PROGRESSBAR_HEIGHT = 13;
static const int CANCELBUTTON_WIDTH = 16;
static const int SEPARATOR_HEIGHT = 2;

ProgressBar::ProgressBar(QWidget *parent)
    : QWidget(parent), m_titleVisible(true), m_separatorVisible(true), m_cancelEnabled(true),
      m_progressHeight(0),
      m_minimum(1), m_maximum(100), m_value(1), m_cancelButtonFader(0), m_finished(false),
      m_error(false)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setMouseTracking(true);
}

ProgressBar::~ProgressBar()
{
}

bool ProgressBar::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Enter:
        {
            QPropertyAnimation *animation = new QPropertyAnimation(this, "cancelButtonFader");
            animation->setDuration(125);
            animation->setEndValue(1.0);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
        break;
    case QEvent::Leave:
        {
            QPropertyAnimation *animation = new QPropertyAnimation(this, "cancelButtonFader");
            animation->setDuration(225);
            animation->setEndValue(0.0);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
        break;
    default:
        return QWidget::event(e);
    }
    return false;
}


void ProgressBar::reset()
{
    m_value = m_minimum;
    update();
}

void ProgressBar::setRange(int minimum, int maximum)
{
    m_minimum = minimum;
    m_maximum = maximum;
    if (m_value < m_minimum || m_value > m_maximum)
        m_value = m_minimum;
    update();
}

void ProgressBar::setValue(int value)
{
    if (m_value == value
            || m_value < m_minimum
            || m_value > m_maximum) {
        return;
    }
    m_value = value;
    update();
}

void ProgressBar::setFinished(bool b)
{
    if (b == m_finished)
        return;
    m_finished = b;
    update();
}

QString ProgressBar::title() const
{
    return m_title;
}

bool ProgressBar::hasError() const
{
    return m_error;
}

void ProgressBar::setTitle(const QString &title)
{
    m_title = title;
    updateGeometry();
    update();
}

void ProgressBar::setTitleVisible(bool visible)
{
    if (m_titleVisible == visible)
        return;
    m_titleVisible = visible;
    updateGeometry();
    update();
}

bool ProgressBar::isTitleVisible() const
{
    return m_titleVisible;
}

void ProgressBar::setSeparatorVisible(bool visible)
{
    if (m_separatorVisible == visible)
        return;
    m_separatorVisible = visible;
    update();
}

bool ProgressBar::isSeparatorVisible() const
{
    return m_separatorVisible;
}

void ProgressBar::setCancelEnabled(bool enabled)
{
    if (m_cancelEnabled == enabled)
        return;
    m_cancelEnabled = enabled;
    update();
}

bool ProgressBar::isCancelEnabled() const
{
    return m_cancelEnabled;
}

void ProgressBar::setError(bool on)
{
    m_error = on;
    update();
}

QSize ProgressBar::sizeHint() const
{
    int width = 50;
    int height = PROGRESSBAR_HEIGHT + 6;
    if (m_titleVisible) {
        QFontMetrics fm(titleFont());
        width = qMax(width, fm.width(m_title) + 16);
        height += fm.height() + 4;
    }
    if (m_separatorVisible)
        height += SEPARATOR_HEIGHT;
    return QSize(width, height);
}

namespace { const int INDENT = 6; }

void ProgressBar::mousePressEvent(QMouseEvent *event)
{
    if (m_cancelEnabled) {
        if (event->modifiers() == Qt::NoModifier
            && m_cancelRect.contains(event->pos())) {
            event->accept();
            emit clicked();
            return;
        }
    }
    QWidget::mousePressEvent(event);
}

QFont ProgressBar::titleFont() const
{
    QFont boldFont(font());
    boldFont.setPointSizeF(Utils::StyleHelper::sidebarFontSize());
    boldFont.setBold(true);
    return boldFont;
}

void ProgressBar::mouseMoveEvent(QMouseEvent *)
{
    update();
}

void ProgressBar::paintEvent(QPaintEvent *)
{
    // TODO move font into Utils::StyleHelper
    // TODO use Utils::StyleHelper white

    if (bar.isNull())
        bar.load(Utils::StyleHelper::dpiSpecificImageFile(QLatin1String(":/core/images/progressbar.png")));

    double range = maximum() - minimum();
    double percent = 0.;
    if (range != 0)
        percent = (value() - minimum()) / range;
    if (percent > 1)
        percent = 1;
    else if (percent < 0)
        percent = 0;

    if (finished())
        percent = 1;

    QPainter p(this);
    QFont fnt(titleFont());
    p.setFont(fnt);
    QFontMetrics fm(fnt);

    int titleHeight = m_titleVisible ? fm.height() : 0;

    // Draw separator
    int separatorHeight = m_separatorVisible ? SEPARATOR_HEIGHT : 0;
    if (m_separatorVisible) {
        p.setPen(Utils::StyleHelper::sidebarShadow());
        p.drawLine(0,0, size().width(), 0);

        p.setPen(Utils::StyleHelper::sidebarHighlight());
        p.drawLine(1, 1, size().width(), 1);
    }

    if (m_titleVisible) {
        QRect textBounds = fm.boundingRect(m_title);
        textBounds.moveCenter(rect().center());
        int alignment = Qt::AlignHCenter;

        int textSpace = rect().width() - 8;
        // If there is not enough room when centered, we left align and
        // elide the text
        QString elidedtitle  = fm.elidedText(m_title, Qt::ElideRight, textSpace);

        QRect textRect = rect().adjusted(3, separatorHeight - 1, -3, 0);
        textRect.setHeight(titleHeight+5);

        p.setPen(QColor(0, 0, 0, 120));
        p.drawText(textRect, alignment | Qt::AlignBottom, elidedtitle);
        p.translate(0, -1);
        p.setPen(Utils::StyleHelper::panelTextColor());
        p.drawText(textRect, alignment | Qt::AlignBottom, elidedtitle);
        p.translate(0, 1);
    }

    m_progressHeight = PROGRESSBAR_HEIGHT;
    m_progressHeight += ((m_progressHeight % 2) + 1) % 2; // make odd
    // draw outer rect
    const QRect rect(INDENT - 1, titleHeight + separatorHeight + (m_titleVisible ? 4 : 3),
                     size().width() - 2 * INDENT + 1, m_progressHeight);
    Utils::StyleHelper::drawCornerImage(bar, &p, rect, 3, 3, 3, 3);

    // draw inner rect
    QColor c = Utils::StyleHelper::panelTextColor();
    c.setAlpha(180);
    p.setPen(Qt::NoPen);

    QRectF inner = rect.adjusted(2, 2, -2, -2);
    inner.adjust(0, 0, qRound((percent - 1) * inner.width()), 0);
    if (m_error) {
        QColor red(255, 60, 0, 210);
        c = red;
        // avoid too small red bar
        if (inner.width() < 10)
            inner.adjust(0, 0, 10 - inner.width(), 0);
    } else if (m_finished) {
        c = QColor(90, 170, 60);
    }

    // Draw line and shadow after the gradient fill
    if (value() > 0 && value() < maximum()) {
        p.fillRect(QRect(inner.right(), inner.top(), 2, inner.height()), QColor(0, 0, 0, 20));
        p.fillRect(QRect(inner.right(), inner.top(), 1, inner.height()), QColor(0, 0, 0, 60));
    }
    QLinearGradient grad(inner.topLeft(), inner.bottomLeft());
    grad.setColorAt(0, c.lighter(130));
    grad.setColorAt(0.4, c.lighter(106));
    grad.setColorAt(0.41, c.darker(106));
    grad.setColorAt(1, c.darker(130));
    p.setPen(Qt::NoPen);
    p.setBrush(grad);
    p.drawRect(inner);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(0, 0, 0, 30), 1));

    p.drawLine(inner.topLeft() + QPointF(0.5, 0.5), inner.topRight() + QPointF(-0.5, 0.5));
    p.drawLine(inner.topLeft() + QPointF(0.5, 0.5), inner.bottomLeft() + QPointF(0.5, -0.5));
    p.drawLine(inner.topRight() + QPointF(-0.5, 0.5), inner.bottomRight() + QPointF(-0.5, -0.5));
    p.drawLine(inner.bottomLeft() + QPointF(0.5, -0.5), inner.bottomRight() + QPointF(-0.5, -0.5));

    if (m_cancelEnabled) {
        // Draw cancel button
        p.setOpacity(m_cancelButtonFader);

        if (value() < maximum() && !m_error) {
            m_cancelRect = QRect(rect.adjusted(rect.width() - CANCELBUTTON_WIDTH + 2, 1, 0, 0));
            const bool hover = m_cancelRect.contains(mapFromGlobal(QCursor::pos()));
            const QRectF cancelVisualRect(m_cancelRect.adjusted(0, 1, -2, -2));
            QLinearGradient grad(cancelVisualRect.topLeft(), cancelVisualRect.bottomLeft());
            int intensity = hover ? 90 : 70;
            QColor buttonColor(intensity, intensity, intensity, 255);
            grad.setColorAt(0, buttonColor.lighter(130));
            grad.setColorAt(1, buttonColor.darker(130));
            p.setPen(Qt::NoPen);
            p.setBrush(grad);
            p.drawRect(cancelVisualRect);

            p.setPen(QPen(QColor(0, 0, 0, 30)));
            p.drawLine(cancelVisualRect.topLeft() + QPointF(-0.5, 0.5), cancelVisualRect.bottomLeft() + QPointF(-0.5, -0.5));
            p.setPen(QPen(QColor(0, 0, 0, 120)));
            p.drawLine(cancelVisualRect.topLeft() + QPointF(0.5, 0.5), cancelVisualRect.bottomLeft() + QPointF(0.5, -0.5));
            p.setPen(QPen(QColor(255, 255, 255, 30)));
            p.drawLine(cancelVisualRect.topLeft() + QPointF(1.5, 0.5), cancelVisualRect.bottomLeft() + QPointF(1.5, -0.5));
            p.setPen(QPen(hover ? Utils::StyleHelper::panelTextColor() : QColor(180, 180, 180), 1.2, Qt::SolidLine, Qt::FlatCap));
            p.setRenderHint(QPainter::Antialiasing, true);
            p.drawLine(cancelVisualRect.topLeft() + QPointF(4.0, 2.0), cancelVisualRect.bottomRight() + QPointF(-3.0, -2.0));
            p.drawLine(cancelVisualRect.bottomLeft() + QPointF(4.0, -2.0), cancelVisualRect.topRight() + QPointF(-3.0, 2.0));
        }
    }
}
