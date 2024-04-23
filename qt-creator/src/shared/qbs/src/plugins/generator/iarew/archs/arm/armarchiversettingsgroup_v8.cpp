/****************************************************************************
**
** Copyright (C) 2019 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: http://www.qt.io/licensing
**
** This file is part of Qbs.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms and
** conditions see http://www.qt.io/terms-conditions. For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "armarchiversettingsgroup_v8.h"

#include "../../iarewutils.h"

namespace qbs {
namespace iarew {
namespace arm {
namespace v8 {

constexpr int kArchiverArchiveVersion = 0;
constexpr int kArchiverDataVersion = 0;

namespace {

// Output page options.

struct OutputPageOptions final
{
    explicit OutputPageOptions(const QString &baseDirectory,
                               const ProductData &qbsProduct)
    {
        outputFile = QLatin1String("$PROJ_DIR$/")
                + gen::utils::targetBinaryPath(baseDirectory, qbsProduct);
    }

    QString outputFile;
};

} // namespace

// ArmArchiverSettingsGroup

ArmArchiverSettingsGroup::ArmArchiverSettingsGroup(
        const Project &qbsProject,
        const ProductData &qbsProduct,
        const std::vector<ProductData> &qbsProductDeps)
{
    Q_UNUSED(qbsProductDeps)

    setName(QByteArrayLiteral("IARCHIVE"));
    setArchiveVersion(kArchiverArchiveVersion);
    setDataVersion(kArchiverDataVersion);
    setDataDebugInfo(gen::utils::debugInformation(qbsProduct));

    const QString buildRootDirectory = gen::utils::buildRootPath(qbsProject);
    buildOutputPage(buildRootDirectory, qbsProduct);
}

void ArmArchiverSettingsGroup::buildOutputPage(
        const QString &baseDirectory,
        const ProductData &qbsProduct)
{
    const OutputPageOptions opts(baseDirectory, qbsProduct);
    // Add 'IarchiveOverride' item (Override default).
    addOptionsGroup(QByteArrayLiteral("IarchiveOverride"),
                    {1});
    // Add 'IarchiveOutput' item (Output filename).
    addOptionsGroup(QByteArrayLiteral("IarchiveOutput"),
                    {opts.outputFile});
}

} // namespace v8
} // namespace arm
} // namespace iarew
} // namespace qbs
