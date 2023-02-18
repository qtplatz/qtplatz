// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "persistentsettings.h"

#include "fileutils.h"
#include "qtcassert.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QRect>
#include <QRegularExpression>
#include <QStack>
#include <QTextStream>
#include <QXmlStreamAttributes>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#ifdef QT_GUI_LIB
#include <QMessageBox>
#endif

// Read and write rectangle in X11 resource syntax "12x12+4+3"
static QString rectangleToString(const QRect &r)
{
    QString result;
    QTextStream str(&result);
    str << r.width() << 'x' << r.height();
    if (r.x() >= 0)
        str << '+';
    str << r.x();
    if (r.y() >= 0)
        str << '+';
    str << r.y();
    return result;
}

static QRect stringToRectangle(const QString &v)
{
    static QRegularExpression pattern("^(\\d+)x(\\d+)([-+]\\d+)([-+]\\d+)$");
    Q_ASSERT(pattern.isValid());
    const QRegularExpressionMatch match = pattern.match(v);
    return match.hasMatch() ?
        QRect(QPoint(match.captured(3).toInt(), match.captured(4).toInt()),
              QSize(match.captured(1).toInt(), match.captured(2).toInt())) :
        QRect();
}

/*!
    \class Utils::PersistentSettingsReader

    \brief The PersistentSettingsReader class reads a QVariantMap of arbitrary,
    nested data structures from an XML file.

    Handles all string-serializable simple types and QVariantList and QVariantMap. Example:
    \code
<qtcreator>
    <data>
        <variable>ProjectExplorer.Project.ActiveTarget</variable>
        <value type="int">0</value>
    </data>
    <data>
        <variable>ProjectExplorer.Project.EditorSettings</variable>
        <valuemap type="QVariantMap">
            <value type="bool" key="EditorConfiguration.AutoIndent">true</value>
        </valuemap>
    </data>
    \endcode

    When parsing the structure, a parse stack of ParseValueStackEntry is used for each
    <data> element. ParseValueStackEntry is a variant/union of:
    \list
    \li simple value
    \li map
    \li list
    \endlist

    You can register string-serialize functions for custom types by registering them in the Qt Meta
    type system. Example:
    \code
      QMetaType::registerConverter(&MyCustomType::toString);
      QMetaType::registerConverter<QString, MyCustomType>(&myCustomTypeFromString);
    \endcode

    When entering a value element ( \c <value> / \c <valuelist> , \c <valuemap> ), entry is pushed
    accordingly. When leaving the element, the QVariant-value of the entry is taken off the stack
    and added to the stack entry below (added to list or inserted into map). The first element
    of the stack is the value of the <data> element.

    \sa Utils::PersistentSettingsWriter
*/

namespace Utils {

struct Context // Basic context containing element name string constants.
{
    Context() {}
    const QString qtCreatorElement = QString("qtcreator");
    const QString dataElement = QString("data");
    const QString variableElement = QString("variable");
    const QString typeAttribute = QString("type");
    const QString valueElement = QString("value");
    const QString valueListElement = QString("valuelist");
    const QString valueMapElement = QString("valuemap");
    const QString keyAttribute = QString("key");
};

struct ParseValueStackEntry
{
    explicit ParseValueStackEntry(QVariant::Type t = QVariant::Invalid, const QString &k = QString()) : type(t), key(k) {}
    explicit ParseValueStackEntry(const QVariant &aSimpleValue, const QString &k);

    QVariant value() const;
    void addChild(const QString &key, const QVariant &v);

    QVariant::Type type;
    QString key;
    QVariant simpleValue;
    QVariantList listValue;
    QVariantMap mapValue;
};

ParseValueStackEntry::ParseValueStackEntry(const QVariant &aSimpleValue, const QString &k) :
    type(aSimpleValue.type()), key(k), simpleValue(aSimpleValue)
{
    QTC_ASSERT(simpleValue.isValid(), return);
}

QVariant ParseValueStackEntry::value() const
{
    switch (type) {
    case QVariant::Invalid:
        return QVariant();
    case QVariant::Map:
        return QVariant(mapValue);
    case QVariant::List:
        return QVariant(listValue);
    default:
        break;
    }
    return simpleValue;
}

void ParseValueStackEntry::addChild(const QString &key, const QVariant &v)
{
    switch (type) {
    case QVariant::Map:
        mapValue.insert(key, v);
        break;
    case QVariant::List:
        listValue.push_back(v);
        break;
    default:
        qWarning() << "ParseValueStackEntry::Internal error adding " << key << v << " to "
                 << QVariant::typeToName(type) << value();
        break;
    }
}

class ParseContext : public Context
{
public:
    QVariantMap parse(const FilePath &file);

private:
    enum Element { QtCreatorElement, DataElement, VariableElement,
                   SimpleValueElement, ListValueElement, MapValueElement, UnknownElement };

    Element element(const QStringView &r) const;
    static inline bool isValueElement(Element e)
        { return e == SimpleValueElement || e == ListValueElement || e == MapValueElement; }
    QVariant readSimpleValue(QXmlStreamReader &r, const QXmlStreamAttributes &attributes) const;

