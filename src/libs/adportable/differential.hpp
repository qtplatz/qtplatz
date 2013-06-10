// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
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
// Smoothing and Differentiation of Data
// by Simplified Least Sequares Procedures.
// Abraham Savitzky and Marcel J. E. Golay, Anal. Chem. (1964) 36, 1627.

#pragma once

namespace adportable {

    template <class T>
    class differential {
    protected:
        typedef struct {
            long norm;
            T B[30];
        } Polynomial;
        Polynomial * table_;
        long n_;
    public:
        virtual ~differential() {};
        differential(long n, long order = 0) {  // 0 := quadratic, 1:= qubic
            ndiff(n, order);
        };
        inline T convolute(const T * py) const {
            T fxi;
            fxi = table_->B[0] * py[0];
            for (int j = 1; j <= (n_ / 2); ++j)
                fxi += table_->B[j] * (- py[-j] + py[j]);
            fxi = fxi / table_->norm;
            return fxi;
        };
        T operator()(const T * py) const {  return convolute(py);  }
        long ndiff() const               {  return n_;  };
        void ndiff(long n, long order) {
            static Polynomial _1st_derivative0[] = { /* convolutes 1st derivative quadratic */
                /*  5 */ {   10,    { 0, 1, 2, }, },
                /*  7 */ {   28,    { 0, 1, 2, 3, }, },
                /*  9 */ {   60,    { 0, 1, 2, 3, 4, }, },
                /* 11 */ {  110,    { 0, 1, 2, 3, 4, 5, }, },
                /* 13 */ {  182,    { 0, 1, 2, 3, 4, 5, 6, }, },
                /* 15 */ {  280,    { 0, 1, 2, 3, 4, 5, 6, 7, }, },
                /* 17 */ {  408,    { 0, 1, 2, 3, 4, 5, 6, 7, 8, }, },
                /* 19 */ {  570,    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, }, },
                /* 21 */ {  770,    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, }, },
                /* 23 */ { 1012,    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, }, },
                /* 25 */ { 1300,    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, }, },
            };
            static Polynomial _1st_derivative1[] = { /* convolutes 1st derivative qubic */
                { 12,        { 0, 8, -1, }, },
                { 252,       { 0, 58, 67, 22, }, },
                { 1188,      { 0, 126, 193, 142, -86 ,}, },
                { 5148,      { 0, 296, 503, 532, 294, -300, }, },
                { 24024,     { 0, 832, 1489, 1796, -1578, 660, -1133, }, },
                { 334152,    { 0, 7506, 13843, 17842, 18334, 14150, 4121, -12922, }, },
                { 23256,     { 0, 358, 673, 902, 1002, 930, 643, 98, -748, }, },
                { 255816,    { 0, 2816, 5363, 8179, 8574, 8700, 7481, 4648, -68, -6936, }, },
                { 3634092,   { 0, 29592, 56881, 79504, 95338, 101900, 96947, 78176, 43284, -10032, 84075, }, },
                { 197340,    { 0, 1222, 2365, 3350, 4098, 4530, 4567, 4130, 3140, 1518, -815, -3938, }, },
                { 1776060,   { 0, 8558, 16649, 23806, 29562, 33450, 35003, 33754, 29236, 20982, 8525, -8602, -30866, }, },
            };
            n_ = (n < 5) ? 5 : (n > 25) ? 25 : (n | 1);  // make sure n_ to be odd number
            if (order == 0)
                table_ = &_1st_derivative0[(n_ - 5) / 2];
            else
                table_ = &_1st_derivative1[(n_ - 5) / 2];
        };
    };

    template <class T>
    class differential2 {
        // Smoothing and Differentiation of Data
        // by Simplified Least Sequares Procedures.
        // Abraham Savitzky and Marcel J. E. Golay, Anal. Chem. (1964) 36, 1627.
    protected:
        typedef struct {
            long norm;
            T B[30];
        } Polynomial;
        Polynomial * table_;
        long n_;
    public:
        virtual ~differential2() {};
        differential2(long n, long order = 0) {
            ndiff(n, order);
        };
        T convolute(const T * py) const {
            py += (n_ / 2);
            T fxi;
            fxi = table_->B[0] * py[0];
            for (int j = 1; j <= (n_ / 2); ++j)
                fxi += table_->B[j] * (py[-j] + py[j]); // convolute even
            fxi = fxi / table_->norm;
            return fxi;
        };
        T operator()(const T * py) const {  return convolute(py);  }
        long ndiff() const {
            return n_;
        };
        void ndiff(long n, long order) {
            static Polynomial _2nd_derivative0[] = {
                { 7,      { -2, -1, 2, }, },
                { 42,     { -4, -3,  0, 5, }, },
                { 462,    { -20, -17, -8, 7, 28, }, },
                { 429,    { -10, -9, -6, -1, 6, 15, }, },
                { 1001,   { -14, -13, -10, -5, 2, 11, 22, }, },
                { 6188,   { -56, -53, -48, -29, -8, 19, 52, 91, }, },
                { 3876,   { -24, -23, -20, -15, -8, 1, 12, 25, 40, }, },
                { 6783,   { -30, -29, -26, -21, -14, -5, 6, 19, 34, 51, }, },
                { 33649,  { -110, -107, -98, -83, -62, -35, -2, 37, 82, 133, 190, }, },
                { 17710,  { -44, -43, -40, -35, -28, -19, -8, 5, 20, 37, 56, 77, }, },
                { 26910,  { -52, -51, -48, -43, -36, -27, -16, -3, 12, 29, 48, 69, 92, }, },
            };
            static Polynomial _2nd_derivative1[] = {
                { 3,         { -90, 48, -3, }, },
                { 99,        { -630, -171, 603, -117, }, },
                { 4719,      { -12210, -6963, 4983, 12243, -4158, }, },
                { 16731,     { -22230, -15912, 117, 17082, 20358, -10530, }, },
                { 160446,    { -124740, -99528, -32043, 53262, 115632, 98010, }, },
                { 277134,    { -137340, -116577, -59253, 19737, 95568, 133485, 88803, -93093, }, },
                { 478686,    { -160740, -141873, -88749, -11799, 71592, 137085, 153387, 82251, 121524, }, },
                { 490314,    { -116820, -105864, -74601, -27846, 26376, 76830, 109071, 105444, 45084, -96084, }, },
                { 245157,    { -42966, -39672, -30183,   -15678, 1878,  19734, 34353,   41412, 38802, 11628, -37791, }, },
                { 2812095,   { -373230, -349401, -280275, -172935, -39186, 104445, 236709, 331635, 358530, 281979, 61845, -346731, }, },
                { 4292145,   { -441870, -418011, -348429, -239109, -100026, 54855, 207579, 336201, 414786, 413409, 298155, 31119, 429594, }, },
            };
            n_ = (n < 5) ? 5 : (n > 25) ? 25 : n;
            if (order == 0)
                table_ = &_2nd_derivative0[(n_ - 5) / 2];
            else
                table_ = &_2nd_derivative1[(n_ - 5) / 2];
        };
    };

}