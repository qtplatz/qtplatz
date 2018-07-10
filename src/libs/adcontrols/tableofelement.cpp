// -*- C++ -*-
/**************************************************************************
 ** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
 ** Copyright (C) 2013-2014 MS-Cheminformatics LLC
 *
 ** Contact: info@ms-cheminfo.com
 **
 ** Commercial Usage
 **
 ** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
 ** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
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

#include "adcontrols.hpp"
#include "element.hpp"
#include "isotopes.hpp" // iterator
#include "tableofelement.hpp"
#include <adportable/array_wrapper.hpp>
#include <adportable/float.hpp>
#include <boost/noncopyable.hpp>
#include <algorithm>
#include <cassert>
#include <mutex>
#include <numeric>
#include <sstream>

using namespace adcontrols;
using namespace adcontrols::detail;

namespace adcontrols {
    namespace detail {
	
        static struct element {
            const char * symbol_;
            const char * name_;
            int atomicNumber_;
            int valence_;
            int isotopeCount_;
            adcontrols::toe::isotope /* mass, abundance */ ma_[10];
        } __elementTable__ [] = {
            // http://physics.nist.gov/cgi-bin/Compositions/stand_alone.pl
            // Symbol Name AtomicNumber Valence NumberOfIsotope Weight_1      Ratio_1...
            { "H",   "Hydrogen",       1,   1,  2,  { { 1.00782503207,        0.999885 }, 
                                                      { 2.0141017778,         0.000115 } } },
            { "He",  "Helium",         2,   0,  2,  { { 3.0160293191,         0.00000134 },  
                                                      { 4.00260325415,        0.99999866 } } },
            { "Li",  "Lithium",        3,   1,  2,  { { 6.015122795,          0.0759 },      
                                                      { 7.01600455,           0.9241 } } },
            { "Be",  "Beryllium",      4,   2,  1,  { { 9.0121822,            1 } } },
            { "B",   "Boron",          5,   3,  2,  { { 10.0129370,           0.1997, },       
                                                      { 11.0093054,           0.8017 } } },
            { "C",   "Carbon",         6,   4,  2,  { { 12,                   0.9893 },      
                                                      { 13.0033548378,        0.0107 } } },
            { "N",   "Nitrogen",       7,   3,  2,  { { 14.0030740048,        0.99632 },     
                                                      { 15.0001088982,        0.00368 } } },
            { "O",   "Oxygen",         8,   2,  3,  { { 15.99491461956,         0.99757 },     
                                                      { 16.99913170,            0.00038 },     
                                                      { 17.9991610,             0.00205 } } },
            { "F",   "Fluorine",       9,   1,  1,  { { 18.99840322,            1 } } },
            { "Ne",  "Neon",           10,  0,  3,  { { 19.9924401754,          0.9048 },      
                                                      { 20.99384668,            0.0027 },      
                                                      { 21.991385114,           0.0925 } } },
            { "Na",  "Sodium",         11,  1,  1,  { { 22.9897692809,          1 } } },
            { "Mg",  "Magnesium",      12,  2,  3,  { { 23.985041700,           0.7899 },      
                                                      { 24.98583692,            0.1 },         
                                                      { 25.982592929,           0.1101 } } },
            { "Al",  "Aluminium",      13,  3,  1,  { { 26.98153863,            1 } } },
            { "Si",  "Silicon",        14,  4,  3,  { { 27.9769265325,          0.922297 },    
                                                      { 28.976494700,           0.046832 },
                                                      { 29.97377017,            0.030872 } } },
            { "P",   "Phosphorus",     15,  5,  1,  { { 30.97376163,            1 } } },
            { "S",   "Sulfur",         16,  4,  4,  { { 31.97207100,            0.9493 },      
                                                      { 32.97145876,            0.0076 },      
                                                      { 33.96786690,            0.0429 },      
                                                      { 35.96708076,            0.0002 } } },
            { "Cl",  "Chlorine",       17,  1,  2,  { { 34.96885268,            0.7578 },      
                                                      { 36.96590259,            0.2422 } } },
            { "Ar",  "Argon",          18,  0,  3,  { { 35.967545106,           0.003365 },    
                                                      { 37.9627324,             0.000632 },    
                                                        { 39.9623831225,        0.996003 } } },
            { "K",   "Potassium",      19,  1,  3,  { { 38.96370668,            0.932581 },    
                                                        { 39.96399848,          0.000117 },    
                                                        { 40.96182576,          0.067302 } } },
            { "Ca",  "Calcium",        20,  2,  6,  { { 39.96259098,            0.96941 },     
                                                        { 41.95861801,          0.00647 },     
                                                        { 42.9587666,           0.00135 },     
                                                        { 43.9554818,           0.02086 },     
                                                        { 45.9536926,           0.00004 },     
                                                        { 47.952534,            0.00187 } } },
            { "Sc",  "Scandium",       21,  3,  1,  { { 44.9559119,             1 } } },
            { "Ti",  "Titanium",       22,  2,  5,  { { 45.9526316,             0.0825 },      
                                                        { 46.9517631,           0.0744 },      
                                                        { 47.9479463,           0.7372 },      
                                                        { 48.9478700,           0.0541 },      
                                                        { 49.9447912,           0.0518 } } },
            { "V",   "Vanadium",       23,  3,  2,  {   { 49.9471585,             0.0025 },      
                                                        { 50.9439595,           0.9975 } } },
            { "Cr",  "Chromium",       24,  3,  4,  {   { 49.9460442,             0.04345 },     
                                                        { 51.9405075,           0.83789 },     
                                                        { 52.9406494,           0.09501 },     
                                                        { 53.9388804,           0.02365 } } },
            { "Mn",  "Manganese",      25,  4,  1,  {   { 54.9380451,           1 } } },
            { "Fe",  "Iron",           26,  3,  4,  {   { 53.9396105,           0.05845 },     
                                                        { 55.9349375,           0.91754 },     
                                                        { 56.9353940,           0.02119 },     
                                                        { 57.9332756,           0.00282 } } },
            { "Co",  "Cobalt",         27,  3,  1,  {   { 58.9331950,           1 } } },
            { "Ni",  "Nickel",         28,  3,  5,  {   { 57.9353429,           0.680769 },    
                                                        { 59.9307864,           0.262231 },    
                                                        { 60.9310560,           0.011399 },    
                                                        { 61.9283451,           0.036345 },    
                                                        { 63.9279660,           0.009256 } } },
            { "Cu",  "Copper",         29,  2,  2,  {   { 62.9295975,           0.6917 },      
                                                        { 64.9277895,           0.3083 } } },
            { "Zn",  "Zinc",           30,  2,  5,  {   { 63.9291422,           0.4863 },      
                                                        { 65.9260334,           0.279 },       
                                                        { 66.9271273,           0.041 },       
                                                        { 67.9248442,           0.1875 },      
                                                        { 69.9253193,           0.0062 } } },
            { "Ga",  "Gallium",        31,  3,  2,  {   { 68.9255736,           0.60108 },     
                                                        { 70.9247013,           0.39892 } } },
            { "Ge",  "Germanium",      32,  4,  5,  {   { 69.9242474,           0.2084 },      
                                                        { 71.9220758,           0.2754 },      
                                                        { 72.9234589,           0.0773 },      
                                                        { 73.9211778,           0.3628 },      
                                                        { 75.9214026,           0.0761 } } },
            { "As",  "Arsenic",        33,  3,  1,  {   { 74.9215965,           1 } } },
            { "Se",  "Selenium",       34,  4,  6,  {   { 73.9224764,           0.0089 },      
                                                        { 75.9192136,           0.0937 },      
                                                        { 76.9199140,           0.0763 },      
                                                        { 77.9173091,           0.2377 },      
                                                        { 79.9165213,           0.4961 },      
                                                        { 81.9166994,           0.0873 } } },
            { "Br",  "Bromine",        35,  1,  2,  {   { 78.9183371,           0.5069 },      
                                                        { 80.9162906,           0.4931 } } },
            { "Kr",  "Krypton",        36,  0,  6,  {   { 77.9203648,           0.0035 },      
                                                        { 79.9163790,           0.0228 },      
                                                        { 81.9134836,           0.1158 },      
                                                        { 82.914136,            0.1149 },      
                                                        { 83.911507,            0.57 },        
                                                        { 85.91061073,          0.173 } } },
            { "Rb",  "Rubidium",       37,  1,  2,  {   { 84.911789738,         0.7217 },      
                                                        { 86.909180527,         0.2783 } } },
            { "Sr",  "Strontium",      38,  2,  4,  {   { 83.913425,            0.0056 },      
                                                        { 85.9092602,           0.0986 },      
                                                        { 86.9088771,           0.07 },        
                                                        { 87.9056121,           0.8258 } } },
            { "Y",   "Yttrium",        39,  3,  1,  {   { 88.9058483,           1 } } },
            { "Zr",  "Zirconium",      40,  4,  5,  {   { 89.9047044,           0.5145 },      
                                                        { 90.9056458,           0.1122 },      
                                                        { 91.9050408,           0.1715 },      
                                                        { 93.9063152,           0.1738 },      
                                                        { 95.9082734,           0.028 } } },
            { "Nb",  "Niobium",        41,  3,  1,  {   { 92.9063781,           1 } } },
            { "Mo",  "Molybdenum",     42,  3,  7,  {   { 91.906811,            0.1484 } ,      
                                                        { 93.9050883,           0.0925 },      
                                                        { 94.9058421,           0.1592 },      
                                                        { 95.9046795,           0.1668 },      
                                                        { 96.9060215,           0.0955 },      
                                                        { 97.9054082,           0.2413 },      
                                                        { 99.907477,            0.0963 } } },
            { "Tc", "Technetium",      43,  2,   1, {   { 96.906365,            1.0000 },
                                                        { 97.907216,            0.0000 },
                                                        { 98.9062547,           0.0000 } } },
            { "Ru",  "Ruthenium",      44,  3,  7,  {   { 95.907598,            0.0554 },      
                                                        { 97.905287,            0.0187 },      
                                                        { 98.9059393,           0.1276 },      
                                                        { 99.9042195,           0.126 },       
                                                        { 100.9055821,          0.1706 },      
                                                        { 101.9043493,          0.3155 },      
                                                        { 103.905433,           0.1862 } } },
            { "Rh",  "Rhodium",        45,  3,  1,  {   { 102.905504,           1 } } },
            { "Pd",  "Palladium",      46,  2,  6,  {   { 101.905609,           0.0102 },      
                                                        { 103.904036,           0.1114 },      
                                                        { 104.905085,           0.2233 },      
                                                        { 105.903486,           0.2733 },      
                                                        { 107.9038928,          0.2646 },      
                                                        { 109.905153,           0.1172 } } },
            { "Ag",  "Silver",         47,  1,  2,  {   { 106.905097,           0.51839 },     
                                                        { 108.904752,           0.48161 } } },
            { "Cd",  "Cadmium",        48,  2,  8,  {   { 105.906459,           0.0125 },      
                                                        { 107.904184,           0.0089 },      
                                                        { 109.9030021,          0.1249 },      
                                                        { 110.9041781,          0.128 },       
                                                        { 111.9027578,          0.2413 },      
                                                        { 112.9044017,          0.1222 },      
                                                        { 113.9033585,          0.2873 },      
                                                        { 115.904756,           0.0749 } } },
            { "In",  "Indium",         49,  3,  2,  {   { 112.904058,           0.0429 },      
                                                        { 114.903878,           0.9571 } } },
            { "Sn",  "Tin",            50,  2,  10, {   { 111.904818,           0.0097 },      
                                                        { 111.904818,           0.0066 },      
                                                        { 114.903342,           0.0034 },      
                                                        { 115.901741,           0.1454 },      
                                                        { 116.902952,           0.0768 },      
                                                        { 117.901603,           0.2422 },      
                                                        { 118.903308,           0.0859 },      
                                                        { 119.9021947,          0.3258 },      
                                                        { 121.9034390,          0.0463 },      
                                                        { 123.9052739,          0.0579 } } },
            { "Sb",  "Antimony",       51,  3,  2,  {   { 120.9038157,          0.5721 },      
                                                        { 122.9042140,          0.4279 } } },
            { "Te",  "Tellurium",      52,  4,  8,  {   { 119.904020,           0.0009 },      
                                                        { 121.9030439,          0.0255 },      
                                                        { 122.9042700,          0.0089 },      
                                                        { 123.9028179,          0.0474 },      
                                                        { 124.9044307,          0.0707 },      
                                                        { 125.9033117,          0.1884 },      
                                                        { 127.9044631,          0.3174 },      
                                                        { 129.9062244,          0.3408 } } },
            { "I",   "Iodine",         53,  1,  1,  {   { 126.904473,           1 } } },
            { "Xe",  "Xenon",          54,  0,  9,  {   { 123.9058930,          0.0009 },      
                                                        { 125.904274,           0.0009 },      
                                                        { 127.9035313,          0.0192 },      
                                                        { 128.9047794,          0.2644 },      
                                                        { 129.9035080,          0.0408 },      
                                                        { 130.9050824,          0.2118 },      
                                                        { 131.9041535,          0.2689 },      
                                                        { 133.9053945,          0.1044 },      
                                                        { 135.907219,           0.0887 } } },
            { "Cs",  "Caesium",        55,  0,  1,  {   { 132.905451933,        1 } } },
            { "Ba",  "Barium",         56,  2,  7,  {   { 129.9063208,          0.00106 },     
                                                        { 131.9050613,          0.00101 },     
                                                        { 133.9045084,          0.02417 },     
                                                        { 134.9056886,          0.06592 },     
                                                        { 135.9045759,          0.07854 },     
                                                        { 136.9058274,          0.11232 },     
                                                        { 137.9052472,          0.71698 } } },
            { "La",  "Lanthanum",      57,  3,  2,  {   { 137.907112,           0.0009 },      
                                                        { 138.9063533,          0.9991 } } },
            { "Ce",  "Cerium",         58,  3,  4,  {   { 135.907172,           0.00185 },     
                                                        { 137.905991,           0.00251 },     
                                                        { 139.9054387,          0.8845 },      
                                                        { 141.909244,           0.11114 } } },
            { "Pr",  "Praseodymium",   59,  3,  1,  {   { 140.9076528,          1 } } },
            { "Nd",  "Neodymium",      60,  3,  7,  {   { 141.9077233,          0.272 },
                                                        { 142.9098143,          0.122 },
                                                        { 143.9100873,          0.238 },
                                                        { 144.9125736,          0.083 },
                                                        { 145.9131169,          0.172 },
                                                        { 147.916893,           0.057 },
                                                        { 149.920891,           0.056 } } },
            { "Pm",  "Promethium",     61,  3,   1, {   { 144.912749,           1.0000 },
                                                        { 146.9151385,          0  } } },
            { "Sm",  "Samarium",       62,  3,  7,  {   { 143.911999,           0.0307 },
                                                        { 146.9148979,          0.1499 },
                                                        { 147.9148227,          0.1124 },
                                                        { 148.9171847,          0.1382 },
                                                        { 149.9172755,          0.0738 },
                                                        { 151.9197324,          0.2675 },
                                                        { 153.9222093,          0.2275 } } },
            { "Eu",  "Europium",       63,  3,  2,  {   { 150.9198502,          0.4781 },
                                                        { 152.9212303,          0.5219 } } },
            { "Gd",  "Gadolinium",     64,  3,  7,  {   { 151.9197910,          0.002 },
                                                        { 153.9208656,          0.0218 },
                                                        { 154.9226220,          0.148 },
                                                        { 155.9221227,          0.2047 },
                                                        { 156.9239601,          0.1565 },
                                                        { 157.9241039,          0.2484 },
                                                        { 159.9270541,          0.2186 } } },
            { "Tb",  "Terbium",        65,  3,  1,  {   { 158.9253468,          1 } } },
            { "Dy",  "Dysprosium",     66,  3,  7,  {   { 155.924283,           0.0006 },
                                                        { 157.924409,           0.001 },
                                                        { 159.9251975,          0.0234 },
                                                        { 160.9269334,          0.1891 },
                                                        { 161.9267984,          0.2551 },
                                                        { 162.9287312,          0.249 },
                                                        { 163.9291748,          0.2818 } } },
            { "Ho",  "Holmium",        67,  3,  1,  {   { 164.9303221,          1 } } },
            { "Er",  "Erbium",         68,  3,  6,  {   { 161.928778,           0.0014 },
                                                        { 163.929200,           0.0161 },
                                                        { 165.9302931,          0.3361 },
                                                        { 166.9320482,          0.2293 },
                                                        { 167.9323702,          0.2678 },
                                                        { 169.9354643,          0.1493 } } },
            { "Tm",  "Thulium",        69,  3,  1,  {   { 168.9342133,          1 } } },
            { "Yb",  "Ytterbium",      70,  3,  7,  {   { 167.933897,           0.0013 },
                                                        { 169.9347618,          0.0304 },
                                                        { 170.9363258,          0.1428 },
                                                        { 171.9363815,          0.2183 },
                                                        { 172.9382108,          0.1613 },
                                                        { 173.9388621,          0.3183 },
                                                        { 175.9425717,          0.1276 } } },
            { "Lu",  "Lutetium",       71,  3,  2,  {   { 174.9407718,          0.9741 },
                                                        { 175.9426863,          0.0259 } } },
            { "Hf",  "Hafnium",        72,  3,  6,  {   { 173.940046,           0.0016 },
                                                        { 175.9414086,          0.0526 },
                                                        { 176.9432207,          0.186 },
                                                        { 177.9436988,          0.2728 },
                                                        { 178.9458161,          0.1362 },
                                                        { 179.9465500,          0.3508 } } },
            { "Ta",  "Tantalum",       73,  5,  2,  {   { 179.9474648,          0.00012 },
                                                        { 180.9479958,          0.99988 } } },
            { "W",   "Tungsten",       74,  4,  5,  {   { 179.946704,           0.0012 },
                                                        { 181.9482042,          0.265 },
                                                        { 182.9502230,          0.1431 },
                                                        { 183.9509312,          0.3064 },
                                                        { 185.9543641,          0.2843 } } },
            { "Re",  "Rhenium",        75,  3,  2,  {   { 184.9529550,          0.374 },
                                                        { 186.9557531,          0.626 } } },
            { "Os",  "Osmium",         76,  3,  7,  {   { 183.9524891,          0.0002 },
                                                        { 185.9538382,          0.0159 },
                                                        { 186.9557505,          0.0196 },
                                                        { 187.9558382,          0.1324 },
                                                        { 188.9581475,          0.1615 },
                                                        { 189.9584470,          0.2626 },
                                                        { 191.9614807,          0.4078 } } },
            { "Ir",  "Iridium",        77,  3,  2,  {   { 190.9605940,          0.373 },
                                                        { 192.9629264,          0.627 } } },
            { "Pt",  "Platinum",       78,  2,  6,  {   { 189.959932,           0.00014 },
                                                        { 191.9610380,          0.00782 },
                                                        { 193.9626803,          0.32967 },
                                                        { 194.9647911,          0.33832 },
                                                        { 195.9649515,          0.25242 },
                                                        { 197.967893,           0.07163 } } },
            { "Au",  "Gold",           79,  1,  1,  {   { 196.9665687,          1 } } },
            { "Hg",  "Mercury",        80,  1,  7,  {   { 195.965833,   0.0015 },
                                                        { 197.9667690,  0.0997 },
                                                        { 198.9682799,  0.1687 },
                                                        { 199.9683260,  0.231 },
                                                        { 200.9703023,  0.1318 },
                                                        { 201.9706430,  0.2986 },
                                                        { 203.9734939,  0.0687 } } },
            { "Tl",  "Thallium",       81,  3,  2,  {   { 202.9723442,  0.29524 },
                                                        { 204.9744275,  0.70476 } } },
            { "Pb",  "Lead",           82,  2,  4,  {   { 203.9730436,  0.014 },
                                                        { 205.9744653,  0.241 },
                                                        { 206.9758969,  0.221 },
                                                        { 207.9766521,  0.524 } } },
            { "Bi",  "Bismuth",        83,  3,  1,  { { 208.9803987,    1 } } },
            { "Po", "Polonium",        84,  0,   1, { { 208.9824304,    1.0000 } } }, // 209.982 8737(13)	
            { "At", "Astatine",        85,  0,   1, { { 209.987148,     1.0000 } } }, // 210.987 4963(30)	
            { "Rn", "Radon",           86,  0,   1, { { 210.990601,     1.0000 } } }, // 220.011 3940(24), 222.017 5777(25)	
            { "Fr", "Francium",        87,  0,   1, { { 223.0197359,    1.0000 } } },
            { "Ra", "Radon",           88,  2,   1, { { 223.0185022,    1.0000 } } }, // 224.020 2118(24), 226.025 4098(25), 228.031 0703(26)			
            { "Ac", "Actinium",        89,  0,   1, { { 227.0277521,    1.0000 } } },
            { "Th",  "Thorium",        90,  4,  1,  { { 232.0380553,    1 } } }, // 230.033 1338(19)	
            { "Pa",  "Protactinium",   91,  0,  1,  { { 231.0358840,    1 } } },
            { "U",   "Uranium",        92,  4,  3,  { { 233.0396352,    0 },
                                                        { 234.0409521,  0.000055 },
                                                        { 235.0439299,  0.0072 },
                                                        { 236.0455680,  0 },
                                                        { 231.0358840,  0.992745 } } },
            { "Np", "Neptunium",       93,  3,   1, { { 236.046570,     1.0000 } } },
            { "Pu", "Plutonium",       94,  0,   1, { { 238.0495599,    1.0000 } } },
            { "Am", "Americium",       95,  0,   1, { { 241.0568291,    1.0000 } } },
            { "Cm", "Curium",          96,  0,   1, { { 243.0613891,    1.0000 } } },
            { "Bk", "Berkelium",       97,  0,   1, { { 247.070307,     1.0000 } } },
            { "Cf", "Californium",     98,  0,   1, { { 249.0748535,    1.0000 } } },
            { "Es", "Einsteinium",     99,  0,   1, { { 252.082980,     1.0000 } } },
            { "Fm", "Fermium",        100,  0,   1, { { 257.095105,     1.0000 } } },
            { "Md", "Mendelevium",    101,  0,   1, { { 258.098431,     1.0000 } } },
            { "No", "Nobelium",       102,  0,   1, { { 259.10103,      1.0000 } } },
            { "Lr", "Lawrencium",     103,  0,   1, { { 262.10963,      1.0000 } } },
        };
	
        struct superatom {
            const char * name_;
            const char * alias_;
            const char * formula_;
            int valence_;
        } __superAtoms__[] = {
            // ;AminoAcid"Alias", Formula, Valence },
            { "A", "Ala",           "C3H5NO",               2 },
            { "R", "Arg",           "C6H12N4O",             2 },
            { "N", "Asn",           "C4H6N2O2",             2 },
            { "D", "Asp",           "C4H5NO3",              2 },
            { "C", "Cys",           "C3H5NOS",              2 },
            { "Q", "Gln",           "C5H8N2O2",             2 },
            { "E", "Glu",           "C5H7NO3",              2 },
            { "G", "Gly",           "C2H3NO",               2 },
            { "H", "His",           "C6H7N3O",              2 },
            { "I", "Ile",           "C6H11NO",              2 },
            { "L", "Leu",           "C6H11NO",              2 },
            { "K", "Lys",           "C6H12N2O",             2 },
            { "M", "Met",           "C5H9NOS",              2 },
            { "F", "Phe",           "C9H9NO",               2 },
            { "P", "Pro",           "C5H7NO",               2 },
            { "S", "Ser",           "C3H5NO2",              2 },
            { "T", "Thr",           "C4H7NO2",              2 },
            { "W", "Trp",           "C11H10N2O",            2 },
            { "Y", "Tyr",           "C9H9NO2",              2 },
            { "V", "Val",           "C5H9NO",               2 },
        };
	
        /*
          [DNA] },
          ; },
          ;Symbol"Alias", Formula, Valence },
          ; },
          { "A", "Adenine",       C10H12N5O5P,          2 },
          { "C", "Cytosine",      C9H12N3O6P,           2 },
          { "G", "Guanine",       C10H12N5O6P,          2 },
          { "T", "Thymine",       C10H13N2O7P,          2 },
          },
          [RNA] },
          ; },
          ;Symbol"Alias", Formula, Valence },
          ; },
          { "A", "Adenine",       C10H12N5O6P,          2 },
          { "C", "Cytosine",      C9H12N3O7P,           2 },
          { "G", "Guanine",       C10H12N5O7P,          2 },
          { "U", "Uracil",        C9H11N2O8P,           2 },
        */

    } // namespace detail
} // namespace adcontrols