    bool handleStartElement(QXmlStreamReader &r);
    bool handleEndElement(const QStringView &name);

    static QString formatWarning(const QXmlStreamReader &r, const QString &message);

    QStack<ParseValueStackEntry> m_valueStack;
    QVariantMap m_result;
    QString m_currentVariableName;
};

QVariantMap ParseContext::parse(const FilePath &file)
{
    QXmlStreamReader r(file.fileContents().value_or(QByteArray()));

    m_result.clear();
    m_currentVariableName.clear();

    while (!r.atEnd()) {
        switch (r.readNext()) {
        case QXmlStreamReader::StartElement:
            if (handleStartElement(r))
                return m_result;
            break;
        case QXmlStreamReader::EndElement:
            if (handleEndElement(r.name()))
                return m_result;
            break;
        case QXmlStreamReader::Invalid:
            qWarning("Error reading %s:%d: %s", qPrintable(file.fileName()),
                     int(r.lineNumber()), qPrintable(r.errorString()));
            return QVariantMap();
        default:
            break;
        } // switch token
    } // while (!r.atEnd())
    return m_result;
}

bool ParseContext::handleStartElement(QXmlStreamReader &r)
{
    const QStringView name = r.name();
    const Element e = element(name);
    if (e == VariableElement) {
        m_currentVariableName = r.readElementText();
        return false;
    }
    if (!ParseContext::isValueElement(e))
        return false;

    const QXmlStreamAttributes attributes = r.attributes();
    const QString key = attributes.hasAttribute(keyAttribute) ?
                attributes.value(keyAttribute).toString() : QString();
    switch (e) {
    case SimpleValueElement: {
        // This reads away the end element, so, handle end element right here.
        const QVariant v = readSimpleValue(r, attributes);
        if (!v.isValid()) {
            qWarning() << ParseContext::formatWarning(r, QString::fromLatin1("Failed to read element \"%1\".").arg(name.toString()));
            return false;
        }
        m_valueStack.push_back(ParseValueStackEntry(v, key));
        return handleEndElement(name);
    }
    case ListValueElement:
        m_valueStack.push_back(ParseValueStackEntry(QVariant::List, key));
        break;
    case MapValueElement:
        m_valueStack.push_back(ParseValueStackEntry(QVariant::Map, key));
        break;
    default:
        break;
    }
    return false;
}

bool ParseContext::handleEndElement(const QStringView &name)
{
    const Element e = element(name);
    if (ParseContext::isValueElement(e)) {
        QTC_ASSERT(!m_valueStack.isEmpty(), return true);
        const ParseValueStackEntry top = m_valueStack.pop();
        if (m_valueStack.isEmpty()) { // Last element? -> Done with that variable.
            QTC_ASSERT(!m_currentVariableName.isEmpty(), return true);
            m_result.insert(m_currentVariableName, top.value());
            m_currentVariableName.clear();
            return false;
        }
        m_valueStack.top().addChild(top.key, top.value());
    }
    return e == QtCreatorElement;
}

QString ParseContext::formatWarning(const QXmlStreamReader &r, const QString &message)
{
    QString result = QLatin1String("Warning reading ");
    if (const QIODevice *device = r.device())
        if (const auto file = qobject_cast<const QFile *>(device))
            result += QDir::toNativeSeparators(file->fileName()) + QLatin1Char(':');
    result += QString::number(r.lineNumber());
    result += QLatin1String(": ");
    result += message;
    return result;
}

ParseContext::Element ParseContext::element(const QStringView &r) const
{
    if (r == valueElement)
        return SimpleValueElement;
    if (r == valueListElement)
        return ListValueElement;
    if (r == valueMapElement)
        return MapValueElement;
    if (r == qtCreatorElement)
        return QtCreatorElement;
    if (r == dataElement)
        return DataElement;
    if (r == variableElement)
        return VariableElement;
    return UnknownElement;
}

QVariant ParseContext::readSimpleValue(QXmlStreamReader &r, const QXmlStreamAttributes &attributes) const
{
    // Simple value
    const QStringView type = attributes.value(typeAttribute);
    const QString text = r.readElementText();
    if (type == QLatin1String("QChar")) { // Workaround: QTBUG-12345
        QTC_ASSERT(text.size() == 1, return QVariant());
        return QVariant(QChar(text.at(0)));
    }
    if (type == QLatin1String("QRect")) {
        const QRect rectangle = stringToRectangle(text);
        return rectangle.isValid() ? QVariant(rectangle) : QVariant();
    }
    QVariant value;
    value.setValue(text);
    value.convert(QMetaType::type(type.toLatin1().constData()));
    return value;
}

// =================================== PersistentSettingsReader

PersistentSettingsReader::PersistentSettingsReader() = default;

QVariant PersistentSettingsReader::restoreValue(const QString &variable, const QVariant &defaultValue) const
{
    if (m_valueMap.contains(variable))
        return m_valueMap.value(variable);
    return defaultValue;
}

QVariantMap PersistentSettingsReader::restoreValues() const
{
    return m_valueMap;
}

bool PersistentSettingsReader::load(const FilePath &fileName)
{
    m_valueMap.clear();

    if (fileName.fileSize() == 0) // skip empty files
        return false;

    ParseContext ctx;
    m_valueMap = ctx.parse(fileName);
    return true;
}

/*!
    \class Utils::PersistentSettingsWriter

    \brief The PersistentSettingsWriter class serializes a QVariantMap of
    arbitrary, nested data structures to an XML file.
    \sa Utils::PersistentSettingsReader
*/

static void writeVariantValue(QXmlStreamWriter &w, const Context &ctx,
                              const QVariant &variant, const QString &key = QString())
{
    switch (static_cast<int>(variant.type())) {
    case static_cast<int>(QVariant::StringList):
    case static_cast<int>(QVariant::List): {
        w.writeStartElement(ctx.valueListElement);
        w.writeAttribute(ctx.typeAttribute, QLatin1String(QVariant::typeToName(QVariant::List)));
        if (!key.isEmpty())
            w.writeAttribute(ctx.keyAttribute, key);
        const QList<QVariant> list = variant.toList();
        for (const QVariant &var : list)
            writeVariantValue(w, ctx, var);
        w.writeEndElement();
        break;
    }
    case static_cast<int>(QVariant::Map): {
        w.writeStartElement(ctx.valueMapElement);
        w.writeAttribute(ctx.typeAttribute, QLatin1String(QVariant::typeToName(QVariant::Map)));
        if (!key.isEmpty())
            w.writeAttribute(ctx.keyAttribute, key);
        const QVariantMap varMap = variant.toMap();
        const QVariantMap::const_iterator cend = varMap.constEnd();
        for (QVariantMap::const_iterator i = varMap.constBegin(); i != cend; ++i)
            writeVariantValue(w, ctx, i.value(), i.key());
        w.writeEndElement();
    }
    break;
    case static_cast<int>(QMetaType::QObjectStar): // ignore QObjects!
    case static_cast<int>(QMetaType::VoidStar): // ignore void pointers!
        break;
    default:
        w.writeStartElement(ctx.valueElement);
        w.writeAttribute(ctx.typeAttribute, QLatin1String(variant.typeName()));
        if (!key.isEmpty())
            w.writeAttribute(ctx.keyAttribute, key);
        switch (variant.type()) {
        case QVariant::Rect:
            w.writeCharacters(rectangleToString(variant.toRect()));
            break;
        default:
            w.writeCharacters(variant.toString());
            break;
        }
        w.writeEndElement();
        break;
    }
}

PersistentSettingsWriter::PersistentSettingsWriter(const FilePath &fileName, const QString &docType) :
    m_fileName(fileName), m_docType(docType)
{ }

bool PersistentSettingsWriter::save(const QVariantMap &data, QString *errorString) const
{
    if (data == m_savedData)
        return true;
    return write(data, errorString);
}

#ifdef QT_GUI_LIB
bool PersistentSettingsWriter::save(const QVariantMap &data, QWidget *parent) const
{
    QString errorString;
    const bool success = save(data, &errorString);
    if (!success)
        QMessageBox::critical(parent,
                              QCoreApplication::translate("Utils::FileSaverBase", "File Error"),
                              errorString);
    return success;
}
#endif // QT_GUI_LIB

FilePath PersistentSettingsWriter::fileName() const
{ return m_fileName; }

//** * @brief Set contents of file (e.g. from data read from it). */
void PersistentSettingsWriter::setContents(const QVariantMap &data)
{
    m_savedData = data;
}

bool PersistentSettingsWriter::write(const QVariantMap &data, QString *errorString) const
{
    m_fileName.parentDir().ensureWritableDir();
    FileSaver saver(m_fileName, QIODevice::Text);
    if (!saver.hasError()) {
        const Context ctx;
        QXmlStreamWriter w(saver.file());
        w.setAutoFormatting(true);
        w.setAutoFormattingIndent(1); // Historical, used to be QDom.
        w.writeStartDocument();
        w.writeDTD(QLatin1String("<!DOCTYPE ") + m_docType + QLatin1Char('>'));
        w.writeComment(QString::fromLatin1(" Written by %1 %2, %3. ").
                       arg(QCoreApplication::applicationName(),
                           QCoreApplication::applicationVersion(),
                           QDateTime::currentDateTime().toString(Qt::ISODate)));
        w.writeStartElement(ctx.qtCreatorElement);
        const QVariantMap::const_iterator cend = data.constEnd();
        for (QVariantMap::const_iterator it =  data.constBegin(); it != cend; ++it) {
            w.writeStartElement(ctx.dataElement);
            w.writeTextElement(ctx.variableElement, it.key());
            writeVariantValue(w, ctx, it.value());
            w.writeEndElement();
        }
        w.writeEndDocument();

        saver.setResult(&w);
    }
    bool ok = saver.finalize();
    if (ok) {
        m_savedData = data;
    } else if (errorString) {
        m_savedData.clear();
        *errorString = saver.errorString();
    }

    return ok;
}

} // namespace Utils
