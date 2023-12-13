/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "document.hpp"
#include "chemconnection.hpp"
#include "chemquery.hpp"
#include "chemschema.hpp"
#include "chemspider.hpp"
#include <adchem/drawing.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/pugrest.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adprot/aminoacid.hpp>
#include <adportable/debug.hpp>
#include <adportable/json_helper.hpp>
#include <pug/http_client_async.hpp>
#include <app/app_version.h>
#include <qtwrapper/settings.hpp>
#include <QTextEdit>
#include <QSqlError>
#include <QSqlQuery>
#include <boost/json.hpp>
#include <future>

#if defined _MSC_VER
# pragma warning(disable:4267) // size_t to unsigned int possible loss of data (x64 int on MSC is 32bit)
#endif

#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/RDKitBase.h>
#include <GraphMol/SmilesParse/SmilesParse.h>
#include <GraphMol/SmilesParse/SmilesWrite.h>
#include <GraphMol/Descriptors/MolDescriptors.h>
#include <GraphMol/FileParsers/MolSupplier.h>
#include <GraphMol/inchi.h>
#if defined _MSC_VER
# pragma warning(default:4267) // size_t to unsigned int possible loss of data (x64 int on MSC is 32bit)
#endif

#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <filesystem>

#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

namespace chemistry {

    struct user_preference {
        static std::filesystem::path path( QSettings * settings ) {
            std::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "chemistry";
        }
    };

    struct document::impl {

        impl() : settings_( std::make_unique< QSettings >(
                                QSettings::IniFormat, QSettings::UserScope
                                , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                , QLatin1String( "chemistry" ) ) ) {
        }

        std::unique_ptr< QSettings > settings_;
        QSqlDatabase db_;

        static QSettings * settings() { return impl::instance().settings_.get(); }
        static QSqlDatabase sqlDatabase() { return impl::instance().db_; }

        static impl& instance() {
            static impl __impl;
            return __impl;
        }
    };