///////////////////////////////
////////////////////////

std::atomic<TableOfElement * > TableOfElement::instance_( 0 );
std::mutex TableOfElement::mutex_;

TableOfElement::~TableOfElement()
{
}

TableOfElement::TableOfElement()
{
    for ( const auto& e: detail::__elementTable__ )
        index_[ e.symbol_ ] = e.atomicNumber_;
}

TableOfElement *
TableOfElement::instance()
{
    typedef TableOfElement T;

    T * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new T();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}


namespace adcontrols {

	namespace mol {

        element::element( const detail::element * p ) : p_( p ), count_( 1 )
        {
        }

        element::element( const element& t ) : p_( t.p_ ), count_( t.count_ )
        {
        }

        element::operator bool () const
        {
            return p_ != 0;
        }

        const char *
        element::symbol() const
        {
            if ( p_ )
                return p_->symbol_;
            return 0;
        }

        const char *
        element::name() const
        {
            if ( p_ )
                return p_->name_;
            return 0;
        }

        int
        element::atomicNumber() const
        {
            if ( p_ )
                return p_->atomicNumber_;
            return 0;
        }

        int
        element::valence() const
        {
            if ( p_ )
                return p_->valence_;
            return -1;
        }

        toe::isotopes
        element::isotopes() const
        {
            if ( p_ )
                return toe::isotopes( p_->ma_, size_t( p_->isotopeCount_ )  );
            return toe::isotopes( 0, 0 );
        }

