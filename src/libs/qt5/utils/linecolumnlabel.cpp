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

#include "linecolumnlabel.h"

#include <QMouseEvent>

/*!
    \class Utils::LineColumnLabel

    \brief The LineColumnLabel class is a label suitable for displaying cursor
    positions, and so on, with a fixed
    width derived from a sample text.
*/

namespace Utils {

LineColumnLabel::LineColumnLabel(QWidget *parent)
    : QLabel(parent)
    , m_pressed(false)
{
}

void LineColumnLabel::setText(const QString &text, const QString &maxText)
{
    QLabel::setText(text);
    m_maxText = maxText;
}

QSize LineColumnLabel::sizeHint() const
{
    return fontMetrics().boundingRect(m_maxText).size();
}

QString LineColumnLabel::maxText() const
{
    return m_maxText;
}

void LineColumnLabel::setMaxText(const QString &maxText)
{
    m_maxText = maxText;
}

void LineColumnLabel::mousePressEvent(QMouseEvent *ev)
{
    QLabel::mousePressEvent(ev);
    if (ev->button() == Qt::LeftButton)
        m_pressed = true;
}

void LineColumnLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    QLabel::mouseReleaseEvent(ev);
    if (ev->button() != Qt::LeftButton)
        return;
    if (m_pressed && rect().contains(ev->pos()))
        emit clicked();
    m_pressed = false;
}

} // namespace Utils
