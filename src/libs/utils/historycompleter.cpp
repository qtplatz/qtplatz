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

#include "historycompleter.h"
#include "fancylineedit.h"

#include "qtcassert.h"

#include <QSettings>

#include <QItemDelegate>
#include <QKeyEvent>
#include <QListView>
#include <QPainter>

namespace Utils {
namespace Internal {

static QSettings *theSettings = 0;

class HistoryCompleterPrivate : public QAbstractListModel
{
public:
    HistoryCompleterPrivate() : maxLines(30) {}

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    void clearHistory();
    void addEntry(const QString &str);

    QStringList list;
    QString historyKey;
    int maxLines;
};

class HistoryLineDelegate : public QItemDelegate
{
public:
    HistoryLineDelegate(QObject *parent)
        : QItemDelegate(parent)
        , pixmap(QLatin1String(":/core/images/editclear.png"))
    {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QItemDelegate::paint(painter,option,index);
        QRect r = QStyle::alignedRect(option.direction, Qt::AlignRight | Qt::AlignVCenter , pixmap.size(), option.rect);
        painter->drawPixmap(r, pixmap);
    }

    QPixmap pixmap;
};

class HistoryLineView : public QListView
{
public:
    HistoryLineView(HistoryCompleterPrivate *model_)
        : model(model_)
    {
        HistoryLineDelegate *delegate = new HistoryLineDelegate(this);
        pixmapWidth = delegate->pixmap.width();
        setItemDelegate(delegate);
    }

private:
    void mousePressEvent(QMouseEvent *event)
    {
        int rr= event->x();
        if (layoutDirection() == Qt::LeftToRight)
            rr = viewport()->width() - event->x();
        if (rr < pixmapWidth) {
            model->removeRow(indexAt(event->pos()).row());
            return;
        }
        QListView::mousePressEvent(event);
    }

    HistoryCompleterPrivate *model;
    int pixmapWidth;
};

} // namespace Internal

using namespace Internal;

int HistoryCompleterPrivate::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : list.count();
}

QVariant HistoryCompleterPrivate::data(const QModelIndex &index, int role) const
{
    if (index.row() >= list.count() || index.column() != 0)
        return QVariant();
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return list.at(index.row());
    return QVariant();
}

bool HistoryCompleterPrivate::removeRows(int row, int count, const QModelIndex &parent)
{
    QTC_ASSERT(theSettings, return false);
    if (row + count > list.count())
        return false;
    beginRemoveRows(parent, row, row + count -1);
    for (int i = 0; i < count; ++i)
        list.removeAt(row);
    theSettings->setValue(historyKey, list);
    endRemoveRows();
    return true;
}

void HistoryCompleterPrivate::clearHistory()
{
    beginResetModel();
    list.clear();
    endResetModel();
}

void HistoryCompleterPrivate::addEntry(const QString &str)
{
    const QString entry = str.trimmed();
    if (entry.isEmpty())
        return;
    int removeIndex = list.indexOf(entry);
    beginResetModel();
    if (removeIndex != -1)
        list.removeAt(removeIndex);
    list.prepend(entry);
    list = list.mid(0, maxLines - 1);
    endResetModel();
    theSettings->setValue(historyKey, list);
}

HistoryCompleter::HistoryCompleter(const QString &historyKey, QObject *parent)
    : QCompleter(parent),
      d(new HistoryCompleterPrivate)
{
    QTC_ASSERT(!historyKey.isEmpty(), return);
    QTC_ASSERT(theSettings, return);

    d->historyKey = QLatin1String("CompleterHistory/") + historyKey;
    d->list = theSettings->value(d->historyKey).toStringList();

    setModel(d);
    setPopup(new HistoryLineView(d));
}

bool HistoryCompleter::removeHistoryItem(int index)
{
    return d->removeRow(index);
}

HistoryCompleter::~HistoryCompleter()
{
    delete d;
}

int HistoryCompleter::historySize() const
{
    return d->rowCount();
}

int HistoryCompleter::maximalHistorySize() const
{
    return d->maxLines;
}

void HistoryCompleter::setMaximalHistorySize(int numberOfEntries)
{
    d->maxLines = numberOfEntries;
}

void HistoryCompleter::clearHistory()
{
    d->clearHistory();
}

void HistoryCompleter::addEntry(const QString &str)
{
    d->addEntry(str);
}

void HistoryCompleter::setSettings(QSettings *settings)
{
    Internal::theSettings = settings;
}

} // namespace Utils
