// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once

#include <string>
#include <utility>
#include <vector>

std::vector<std::pair<std::string, std::string>> ei_rules = {
    { "C-C cleavage cation left",
      "[C:1]-[C:2]>>[C+:1].[C:2]" },

    { "C-C cleavage cation right",
      "[C:1]-[C:2]>>[C:1].[C+:2]" },

    { "alpha cleavage next to O/N/S",
      "[C:1]-[C:2]-[O,N,S:3]>>[C+:1].[C:2]-[O,N,S:3]" },

    { "carbonyl alpha cleavage acyl ion",
      "[C:1]-[C:2](=[O:3])-[C:4]>>[C:1].[C+:2]=[O:3].[C:4]" },

    { "C-O cleavage oxonium",
      "[C:1]-[O:2]-[C:3]>>[C:1].[O+:2]-[C:3]" },

    { "loss HX",
      "[C:1]-[C:2]([F,Cl,Br,I:3])>>[C:1]=[C:2].[F,Cl,Br,I:3]" },

    { "C-X cleavage",
      "[C:1]-[F,Cl,Br,I:2]>>[C+:1].[F,Cl,Br,I:2]" },

    { "perfluoro C-C cleavage left",
      "[C:1]([F])([F])-[C:2]([F])([F])>>[C+:1]([F])[F].[C:2]([F])([F])" },

    { "perfluoro C-C cleavage right",
      "[C:1]([F])([F])-[C:2]([F])([F])>>[C:1]([F])([F]).[C+:2]([F])[F]" }
};
