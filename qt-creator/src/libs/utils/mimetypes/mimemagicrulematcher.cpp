// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR LGPL-3.0

#include "mimemagicrulematcher_p.h"

#include "mimetype_p.h"

using namespace Utils;
using namespace Utils::Internal;

/*!
    \internal
    \class MimeMagicRuleMatcher
    \inmodule QtCore

    \brief The MimeMagicRuleMatcher class checks a number of rules based on operator "or".

    It is used for rules parsed from XML files.

    \sa MimeType, MimeDatabase, MagicRule, MagicStringRule, MagicByteRule, GlobPattern
    \sa MimeTypeParserBase, MimeTypeParser
*/

MimeMagicRuleMatcher::MimeMagicRuleMatcher(const QString &mime, unsigned thePriority) :
    m_list(),
    m_priority(thePriority),
    m_mimetype(mime)
{
}

bool MimeMagicRuleMatcher::operator==(const MimeMagicRuleMatcher &other) const
{
    return m_list == other.m_list &&
           m_priority == other.m_priority;
}

void MimeMagicRuleMatcher::addRule(const MimeMagicRule &rule)
{
    m_list.append(rule);
}

void MimeMagicRuleMatcher::addRules(const QList<MimeMagicRule> &rules)
{
    m_list.append(rules);
}

QList<MimeMagicRule> MimeMagicRuleMatcher::magicRules() const
{
    return m_list;
}

// Check for a match on contents of a file
bool MimeMagicRuleMatcher::matches(const QByteArray &data) const
{
    for (const MimeMagicRule &magicRule : m_list) {
        if (magicRule.matches(data))
            return true;
    }

    return false;
}

// Return a priority value from 1..100
unsigned MimeMagicRuleMatcher::priority() const
{
    return m_priority;
}