    static struct { std::string smiles; std::vector< std::string > synonym; } inidb[] = {
        { "C(C(C(F)(F)F)(F)F)(C(N(C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F", { "PFTBA" } }
        , { "[Cl-].[S+]1C2C=C(C=CC=2N=C2C=CC(=CC=12)N(C)C)N(C)C",                                         { "Methylene blue" } }
        , { "CN(C)C1C=CC(=CC=1)C(C1C=CC(=CC=1)N(C)C)=C1C=CC(C=C1)=[N+](C)C.[Cl-]",                        { "Methyl violet", "crystal violet" } }
        , { "CCN(CC)c1ccc2c(c1)oc-3cc(=[N+](CC)CC)ccc3c2c4ccccc4C(=O)O.[Cl-]",                            { "Rhodamine B" } }
        , { "O=C(Nc1ccc(OCC)cc1)C",                                                                       { "Phenacetin" } }
        , { "c1ccc2c(c1)c(=N)c3c([nH]2)CCCC3",                                                            { "Tacrine" } }
        , { "O=C(N)c1ccc[n+](c1)[C@@H]2O[C@@H]([C@@H](O)[C@H]2O)COP([O-])(=O)OP(=O)([O-])OC[C@H]5O[C@@H](n4cnc3c(ncnc34)N)[C@H](O)[C@@H]5O", { "NAD+" } }
        , { "O=C(N)C1CC=C[N](C=1)[C@@H]2O[C@@H]([C@@H](O)[C@H]2O)COP([O-])(=O)OP(=O)([O-])OC[C@H]5O[C@@H](n4cnc3c(ncnc34)N)[C@H](O)[C@@H]5O", { "NADH" } }
        , { "O=C(N)c1ccc[n+](c1)[C@H]2[C@H](O)[C@H](O)[C@H](O2)COP([O-])(=O)OP(=O)(O)OC[C@H]3O[C@@H](n4cnc5c4ncnc5N)[C@@H]([C@@H]3O)OP(=O)(O)O", { "NADP" } }
        , { "c1ccc2c(c1)ccc(=O)o2",                                                                       { "Coumarin" } }
        , { "CCCCCCCCCCCCCC(=O)O[C@H](CCCCCCCCCCC)CC(=O)O[C@@H]1[C@H]([C@@H](O[C@@H]([C@H]1OP(=O)(O)O)CO)OC[C@@H]2[C@H]([C@@H]([C@H]([C@H](O2)OP(=O)(O)O)NC(=O)C[C@@H](CCCCCCCCCCC)O)OC(=O)C[C@@H](CCCCCCCCCCC)O)O)NC(=O)C[C@@H](CCCCCCCCCCC)OC(=O)CCCCCCCCCCC", { "Lipid A" } }
        , { "c1cc(ccc1N)S(=O)(=O)Nc2ccc(nn2)Cl",     { "Sulfachlorpyridazine" } } // 285
        , { "COc1cc(nc(n1)OC)NS(=O)(=O)c2ccc(cc2)N", { "Sulfadimethoxine" } }     // 310
        , { "Cc1cc(nc(n1)NS(=O)(=O)c2ccc(cc2)N)C",   { "Sulfadimidine" } }        // 278
        , { "Cc1nnc(s1)NS(=O)(=O)c2ccc(cc2)N",       { "Sulfamethizole" } }       // 270
        , { "C[C@H](CCCC(C)C)[C@H]1CC[C@@H]2[C@@]1(CC[C@H]3[C@H]2CC=C4[C@@]3(CC[C@@H](C4)O)C)C", { "Cholesterol" } }
        , { "CN1C=NC2=C1C(=O)N(C(=O)N2C)C", { "Caffeine" } }
        , { "C[C@H](CCC=C(C)C)[C@H]1CC[C@@]2([C@@]1(CC[C@]34[C@H]2CC[C@@H]5[C@]3(C4)CC[C@@H](C5(C)C)OC(=O)/C=C/c6ccc(c(c6)OC)O)C)C", { "Oryzanol A" } }
        , { "C[C@H](CCC(=C)C(C)C)[C@H]1CCC2[C@@]1(CC[C@]34[C@]2(CC[C@@H]5[C@]3(C4)CC[C@@H](C5(C)C)OC(=O)/C=C/c6ccc(c(c6)OC)O)C)C", { "Oryzanol B" } }
        , { "CC(C)C(C)CCC(C)C1CCC2C1(CCC3C2CC=C4C3(CCC(C4)OC(=O)C=CC5=CC(=C(C=C5)O)OC)C)C", { "Campesteryl ferulate" } }
        , { "Cc1c(c2c(c(c1O)C)CC[C@@](O2)(C)CCC[C@H](C)CCC[C@H](C)CCCC(C)C)C", { "α-Tocopherol" } }
        , { "CC1=C(C(=O)C(=C(C1=O)C)CCC(C)(CCCC(C)CCCC(C)CCCC(C)C)O)C",        { "α-TQ",   "α-Tocopherol-quinone" } }
        , { "CC1=C(C(=C(C(=C1O)C)CCC(C)(CCCC(C)CCCC(C)CCCC(C)C)O)O)C",         { "α-THQ",  "α-Tocopherylhydroquinone" } }
        , { "CC(C)CCCC(C)CCCC(C)CCCC1(CCC2=CC(=O)C=CC2(O1)OO)C",               { "α-TOOH", "α-Tocopherol Hydroperoxide" } }
        , { "CC(C)CCC[C@@H](C)CCC[C@@H](C)CCC[C@]1(C)CCC2=C(CO)C(O)=C(C)C(C)=C2O1",  { "5-f-γ-T", "5-formyl-γ-tocopherol" } }
        , { "CC(C)CCC[C@@H](C)CCC[C@@H](C)CCC[C@]1(C)CCC2=C(CO)C(O)=C(CO)C(C)=C2O1",  { "5-f-γ-TOH" } }
        , { "CC(C)CCC[C@@H](C)CCC[C@@H](C)CCC[C@]1(C)CCC2=C(CO)C(O)=C(CO)C(CO)=C2O1",  { "5-f-g-T(OH)2" } }
        , { "Cc1cc(c(c2c1O[C@](CC2)(C)CCC[C@H](C)CCC[C@H](C)CCCC(C)C)C)O", { "β-Tocopherol" } }
        , { "Oc1cc(O)c2C(=O)C(O)= C(Oc2c1)c3ccc(O)c(O)c3", { "quercetin" } }
        , { R"**(O=C(O)[C@]2(O)C[C@@H](O)[C@@H](O)[C@H](OC(=O)\C=C\c1ccc(O)c(O)c1)C2)**", { "Chlorogenic acid" } }
        , { "C[N+](C)(C)CC([O-])=O",                  { "Betaine" } }
        , { "C/C(=N\\c1ccc(cc1)O)/O",                 { "Paracetamol" } }
        , { "CC(=O)Nc1ccc(O)cc1",                     { "Acetaminophen", "paracetamol" } }        //
        , { "COP1(=NP(=NP(=N1)(OC)OC)(OC)OC)OC",      { "Hexamethoxyphosphazine" } }
        , { "CC1=C(C(=O)C2=CC=CC=C2C1=O)CC=C(C)CCCC(C)CCCC(C)CCCC(C)C", { "VK-1", "Phylloquinone" } }
        , { "CC(C)CCCC(C)CCCC(C)CCC/C(=C/CC12C(=O)c3ccccc3C(=O)C1(O2)C)/C", { "VK1-oxide", "PHYLLOQUINONE OXIDE" } }
        , { R"(CC1=C(C(=O)c2ccccc2C1=O)C/C=C(\C)/CC/C=C(\C)/CC/C=C(\C)/CCC=C(C)C)", { "VK-2", "Menatetrenone" } }
        , { "CC1=CC(=O)c2ccccc2C1=O",                 { "VK-3", "Menadione" } }
        , { "CCCCCCCCCCCCCCCCCCCCCCCC(=O)O",          { "Tetracosanoic acid", "Lignoceric acid" } }
        , { "CCCCCCCCCCCCCCCCCC(=O)O",                { "Stearic acid" } }
        , { "CCCCCCCCCCCCCCCCCCCC(=O)O",              { "Arachidic acid" } }
        , { "COP1(=NP(=NP(=N1)(OC)OC)(OC)OC)OC", { "Hexamethoxyphosphazene", "CAS NO: 957-13-1" } }
        , { "C(C(F)F)OP1(=NP(=NP(=N1)(OCC(F)F)OCC(F)F)(OCC(F)F)OCC(F)F)OCC(F)F", { "Hexakis(2,2-Difluoroethoxy)Phosphazene", "CAS NO: 186817-57-2" } }
        , { "C(C(C(F)F)(F)F)OP1(=NP(=NP(=N1)(OCC(C(F)F)(F)F)OCC(C(F)F)(F)F)(OCC(C(F)F)(F)F)OCC(C(F)F)(F)F)OCC(C(F)F)(F)F"
            , { "Hexakis(1H, 1H, 3H-Tetrafluoropropoxy)Phosphazene" }}
        , { "C(C(C(C(C(F)F)(F)F)(F)F)(F)F)OP1(=NP(=NP(=N1)(OCC(C(C(C(F)F)(F)F)(F)F)(F)F)OCC(C(C(C(F)F)(F)F)(F)F)(F)F)(OCC(C(C(C(F)F)(F)F)(F)F)(F)F)OCC(C(C(C(F)F)(F)F)(F)F)(F)F)OCC(C(C(C(F)F)(F)F)(F)F)(F)F", { "Hexakis(1H, 1H, 5H-Octafluoropentoxy)Phosphazene" }}
        , { "C(C(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)OP1(=NP(=NP(=N1)(OCC(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)OCC(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(OCC(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)OCC(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)OCC(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F"
                    , { "Hexakis(1H, 1H, 7H-Dodecafluoroheptoxy)Phosphazene" }}
        , { "C(C(C(C(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)OP1(=NP(=NP(=N1)(OCC(C(C(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)OCC(C(C(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(OCC(C(C(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)OCC(C(C(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)OCC(C(C(C(C(C(C(C(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F"
            , { "Hexakis(1H, 1H, 9H-Perfluorononyloxy)Phosphazene" }}
        , { "c1(nc(nc(n1)C(C(C(F)(F)F)(F)F)(F)F)C(C(C(F)(F)F)(F)F)(F)F)C(C(C(F)(F)F)(F)F)(F)F"
            , { "Tris(Heptafluoropropyl)-S-Triazine" }}
        , { "[H][C@]26C[C@@H](OC(=O)c1cc(OC)c(OC)c(OC)c1)[C@H](OC)[C@@H](C(=O)OC)[C@@]2([H])C[C@]5([H])c4[nH]c3cc(OC)ccc3c4CCN5C6"
            , { "Reserpine" }}
        , { R"(CC[C@H](C)[C@@H]1[C@H](CC[C@@]2(O1)C[C@@H]3C[C@H](O2)C/C=C(/[C@H]([C@H](/C=C/C=C/4\CO[C@H]5[C@@]4([C@@H](C=C([C@H]5O)C)C(=O)O3)O)C)O[C@H]6C[C@@H]([C@H]([C@@H](O6)C)O[C@H]7C[C@@H]([C@H]([C@@H](O7)C)O)OC)OC)\C)C)", { "Ivermectin B1a" }}
        , { R"(C[C@H]1CC[C@]2(C[C@@H]3C[C@H](O2)C/C=C(/[C@H]([C@H](/C=C/C=C/4\CO[C@H]5[C@@]4([C@@H](C=C([C@H]5O)C)C(=O)O3)O)C)O[C@H]6C[C@@H]([C@H]([C@@H](O6)C)O[C@H]7C[C@@H]([C@H]([C@@H](O7)C)O)OC)OC)\C)O[C@@H]1C(C)C)", { "Ivermectin B1b" } }
        , { R"([C@H]1(/C=C/[C@@H](O)CCCCC)C(=O)C[C@H](O)[C@@H]1C/C=C\CCCC(=O)O)", { "PGD2", "Prostaglandin D2" } }
        , { R"([C@H]1(/C=C/[C@@H](O)CCCCC)[C@H](O)CC(=O)[C@@H]1C/C=C\CCCC(=O)O)", { "PGE2", "Prostaglandin E2" } }
        , { R"(O=C(O)CCC/C=C\C[C@H]1[C@@H](O)C[C@@H](O)[C@@H]1/C=C/[C@@H](O)CCCCC)", { "PGF2" } }
        , { R"(O=C(O)CCCC(OO)/C=C/C=C\C\C=C/C\C=C/CCCCC)", { "5-HPETE", "5-hydroperoxyeicosatetraenoic acid" } }
        , { R"(CCCCC/C=C\C/C=C\C=C\C=C\[C@H]1[C@@H](O1)CCCC(=O)O)", { "LTA4", "Leukotriene A4" } }
        , { R"(CCCCC/C=C\C/C=C\C=C\C=C\[C@H]([C@H](CCCC(=O)O)O)SC[C@@H](/C(=N/CC(=O)O)/O)/N=C(\CC[C@@H](C(=O)O)N)/O)", { "LTC4", "Leukotriene C4" } }
        , { R"(CCCCC\C=C/C\C=C/C=C/C=C/[C@@H](SC[C@H](N)C(=O)NCC(=O)O)[C@@H](O)CCCC(=O)O)", { "LTD4", "Leukotriene D4" } }
        , { R"(CCCCC/C=C\C/C=C\C=C\C=C\[C@H]([C@H](CCCC(=O)O)O)SCC(C(=O)O)N)", { "LTE4", "Leukotriene E4" } }
        , { R"(O[C@@H]1[C@H](CC(CCCCC(O)=O)=O)[C@@H](/C=C/[C@@H](O)CCCCC)[C@H](O)C1)", { "PGF1a", "6-keto Prostaglandin F1α" } }
        , { R"(OC(=O)CCC\C=C1\C[C@@H]2[C@@H](/C=C/[C@@H](O)CCCCC)[C@H](O)C[C@@H]2O1)", { "PGI2", "Prostacyclin" } }
        , { R"(CCCCC[C@@H](/C=C/[C@@H]1[C@H]([C@@H]2C[C@@H](O2)O1)C/C=C\CCCC(=O)O)O)", { "Thromboxane A2", "TXA2" } }
        , { R"(CCCCC[C@H](O)\C=C\[C@H]1OC(O)C[C@H](O)[C@@H]1C\C=C/CCCC(O)=O)", { "Thrombxane B2", "TXB2" } }
        , { R"(CCOC(=O)c1ccccc1C(=O)OCC)",                    { "DEP" } } // 222.089
        , { R"(CCCOC(=O)c1ccccc1C(=O)OCCC)",                  { "DPP", "dipropyl pthalate" } } // 250.121
        , { R"(CCCCOC(=O)c1ccccc1C(=O)OCCCC)",                { "DBP" } } // 278.152
        , { R"(CCCCCOC(=O)c1ccccc1C(=O)OCCCCC)",              { "DnPP", "Di-n-pentyl pthalate" } } // 312.136
        , { R"(CCCCOC(=O)c1ccccc1C(=O)OCc2ccccc2)",           { "BBP", "benzylbutyl pthalate" } }  // 312.136
        , { R"(c1ccc(c(c1)C(=O)OC2CCCCC2)C(=O)OC3CCCCC3)",    { "DCHP", "dicycrohexyl pthalate" } } // 330.183
        , { R"(CCCCCCOC(=O)c1ccccc1C(=O)OCCCCCC)",            { "DHP" } } // 334.214
        , { R"(O=C(OCC(CC)CCCC)C1=CC=CC=C1C(OCC(CC)CCCC)=O)", { "DEHP", "Bis(2-ethylhexyl) phthalate", "diethylhexyl phthalate", "DOP" } } // 390.277
        , { R"(CC(C)CCC[C@@H](C)CCC[C@@H](C)CCCC(C)CCC1=C(C)C(=O)C(C)=C(C)C1=O)", { "a-TQ-OH" } } // 390.277
        , { R"(NC(C1=CC=CC=C1)(C2=CC=CC=C2)C3=CC=CC=C3)", { "TPMA" } }
        , { R"(O=S(C1=CC=C(C2=CC3=C(C4=C(C53C6=C(C=CC(C7=CC=C(S(=O)(O)=O)C=C7)=C6)C8=C5C=C(C9=CC=C(S(=O)(O)=O)C=C9)C=C8)C=C(C%10=CC=C(S(=O)(O)=O)C=C%10)C=C4)C=C2)C=C1)(O)=O)", { "spiroBPS" } }
        , { R"(CCCCCCCC/C=C\CCCCCCCC(=O)N)", { "Oleamide" } }
        , { R"(CCCCCCCCCCCCC/C=C/[C@H]([C@H](CO)N)O)", { "Sphingosine" } }
        , { R"(C1=CC=C2C=C3C=CC=CC3=CC2=C1)",                            { "Anthracene" } }
        , { R"(c1ccc2c(c1)ccc3c2cccc3)",                                 { "Phenanthrene" } }
        , { R"(c1cc2ccc3ccc4ccc5ccc6ccc1c7c2c3c4c5c67)",                 { "Coronene" } }
        , { R"(c1ccc2c3ccccc3Cc2c1)",                                    { "Fluorene" } }
        , { R"(c1ccc-2c(c1)-c3cccc4c3c2ccc4)",                           { "Fluoranthene" } }
        , { R"(c1cc2cccc3ccc4cccc1c4c32)",                               { "Pyrene" } }
        , { R"(C1=CC=C2C=CC=CC2=C1)",                                    { "Naphthalene" } }
        , { R"(C1CC2=CC=CC3=C2C1=CC=C3)",                                { "Acenaphthene" }}
        , { R"(C1=CC2=C3C(=C1)C=CC3=CC=C2)",                             { "Acenaphthylene" }}
        , { R"(C1C2=CC=CC=C2C3=CC=CC=C31)",                              { "Fluorene"}}
        , { R"(C1=CC=C2C=C3C=CC=CC3=CC2=C1)",                            { "Anthracene" }}
        , { R"(C1=CC=C2C(=C1)C=CC3=CC=CC=C32)",                          { "Phenanthrene" }}
        , { R"(C1=CC=C2C(=C1)C3=CC=CC4=C3C2=CC=C4)",                     { "Fluoranthene" }}
        , { R"(C1=CC2=C3C(=C1)C=CC4=CC=CC(=C43)C=C2)",                   { "Pyrene" }}
        , { R"(C1=CC=C2C(=C1)C=CC3=CC4=CC=CC=C4C=C32)",                  { "Benz(a)anthracene" }}
        , { R"(C1=CC=C2C(=C1)C=CC3=C2C=CC4=CC=CC=C43)",                  { "Chrysene" }}
        , { R"(C1=CC=C2C3=C4C(=CC=C3)C5=CC=CC=C5C4=CC2=C1)",             { "Benzo(b)fluoranthene" }}
        , { R"(C1=CC=C2C=C3C4=CC=CC5=C4C(=CC=C5)C3=CC2=C1)",             { "Benzo(k)fluoranthene" }}
        , { R"(C1=CC2=C3C(=C1)C4=CC=CC5=C4C(=CC=C5)C3=CC=C2)",           { "Perylene" }}
        , { R"(C1=CC=C2C3=C4C(=CC2=C1)C=CC5=C4C(=CC=C5)C=C3)",           { "Benzo(a)pyrene" }}
        , { R"(C1=CC=C2C(=C1)C3=CC=CC4=C3C5=C(C=CC=C25)C=C4)",           { "Benzo(e)pyrene" }}
        , { R"(C1=CC2=C3C(=C1)C4=CC=CC5=C4C6=C(C=C5)C=CC(=C36)C=C2)",    { "Benzo(g,h,i)perylene" }}
        , { R"(C1=CC=C2C(=C1)C3=C4C2=CC5=CC=CC6=C5C4=C(C=C6)C=C3)",      { "Indeno(1,2,3-cd)pyrene" }}
        , { R"(C1=CC=C2C(=C1)C=CC3=CC4=C(C=CC5=CC=CC=C54)C=C32)",        { "Dibenz(a,h)anthracene" }}
        , { R"(C1=CC2=C3C4=C1C=CC5=C4C6=C(C=C5)C=CC7=C6C3=C(C=C2)C=C7)", { "Coronene" }}
        , { R"(C1=C2C(=C3C=C(C(=O)C(=C3OC2=C(C(=C1I)[O-])I)I)I)C4=C(C(=C(C(=C4Cl)Cl)Cl)Cl)C(=O)[O-].[Na+].[Na+])", { "Rose bengal", "Acid Red 94" }}
        , { R"(C(=O)(C(C(C(C(C(C(C(F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)(F)F)O)", { "PFOA", "Perfluorooctanoic acid" }}
        , { R"(C(C(F)(F)F)O)", { "CF3EtOH", "2,2,2-Trifluoroethanol" }}
        , { R"(FC(C(F)(F)C(F)(F)C(F)(F)CCI)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F)", { "PFC10I", "1H,1H,2H,2H-Heptadecafluorodecyl Iodide" }}
        , { R"(FC(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)CCO)", { "PFC2-OH" }}
        , { R"(FC(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)CCO)", { "PFHEtOH", "Perfluorohexyl ethanol" }}
        , { R"(FC(F)(F)C(F)(F)C(F)(F)C(F)(F)CCO)", { "PFBEtOH" }}
        , { R"(FC=1C(F)=C(F)C(O)=C(F)C1F)", { "F5-Ph-OH" }}
        , { R"(O=C(O)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F)", { "PF14A" }}
        , { R"(O=C(O)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F)", { "PFOA" }}
        , { R"(O=C(O)C(F)(F)C(F)(F)F)", { "PFPA" }}
        , { R"(O=S(=O)(O)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F)", { "PFOS" }}
        , { R"(O=S(=O)(O)C(F)(F)C(F)(F)C(F)(F)C(F)(F)F)", { "PFBS" }}
        , { R"(O=C(OCC(COC(=O)CCC=1C=C(C(O)=C(C1)C(C)(C)C)C(C)(C)C)(COC(=O)CCC=2C=C(C(O)=C(C2)C(C)(C)C)C(C)(C)C)COC(=O)CCC=3C=C(C(O)=C(C3)C(C)(C)C)C(C)(C)C)CCC=4C=C(C(O)=C(C4)C(C)(C)C)C(C)(C)C)", { "Irganox1010" }}
        , { R"(OC=1C(=CC(=CC1C(C)(C)C)C)C(C)(C)C)", { "BHT" }}
        , { R"(CCCCC/C=C\C/C=C\CCCCCCCC(=O)O)",       { "C18:2", "Linolic acid" } }
        , { R"(O=C(O)CCC\C=C/C\C=C/C\C=C/C\C=C/C\C=C/CC)",      { "C20:5", "EPA", "Eicosapentaenoic acid" } }
        // 17941
        , { R"(OC(CCCCC/C=C\C/C=C\C/C=C\C/C=C\CCCCC)=O)",       { "C22:4", "Adrenic acid", "Docosatetraenoic acid"}}//, "90300" }}
        , { R"(CCCCC/C=C\C/C=C\C/C=C\C/C=C\CCCC(=O)O)",         { "C20:4", "Arachidonic acid"        }}//,      "90010" } }
        , { R"(CCCCC\C=C/C\C=C/C\C=C/CCCCCCC(=O)O)",            { "C20:3", "Dihomo-g-linolenic acid" }}//, "90230" }}
        , { R"(O=C(O)CC\C=C/C/C=C\C\C=C/C\C=C/C\C=C/C\C=C/CC)", { "C22:6", "Docosahexaenoic acid"    }}//,  "90310" }}
        , { R"(CC/C=C\C/C=C\C/C=C\C/C=C\C/C=C\CCCCCC(=O)O)",    { "C22:5", "Docosapentaenoic acid"   }}//, "90165" }}
        , { R"(CC/C=C\C/C=C\C/C=C\C/C=C\C/C=C\CCCC(=O)O)",      { "C20:5", "Eicosapentaenoic acid"   }}//, "90110" }}
        , { R"(CC/C=C\C/C=C\C/C=C\CCCCCCCC(=O)O)",              { "C18:3", "α-Linoleic acid"         }}//,       "90210" }}
        , { R"(CCCCCC=CCC=CCC=CCCCCC(=O)O)",                    { "GLA",   "γ-Linoleic acid", "C18:3n-6"           }}//,       "90220" }}
        , { R"(CC/C=C\C/C=C\C/C=C\C/C=C\CCCCC(=O)O)",           { "C18:4", "Stearidonic acid"        }}//,      "90320" }}
        // 17942 Saturated/Monounsaturated fatty acid MaxSpec LC-MS Mixture
        , { R"(O=C(O)CCCCCCCCCCCCCCCCCCC)",           { "C20:0", "Arachidic acid"   }}//,   "9000339",   "17942-1" }}
        , { R"(O=C(O)CCCCCCCCCCC)",                   { "C12:0", "Lauric acid"      }}//,      "10006626",  "17942-2" }}
        , { R"(O=C(O)CCCCCCCCCCCCCCCCCCCCCCC)",       { "C24:0", "Lignoceric acid"  }}//,  "13353",     "17942-3" }}
        , { R"(CCCCCCCCCCCCCC(=O)O)",                 { "C14:0", "Myristic acid"    }}//,    "13351",     "17942-4" }}
        , { R"(O=C(O)CCCCCCCCCCCCC\C=C/CCCCCCCC)",    { "C24:1", "Nervonic acid"    }}//,    "13940",     "17942-5" }}
        , { R"(CCCCCCCC\\C=C/CCCCCCCC(O)=O)",         { "C18:1", "Oleic acid"       }}//,       "90260",     "17942-6" }}
        , { R"(CCCCCCCCCCCCCCCC(=O)O)",               { "C16:0", "Palmitic acid"    }}//,    "10006627",  "17942-7" }}
        , { R"(O=C(O)CCCCCCC\C=C/CCCCCC)",            { "C16:1", "Palmitoleic acid" }}//, "10009871",  "17942-8" }}
        , { R"(CCCCCCCCCCCCCCCCCC(=O)O)",             { "C18:0", "Stearic acid"     }}//,     "10011298",  "17942-9" }}
        // 19227 Odd-chain fatty acid MaxSpec LC-MS Mixture
        , { R"(CCCCCCCCCCC(=O)O)",           { "C11:0", "Undecanoic acid" }}
        , { R"(CCCCCCCCCCCCC(=O)O)",         { "C13:0", "Tridecanoic acid" }}
        , { R"(C=CCCCCCCCCCCCCC(=O)O)",      { "C15:1", "14-Pentadecenoic acid" }}
        , { R"(CCCCCCCCCCCCCCC(=O)O)",       { "C15:0", "Pentadecanoic acid" }}
        , { R"(CCCCCCCCCCCCCCC=CC(=O)O)",    { "C17:1", "cis-10-heptadecenoic acid" }}
        , { R"(CCCCCCCCCCCCCCCCC(=O)O)",     { "C17:0", "Heptadecanoic acid" }}
        , { R"(CCCCCCCCCCCC=CCCCCCC(=O)OC)", { "C19:1", "cis-10-Nonadecenoic acid" }}
        , { R"(CCCCCCCCCCCCCCCCCCC(=O)O)",   { "C19:0", "Nonadecanoic acid" }}
        // Steroids
        , { R"(CC12CCC(CC1CCC3C2C(=O)CC4(C3CCC4C(=O)CO)C)O)",    { "APD", "alfadolone", "14107-37-0" }}
        , { R"(CC12CCC3C(C1CCC2=O)CC=C4C3(CCC(C4)O)C)",          { "DHEA", "Dehydroepiandrosterone", "PRASTERONE" }} // PA 840
        , { R"(CC12CCC3C(C1CCC2=O)CCC4=C3C=CC(=C4)O)",           { "E1", "Estrone", "53-16-7" }}
        , { R"(CC12CCC3C(C1CCC2O)CCC4=C3C=CC(=C4)O)",            { "E2", "Estradiol" }}
        , { R"(CC12CCC3C(C1CC(C2O)O)CCC4=C3C=CC(=C4)O)",         { "E3", "Estriol" }}
        , { R"(CC12CCC(CC1CCC3C2C(CC4(C3CCC4=O)C)O)O)",          { "11-OH-An", "11-α-hydroxyandrosterone" }}
        , { R"(CC12CCC3C(C1CCC2=O)C(C=C4C3(CCC(C4)O)C)O)",       { "7-OH-DHEA", "7α-hydroxydehydroepiandrosterone" }}
        , { R"(CC12CCC3C(C1CC(C2=O)O)CCC4=C3C=CC(=C4)O)",        { "16-OH-E1", "16-hydroxyestrone" }}
        , { R"(CC(=O)C1CCC2C1(CCC3C2C(C=C4C3(CCC(C4)O)C)O)C)",   { "7-OH-P5", "7α-hydroxypregnenolone" }}
        , { R"(CC(=O)C1(CCC2C1(CCC3C2CC=C4C3(CCC(C4)O)C)C)O)",   { "17-OH-P5", "17α-hydroxypregnenolone" }}
        , { R"(CC(=O)C1CCC2C1(CCC3C2CC=C4C3(CCC(C4)O)C)C)",      { "P5", "pregnenolone" }} // PA = 860
        , { R"(CC12CCC(CC1CCC3C2C(CC4(C3CCC4C(=O)CO)C)O)O)",     { "THB", "tetrahydrocorticosterone" }}
        , { R"(CC12CCC(CC1CCC3C2C(CC4(C3CCC4(C(=O)CO)O)C)O)O)",  { "TH-COL", "tetrahydrocortisol" }}
        , { R"(CC12CCC(CC1CCC3C2C(=O)CC4(C3CCC4(C(=O)CO)O)C)O)", { "TH-COR", "tetrahydrocortisone" }}
        , { R"(CC12CCC(CC1CCC3C2CCC4(C3CCC4C(=O)CO)C)O)",        { "TH-DOC", "3beta,5alpha-tetrahydrodeoxycorticosterone" }}
        , { R"(CC12CCC(CC1CCC3C2CCC4(C3CCC4(C(=O)CO)O)C)O)",     { "THS", "tetrahydrodeoxycortisol" }}
        , { R"(CC12CCC(=O)C=C1CCC3C2C(CC4(C3CCC4C(=O)CO)C)O)",   { "COB", "corticosterone" }}
        , { R"(CC12CCC(=O)C=C1CCC3C2C(=O)CC4(C3CCC4(C(=O)CO)O)C)", { "COR", "Cortisone" }}
        , { R"(CC12CCC(=O)C=C1CCC3C2CCC4(C3CCC4(C(=O)CO)O)C)",   { "COS", "11-deoxycortisol", "Cortexolone" }}
    };


}