        int
        element::count() const
        {
            return count_;
        }

        void
        element::count( int v )
        {
            count_ = v;
        }

        // static
        double
        element::monoIsotopicMass( const element& e, int isotope )
        {
            if ( isotope == 0 ) { 

                // return most abundunt
                auto it = std::max_element( e.isotopes().begin(), e.isotopes().end(), []( const toe::isotope& a, const toe::isotope& b ){ return a.abundance < b.abundance; });
                if ( it != e.isotopes().end() )
                    return it->mass;
            } else {

                // find specified isotope
                auto it = std::find_if( e.isotopes().begin(), e.isotopes().end(), [=]( const toe::isotope& a ){ return int ( a.mass + 0.3 ) == isotope; });
                if ( it != e.isotopes().end() )
                    return it->mass;
            }
            return 0;
        }

        double
        element::chemicalMass( const element& e )
        {
            std::pair<double, double> sum
                = std::accumulate( e.isotopes().begin(), e.isotopes().end()
                                   , std::make_pair( 0.0, 0.0 )
                                   , []( const std::pair<double, double>& a, const toe::isotope& i ){
                                       return std::make_pair( a.first + (i.mass * i.abundance), a.second + i.abundance ); });
            
            //assert( adportable::compare<double>::approximatelyEqual( sum.second, 1.0 ) );

            return sum.first;
        }
	}
}

mol::element
TableOfElement::findElement( const std::string& symbol ) const
{
    auto it = index_.find( symbol );
    if ( it != index_.end() ) {
        assert( symbol == __elementTable__[ it->second - 1 ].symbol_ );
        return mol::element( &__elementTable__[ it->second - 1 ] );
    }
    return mol::element(0);
}

double
TableOfElement::electronMass() const
{
    return 0.00054857990943;
}

