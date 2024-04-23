/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Copyright (C) 2022 Raphaël Cotty <raphael.cotty@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qbs.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PROBESRESOLVER_H
#define PROBESRESOLVER_H

#include <language/forward_decls.h>
#include <tools/codelocation.h>

#include <QString>

namespace qbs::Internal {
class Item;
class LoaderState;
class ProductContext;

class ProbesResolver
{
public:
    explicit ProbesResolver(LoaderState &loaderState);
    void resolveProbes(ProductContext &productContext, Item *item);

private:
    ProbeConstPtr findOldProjectProbe(const QString &globalId, bool condition,
                                      const QVariantMap &initialProperties,
                                      const QString &sourceCode) const;
    ProbeConstPtr findOldProductProbe(const QString &productName, bool condition,
                                      const QVariantMap &initialProperties,
                                      const QString &sourceCode) const;
    ProbeConstPtr findCurrentProbe(const CodeLocation &location, bool condition,
                                   const QVariantMap &initialProperties) const;
    enum class CompareScript { No, Yes };
    bool probeMatches(const ProbeConstPtr &probe, bool condition,
                      const QVariantMap &initialProperties, const QString &configureScript,
                      CompareScript compareScript) const;
    void resolveProbe(ProductContext &productContext, Item *parent, Item *probe);

    LoaderState &m_loaderState;
};

} // namespace qbs::Internal

#endif // PROBESRESOLVER_H
