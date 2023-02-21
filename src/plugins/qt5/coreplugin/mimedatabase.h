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

#ifndef MIMEDATABASE_H
#define MIMEDATABASE_H

#include <coreplugin/core_global.h>
#include <QStringList>
#include <QSharedDataPointer>
#include <QSharedPointer>
#include <QByteArray>
#include <QMutex>
#include <QFileInfo>
#include <QPair>

QT_BEGIN_NAMESPACE
class QIODevice;
class QDebug;
QT_END_NAMESPACE

namespace Core {

class MimeTypeData;
class MimeDatabasePrivate;

namespace Internal {
    class BaseMimeTypeParser;
    class FileMatchContext;
    class MainWindow;
}

class CORE_EXPORT IMagicMatcher
{
    Q_DISABLE_COPY(IMagicMatcher)

protected:
    IMagicMatcher() {}

public:
    typedef QSharedPointer<IMagicMatcher> IMagicMatcherSharedPointer;
    typedef QList<IMagicMatcherSharedPointer> IMagicMatcherList;

    // Check for a match on contents of a file
    virtual bool matches(const QByteArray &data) const = 0;
    // Return a priority value from 1..100
    virtual int priority() const = 0;
    virtual ~IMagicMatcher() {}
};

class CORE_EXPORT MagicRule
{
    Q_DISABLE_COPY(MagicRule)
public:
    MagicRule(int startPos, int endPos);
    virtual ~MagicRule();

    virtual QString matchType() const = 0;
    virtual QString matchValue() const = 0;
    virtual bool matches(const QByteArray &data) const = 0;

    int startPos() const;
    int endPos() const;

    static QString toOffset(const QPair<int, int> &startEnd);
    static QPair<int, int> fromOffset(const QString &offset);

private:
    static const QChar kColon;

    const int m_startPos;
    const int m_endPos;
};

class CORE_EXPORT MagicStringRule : public MagicRule
{
public:
    MagicStringRule(const QString &s, int startPos, int endPos);
    virtual ~MagicStringRule();

    virtual QString matchType() const;
    virtual QString matchValue() const;
    virtual bool matches(const QByteArray &data) const;

    static const QString kMatchType;

private:
    const QByteArray m_pattern;
};

class CORE_EXPORT MagicByteRule : public MagicRule
{
public:
    MagicByteRule(const QString &s, int startPos, int endPos);
    virtual ~MagicByteRule();

    virtual QString matchType() const;
    virtual QString matchValue() const;
    virtual bool matches(const QByteArray &data) const;

    static bool validateByteSequence(const QString &sequence, QList<int> *bytes = 0);

    static const QString kMatchType;

private:
    int m_bytesSize;
    QList<int> m_bytes;
};

class CORE_EXPORT MagicRuleMatcher : public IMagicMatcher
{
    Q_DISABLE_COPY(MagicRuleMatcher)
public:
    typedef QSharedPointer<MagicRule> MagicRuleSharedPointer;
    typedef QList<MagicRuleSharedPointer> MagicRuleList;

    MagicRuleMatcher();

    void add(const MagicRuleSharedPointer &rule);
    void add(const MagicRuleList &ruleList);
    MagicRuleList magicRules() const;

    virtual bool matches(const QByteArray &data) const;

    virtual int priority() const;
    void setPriority(int p);

    // Create a list of MagicRuleMatchers from a hash of rules indexed by priorities.
    static IMagicMatcher::IMagicMatcherList createMatchers(const QHash<int, MagicRuleList> &);

private:
    MagicRuleList m_list;
    int m_priority;
};

class CORE_EXPORT MimeGlobPattern
{
public:
    static const unsigned MaxWeight = 100;
    static const unsigned MinWeight = 1;

    explicit MimeGlobPattern(const QString &pattern, unsigned weight = MaxWeight);
    ~MimeGlobPattern();

    bool matches(const QString &fileName) const;
    unsigned weight() const { return m_weight; }
    QString pattern() const { return m_pattern; }

private:
    enum { Suffix, Exact, Glob } m_type;
    QString m_pattern;
    QRegExp m_regexp; // Will be used in \c Glob case only.
    unsigned m_weight;
};


class CORE_EXPORT MimeType
{
public:
    typedef IMagicMatcher::IMagicMatcherList IMagicMatcherList;
    typedef IMagicMatcher::IMagicMatcherSharedPointer IMagicMatcherSharedPointer;

    MimeType();
    MimeType(const MimeType&);
    MimeType &operator=(const MimeType&);
    ~MimeType();

