// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#pragma once

#include <utils/fileutils.h>
#include <utils/textutils.h>

#include <QMap>
#include <QTextBlock>

#include <optional>
#include <vector>

namespace Utils {
class FilePath;
}

namespace TextEditor {

class ICodeStylePreferences;
class TabSettings;

using IndentationForBlock = QMap<int, int>;

class RangeInLines
{
public:
    int startLine;
    int endLine;
};

using RangesInLines = std::vector<RangeInLines>;

class Indenter
{
public:
    explicit Indenter(QTextDocument *doc)
        : m_doc(doc)
    {}

    void setFileName(const Utils::FilePath &fileName) { m_fileName = fileName; }

    virtual ~Indenter() = default;

    // Returns true if key triggers an indent.
    virtual bool isElectricCharacter(const QChar & /*ch*/) const { return false; }

    virtual void setCodeStylePreferences(ICodeStylePreferences * /*preferences*/) {}

    virtual void invalidateCache() {}

    virtual int indentFor(const QTextBlock & /*block*/,
                          const TabSettings & /*tabSettings*/,
                          int /*cursorPositionInEditor*/ = -1)
    {
        return -1;
    }

    virtual int visualIndentFor(const QTextBlock & /*block*/,
                                const TabSettings & /*tabSettings*/)
    {
        return -1;
    }

    virtual void autoIndent(const QTextCursor &cursor,
                            const TabSettings &tabSettings,
                            int cursorPositionInEditor = -1)
    {
        indent(cursor, QChar::Null, tabSettings, cursorPositionInEditor);
    }

    virtual Utils::Text::Replacements format(const RangesInLines & /*rangesInLines*/)
    {
        return Utils::Text::Replacements();
    }

    virtual bool formatOnSave() const { return false; }

    // Expects a list of blocks in order of occurrence in the document.
    virtual IndentationForBlock indentationForBlocks(const QVector<QTextBlock> &blocks,
                                                     const TabSettings &tabSettings,
                                                     int cursorPositionInEditor = -1)
        = 0;
    virtual std::optional<TabSettings> tabSettings() const = 0;

    // Indent a text block based on previous line. Default does nothing
    virtual void indentBlock(const QTextBlock &block,
                             const QChar &typedChar,
                             const TabSettings &tabSettings,
                             int cursorPositionInEditor = -1)
        = 0;

    // Indent at cursor. Calls indentBlock for selection or current line.
    virtual void indent(const QTextCursor &cursor,
                        const QChar &typedChar,
                        const TabSettings &tabSettings,
                        int cursorPositionInEditor = -1)
        = 0;

    // Reindent at cursor. Selection will be adjusted according to the indentation
    // change of the first block.
    virtual void reindent(const QTextCursor &cursor,
                          const TabSettings &tabSettings,
                          int cursorPositionInEditor = -1)
        = 0;

    virtual std::optional<int> margin() const { return std::nullopt; }

protected:
    QTextDocument *m_doc;
    Utils::FilePath m_fileName;
};

} // namespace TextEditor
