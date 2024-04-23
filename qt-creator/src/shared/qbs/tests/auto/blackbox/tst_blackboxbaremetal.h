/****************************************************************************
**
** Copyright (C) 2020 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: https://www.qt.io/licensing/
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

#ifndef TST_BLACKBOXBAREMETAL_H
#define TST_BLACKBOXBAREMETAL_H

#include "tst_blackboxbase.h"

class TestBlackboxBareMetal : public TestBlackboxBase
{
    Q_OBJECT

public:
    TestBlackboxBareMetal();

private slots:
    void targetPlatform();

    void application_data();
    void application();

    void staticLibraryDependencies();
    void externalStaticLibraries();

    void sharedLibraries();

    void userIncludePaths();
    void systemIncludePaths();
    void distributionIncludePaths();
    void compilerIncludePaths();

    void preincludeHeaders();

    void defines();

    void compilerListingFiles_data();
    void compilerListingFiles();

    void linkerMapFile_data();
    void linkerMapFile();

    void compilerDefinesByLanguage();

    void toolchainProbe();

private:

};

#endif // TST_BLACKBOXBAREMETAL_H