    void clear();
    bool isNull() const;
    operator bool() const;

    bool isTopLevel() const;

    QString type() const;
    void setType(const QString &type);

    QStringList aliases() const;
    void setAliases(const QStringList &);

    QString comment() const;
    void setComment(const QString &comment);

    QString localeComment(const QString &locale = QString() /* en, de...*/) const;
    void setLocaleComment(const QString &locale, const QString &comment);

    QList<MimeGlobPattern> globPatterns() const;
    void setGlobPatterns(const QList<MimeGlobPattern> &);

    QStringList subClassesOf() const;
    void setSubClassesOf(const QStringList &);

    // Extension over standard mime data
    QStringList suffixes() const;
    QString preferredSuffix() const;
    bool setPreferredSuffix(const QString&);

    // Check for type or one of the aliases
    bool matchesType(const QString &type) const;

    // Check glob patterns weights and magic priorities so the highest
    // value is returned. A 0 (zero) indicates no match.
    unsigned matchesFile(const QFileInfo &file) const;

    // Return a filter string usable for a file dialog
    QString filterString() const;

    void addMagicMatcher(const IMagicMatcherSharedPointer &matcher);

    const IMagicMatcherList &magicMatchers() const;
    void setMagicMatchers(const IMagicMatcherList &matchers);

    // Convenience for rule-base matchers.
    IMagicMatcherList magicRuleMatchers() const;
    void setMagicRuleMatchers(const IMagicMatcherList &matchers);

    friend QDebug operator<<(QDebug d, const MimeType &mt);

    static QString formatFilterString(const QString &description,
                                      const QList<MimeGlobPattern> &globs);

private:
    explicit MimeType(const MimeTypeData &d);
    unsigned matchesFileBySuffix(Internal::FileMatchContext &c) const;
    unsigned matchesFileByContent(Internal::FileMatchContext &c) const;
    unsigned matchesData(const QByteArray &data) const;

    friend class Internal::BaseMimeTypeParser;
    friend class MimeDatabasePrivate;
    QSharedDataPointer<MimeTypeData> m_d;
};

class CORE_EXPORT MimeDatabase
{
    Q_DISABLE_COPY(MimeDatabase)
public:
    typedef IMagicMatcher::IMagicMatcherList IMagicMatcherList;
    typedef IMagicMatcher::IMagicMatcherSharedPointer IMagicMatcherSharedPointer;

    static bool addMimeTypes(const QString &fileName, QString *errorMessage);
    static bool addMimeTypes(QIODevice *device, QString *errorMessage);
    static bool addMimeType(const  MimeType &mt);

    // Returns a mime type or Null one if none found
    static MimeType findByType(const QString &type);

    // Returns a mime type or Null one if none found
    static MimeType findByFile(const QFileInfo &f);

    // Returns a mime type or Null one if none found
    static MimeType findByData(const QByteArray &data);

    // Return all known suffixes
    static QStringList suffixes();
    static bool setPreferredSuffix(const QString &typeOrAlias, const QString &suffix);
    static QString preferredSuffixByType(const QString &type);
    static QString preferredSuffixByFile(const QFileInfo &f);

    static QStringList filterStrings();
    // Return a string with all the possible file filters, for use with file dialogs
    static QString allFiltersString(QString *allFilesFilter = 0);

    static QList<MimeGlobPattern> globPatterns();
    static void setGlobPatterns(const QString &typeOrAlias, const QList<MimeGlobPattern> &globPatterns);

    static IMagicMatcherList magicMatchers();
    static void setMagicMatchers(const QString &typeOrAlias, const IMagicMatcherList &matchers);

    static QList<MimeType> mimeTypes();

    // The mime types from the functions bellow are considered only in regard to
    // their glob patterns and rule-based magic matchers.
    static void syncUserModifiedMimeTypes();
    static QList<MimeType> readUserModifiedMimeTypes();
    static void writeUserModifiedMimeTypes(const QList<MimeType> &mimeTypes);
    static void clearUserModifiedMimeTypes();

    static QList<MimeGlobPattern> toGlobPatterns(const QStringList &patterns,
                                                 int weight = MimeGlobPattern::MaxWeight);
    static QStringList fromGlobPatterns(const QList<MimeGlobPattern> &globPatterns);

private:
    MimeDatabase();
    ~MimeDatabase();

    static MimeType findByFileUnlocked(const QFileInfo &f);

    friend class Core::Internal::MainWindow;
};

} // namespace Core

#endif // MIMEDATABASE_H
