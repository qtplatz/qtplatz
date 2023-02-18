// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include "utils_global.h"

#include "pathchooser.h"

#include <QStyledItemDelegate>

namespace Utils {

class QTCREATOR_UTILS_EXPORT AnnotatedItemDelegate : public QStyledItemDelegate
{
public:
    AnnotatedItemDelegate(QObject *parent = nullptr);
    ~AnnotatedItemDelegate() override;

    void setAnnotationRole(int role);
    int annotationRole() const;

    void setDelimiter(const QString &delimiter);
    const QString &delimiter() const;

protected:
    void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    int m_annotationRole;
    QString m_delimiter;
};

class QTCREATOR_UTILS_EXPORT PathChooserDelegate : public QStyledItemDelegate
{
public:
    explicit PathChooserDelegate(QObject *parent = nullptr);

    void setExpectedKind(PathChooser::Kind kind);
    void setPromptDialogFilter(const QString &filter);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setHistoryCompleter(const QString &key);

private:
    PathChooser::Kind m_kind = PathChooser::ExistingDirectory;
    QString m_filter;
    QString m_historyKey;
};

class QTCREATOR_UTILS_EXPORT CompleterDelegate : public QStyledItemDelegate
{
public:
    CompleterDelegate(const QStringList &candidates, QObject *parent = nullptr);
    CompleterDelegate(QAbstractItemModel *model, QObject *parent = nullptr);
    CompleterDelegate(QCompleter *completer, QObject *parent = nullptr);
    ~CompleterDelegate() override;

    CompleterDelegate(const CompleterDelegate &other) = delete;
    CompleterDelegate(CompleterDelegate &&other) = delete;

    CompleterDelegate &operator=(const CompleterDelegate &other) = delete;
    CompleterDelegate &operator=(CompleterDelegate &&other) = delete;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const
                              QModelIndex &index) const override;

private:
    QCompleter *m_completer = nullptr;
};

} // Utils