using namespace chemistry;

std::mutex document::mutex_;

document::document(QObject *parent) : QObject(parent)
{
}

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

void
document::initialSetup()
{
    std::filesystem::path dir = user_preference::path( impl::settings() );

    if ( !std::filesystem::exists( dir ) ) {
        if ( !std::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "chemistry::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    std::filesystem::path path = qtwrapper::settings( *impl::settings() ).recentFile( "ChemistryDB", "Files" ).toStdWString();
    if ( path.empty() || path.filename() == "molecules.adfs" || path.filename() == "ChemistryDB.adfs" ) {
        path = dir / "Chemistry.db";
    }

    if ( !std::filesystem::exists( path ) ) {

        if ( !adfs::filesystem().create( path ) ) // create qtplatz ini-db
            return;
    }

    if ( auto connection = std::make_shared< ChemConnection >() ) {

        if ( connection->connect( path ) ) {

            adfs::stmt sql = connection->db();
            if ( sql.prepare( "SELECT name FROM sqlite_master WHERE type='table' AND name='mols'" ) ) {
                if ( sql.step() != adfs::sqlite_row ) {
                    if ( ChemSchema::createTables( sql ) )
                        dbInit( connection.get() );
                } else {
                    bool deprecated(false);
                    sql.prepare( "PRAGMA TABLE_INFO(mols)" );

                    while ( sql.step() == adfs::sqlite_row ) {
                        if ( sql.get_column_value< std::string >( 1 ) == "uuid" )
                            deprecated = true;
                    }

                    if ( deprecated ) {
                        sql.exec( "DROP TABLE IF EXISTS synonyms" );
                        sql.exec( "DROP TABLE IF EXISTS mols" );
                        ChemSchema::createTables( sql );
                    }

                    dbInit( connection.get() );
                }
            }

            qtwrapper::settings( *impl::settings() ).addRecentFiles( "ChemistryDB", "Files", QString::fromStdWString( path.wstring() ) );
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE", "ChemistryDB" );
    db.setDatabaseName( QString::fromStdString( path.string() ) );

    if ( db.open() ) {

        impl::instance().db_ = db;

        emit onConnectionChanged();

    } else {

        QMessageBox::critical(0, tr("Cannot open database"),
                              tr("Unable to establish a database connection.\nClick Cancel to exit.")
                              , QMessageBox::Cancel );
    }
}

void
document::finalClose()
{
    std::filesystem::path dir = user_preference::path( impl::settings() );
    if ( !std::filesystem::exists( dir ) ) {
        if ( !std::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "chemistry::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }
}

QSettings *
document::settings()
{
    return impl::settings();
}

void
document::dbUpdate( ChemConnection * connection )
{
    dbInit( connection );
}

void
document::dbInit( ChemConnection * connection )
{
    auto self( connection->shared_from_this() );

    auto query = std::make_shared< ChemQuery >( connection->db() );

    for ( auto it = adprot::AminoAcid::begin(); it != adprot::AminoAcid::end(); ++it ) {
        if ( auto mol = std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( it->smiles() ) ) ) {
            std::vector< std::string > v { it->symbol() };
            query->insert( *mol, it->smiles(), v );
        }
    }

    for ( const auto& rec : inidb ) {
        if ( auto mol = std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( rec.smiles ) ) ) {
            query->insert( *mol, rec.smiles, rec.synonym );
        }
    }
}

QSqlDatabase
document::sqlDatabase()
{
    return impl::sqlDatabase();
}

void
document::ChemSpiderSearch( const QString& sql, QTextEdit * edit )
{
    auto stmt = sql.toStdString();
    ADDEBUG() << stmt;
    ChemSpider cs( chemSpiderToken().toStdString() );

    if ( cs.AsyncSimpleSearch( stmt ) ) {

        int retry( 10 );
        std::string status;
        while( !cs.GetAsyncSearchStatus( status ) && retry-- )
            edit->append( QString::fromStdString( status ) );

        if ( retry <= 0 )
            return;

        cs.GetAsyncSearchResult();

        edit->setText( QString::fromStdString( cs.rid() + "\n" ) );

        for ( auto& csid: cs.csids() ) {

            edit->append( QString("csid=%1\n").arg( csid ) );
            std::string smiles, InChI, InChIKey;

            if ( cs.GetCompoundInfo( csid, smiles, InChI, InChIKey ) ) {

                edit->append( QString("%1").arg( QString::fromStdString( smiles ) ) );
                edit->append( QString("%1").arg( QString::fromStdString( InChI ) ) );
                edit->append( QString("%1").arg( QString::fromStdString( InChIKey ) ) );

                if ( auto mol = std::unique_ptr< RDKit::RWMol >( RDKit::SmilesToMol( smiles ) ) ) {

                    std::string svg = adchem::drawing::toSVG( *mol );
                    std::string formula = RDKit::Descriptors::calcMolFormula( *mol, true, false );
                    double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula );

                    impl::sqlDatabase().transaction();

                    do {
                        QSqlQuery sql( impl::sqlDatabase() );

                        sql.prepare( "INSERT OR IGNORE INTO mols (InChI) VALUES (?)" );
                        sql.addBindValue( QString::fromStdString( InChI ) );

                        if ( !sql.exec() ) {
                            ADDEBUG() << "SQLite error: " << sql.lastError().text().toStdString()
                                      << ", code = " << sql.lastError().nativeErrorCode().toStdString() << " while inserting " << InChI;
                        }
                    } while( 0 );

                    do {
                        QSqlQuery sql( impl::sqlDatabase() );

                        sql.prepare(
                            "UPDATE mols SET csid = ?, formula = ?, mass = ?, svg = ?, smiles = ?, InChIKey = ? "
                            " WHERE InChI = ?" );

                        sql.addBindValue( csid );
                        sql.addBindValue( QString::fromStdString( formula ) );
                        sql.addBindValue( mass );
                        sql.addBindValue( QByteArray( svg.data(), int( svg.size() ) ) );
                        sql.addBindValue( QString::fromStdString( smiles ) ); // utf8 on db
                        sql.addBindValue( QString::fromStdString( InChIKey ) );
                        sql.addBindValue( QString::fromStdString( InChI ) );

                        if ( !sql.exec() ) {
                            ADDEBUG() << "SQLite error: " << sql.lastError().text().toStdString()
                                      << ", code = " << sql.lastError().nativeErrorCode().toStdString() << " while updating " << InChI;
                        }
                    } while( 0 );

                    impl::sqlDatabase().commit();
                }

            }
        }
        emit onConnectionChanged(); // this will re-run setQuery
    }
}

void
document::setChemSpiderToken( const QString& token )
{
    settings()->setValue( "ChemSpider/SecurityToken", token );
}

QString
document::chemSpiderToken() const
{
    return impl::settings()->value( "ChemSpider/SecurityToken" ).toString();
}

void
document::findCSIDFromInChI( const QString& InChI )
{
    ChemSpider cs( chemSpiderToken().toStdString() );

    if ( cs.AsyncSimpleSearch( InChI.toStdString() ) ) {

        int retry( 10 );
        std::string status;
        while( !cs.GetAsyncSearchStatus( status ) && retry-- )
            ;

        if ( retry <= 0 )
            return;

        cs.GetAsyncSearchResult();

        for ( auto& csid: cs.csids() ) {

            QSqlQuery sql( impl::sqlDatabase() );

            sql.prepare( "UPDATE mols SET csid = ? WHERE InChI = ?" );

            sql.addBindValue( csid );
            sql.addBindValue( InChI );

            if ( !sql.exec() ) {
                ADDEBUG() << "SQLite error: " << sql.lastError().text().toStdString()
                          << ", code = " << sql.lastError().nativeErrorCode().toStdString() << " while updating " << InChI.toStdString();
            }
        }
        emit onConnectionChanged(); // this will re-run setQuery
    }
}

void
document::PubChemREST( const QByteArray& ba )
{
    auto rest= boost::json::value_to< adcontrols::PUGREST >(  adportable::json_helper::parse( ba.toStdString() ) );

    auto url = rest.pug_url().empty() ? adcontrols::PUGREST::to_url( rest, true ) : rest.pug_url();

    auto urlx = adcontrols::PUGREST::parse_url( url );
    ADDEBUG() << "url=" << urlx;
    const int version = 10; // 1.0

    const auto& [port, host, body] = urlx; // const auto& host = std::get<1>(urlx).c_str(); // "pubchem.ncbi.nlm.nih.gov";

    // const char * host = "pubchem.ncbi.nlm.nih.gov";
    // const char * port = "https";
    // auto body_s = adcontrols::PUGREST::to_url( rest, true );
    // const char * body = body_s.c_str();

    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx{ boost::asio::ssl::context::tlsv12_client };
    // verify SSL context
    {
        ctx.set_verify_mode(boost::asio::ssl::verify_peer | boost::asio::ssl::context::verify_fail_if_no_peer_cert);
        ctx.set_default_verify_paths();
        boost::certify::enable_native_https_server_verification(ctx);
    }
    auto future = std::make_shared< session >( boost::asio::make_strand(ioc),  ctx )->run( host, port, body, version );
    ioc.run();

    auto res = future.get();
    emit pugReply( QByteArray( res.body().data() ), QString::fromStdString( url ) );
}
