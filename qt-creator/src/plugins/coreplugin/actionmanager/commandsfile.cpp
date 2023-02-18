// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "commandsfile.h"
#include "command.h"
#include <coreplugin/dialogs/shortcutsettings.h>
#include <coreplugin/icore.h>

#include <app/app_version.h>
#include <utils/qtcassert.h>
#include <utils/fileutils.h>

#include <QKeySequence>
#include <QFile>
#include <QXmlStreamAttributes>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QDateTime>

using namespace Utils;

namespace Core {
namespace Internal {

struct Context // XML parsing context with strings.
{
    const QString mappingElement = "mapping";
    const QString shortCutElement = "shortcut";
    const QString idAttribute = "id";
    const QString keyElement = "key";
    const QString valueAttribute = "value";
};

/*!
    \class Core::Internal::CommandsFile
    \internal
    \inmodule QtCreator
    \brief The CommandsFile class provides a collection of import and export commands.
    \inheaderfile commandsfile.h
*/

/*!
    \internal
*/
CommandsFile::CommandsFile(const FilePath &filename)
    : m_filePath(filename)
{

}

/*!
    \internal
*/
QMap<QString, QList<QKeySequence>> CommandsFile::importCommands() const
{
    QMap<QString, QList<QKeySequence>> result;

    QFile file(m_filePath.toString());
    if (!file.open(QIODevice::ReadOnly|QIODevice::Text))
        return result;

    Context ctx;
    QXmlStreamReader r(&file);

    QString currentId;

    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::StartElement: {
            const auto name = r.name();
            if (name == ctx.shortCutElement) {
                currentId = r.attributes().value(ctx.idAttribute).toString();
                if (!result.contains(currentId))
                    result.insert(currentId, {});
            } else if (name == ctx.keyElement) {
                QTC_ASSERT(!currentId.isEmpty(), continue);
                const QXmlStreamAttributes attributes = r.attributes();
                if (attributes.hasAttribute(ctx.valueAttribute)) {
                    const QString keyString = attributes.value(ctx.valueAttribute).toString();
                    QList<QKeySequence> keys = result.value(currentId);
                    result.insert(currentId, keys << QKeySequence(keyString));
                }
            } // if key element
        } // case QXmlStreamReader::StartElement
        default:
            break;
        } // switch
    } // while !atEnd
    file.close();
    return result;
}

/*!
    \internal
*/

bool CommandsFile::exportCommands(const QList<ShortcutItem *> &items)
{
    FileSaver saver(m_filePath, QIODevice::Text);
    if (!saver.hasError()) {
        const Context ctx;
        QXmlStreamWriter w(saver.file());
        w.setAutoFormatting(true);
        w.setAutoFormattingIndent(1); // Historical, used to be QDom.
        w.writeStartDocument();
        w.writeDTD(QLatin1String("<!DOCTYPE KeyboardMappingScheme>"));
        w.writeComment(QString::fromLatin1(" Written by %1, %2. ").
                       arg(ICore::versionString(),
                           QDateTime::currentDateTime().toString(Qt::ISODate)));
        w.writeStartElement(ctx.mappingElement);
        for (const ShortcutItem *item : std::as_const(items)) {
            const Id id = item->m_cmd->id();
            if (item->m_keys.isEmpty() || item->m_keys.first().isEmpty()) {
                w.writeEmptyElement(ctx.shortCutElement);
                w.writeAttribute(ctx.idAttribute, id.toString());
            } else {
                w.writeStartElement(ctx.shortCutElement);
                w.writeAttribute(ctx.idAttribute, id.toString());
                for (const QKeySequence &k : item->m_keys) {
                    w.writeEmptyElement(ctx.keyElement);
                    w.writeAttribute(ctx.valueAttribute, k.toString());
                }
                w.writeEndElement(); // Shortcut
            }
        }
        w.writeEndElement();
        w.writeEndDocument();

        saver.setResult(&w);
    }
    return saver.finalize();
}

} // namespace Internal
} // namespace Core
