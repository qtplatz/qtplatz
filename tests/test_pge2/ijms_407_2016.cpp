// Copyright 2025, MS-Cheminformatics LLC
// Licensed under the MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <iostream>
#include <format>
#include <libs/adprocessor/peakd.hpp>

                                                                                                                                                                             //extern std::vector< std::tuple< double, double, double, double, double > > __data;


//  1 2.2 164 C9H7O3 163.0403 163.0401 1.2     163 (1); 119 (100); 93 (2)
//  2 2.4 164 C9H7O3 163.0409 163.0401 4.9     163 (1); 119 (100); 93 (2)

// 3a 2.5 a 194 C10H9O4 193.0509 193.0506 1.6  193 (3); 178 (37); 149 (6); 134 (100)
// 4b 2.8 b 194 C10H9O4 193.0513 193.0506 3.6  193 (1); 178 (16); 149 (3); 134 (100)

// 5a 2.5 a 388 C20H19O8 387.1073 387.1085 4.9 387 (10); 369 ( 2); 343 (20);   [325 (2)];  310 (2); 299 (3);  295 ( 1); 284 ( 9); 269 (2 ); 193 (100); 178 (10); 149 (16); 134 (37)
// 6b 2.8 b 388 C20H19O8 387.1093 387.1085 2.0 387 ( 3); 343 ( 3); 341 (11);   [326 (8)];  310 (1); 284 ( 1); 282 (12); 267 (3);  193 (100); 178 (39); 149 (21); 134 (90)
//  7 3.5 388 C20H19O8 387.1085 387.1085 0.0   387 (22); 369 (12); 343 (100);  [328 (72)]; 325 (3); 299 (27); 295 (22); 284 (39); 269 (13); 219 (50); 175 (5); 160 (24)

// 8b 2.8 b 386 C20H17O8 385.0928 385.0929 0.3 385 (0); 341 (67); 326 (42); 297 (12); 282 (63); 267 (11); 193 (90); 178 (40); 149 (23); 134 (100)
//  9 3.1 386 C20H17O8 385.0931 385.0929 0.5   385 (1); 341 (36); 326 (59); 297 (58); 282 (100); 267 (46); 193 (3); 159 (38); 149 (1); 134 (2)
// 10 3.8 386 C20H17O8 385.0922 385.0929 1.8   385 (1); 341 (38); 326 (61); 309 (55); 297 (28); 294 (38) 282 (100); 281 (79); 267 (46); 265 (7); 193 (10); 173 (32); 159 (99); 145 (13); 134 (3); 123 (45)
// 11 4.1 386 C20H17O8 385.0925 385.0929 0.9   385 (100); 370 (25); 341 (62); 326 (64); 282 (67); 267 (14)
// 12 4.7 386 C20H17O8 385.0930 385.0929 0.3   385 (16); 370 (5); 341 (16); 326 (20); 282 (25); 193 (100); 178 (30); 149 (11); 134 (15)
// 13 4.9 386 C20H17O8 385.0919 385.0929 2.6   385 (1); 341 (55); 326 (100); 311 (18); 297 (28); 282 (65); 267 (52); 193 (14)
// 14 5.5 386 C20H17O8 385.0910 385.0929 0.9   385 (2); 341 (23); 326 (100); 311 (97); 283 (7); 267 (17); 239 (6); 193 (88); 178 (22); 163 (34); 148 (18); 134 (10); 121(8)

// 15 6.6 342 C19H17O6 341.1032 341.1031 0.3   341 (27); 326 (90); 311 (29); 297 (13); 282 (69); 267 (100); 249 (4)

int
main()
{
    constexpr int ndim = 25;
    constexpr int nprod = 7;

    Eigen::Matrix<double, ndim, nprod> A;

    A << //     No. 8b;   9;    10;   11;   12;   13;   14
        /* 385 */   0 ,   1 ,   1 , 100 ,  16 ,   1 ,   2 ,
        /* 370 */   0 ,   0 ,   0 ,  25 ,   5 ,   0 ,   0 ,
        /* 341 */  67 ,  36 ,  38 ,  62 ,  16 ,  55 ,  23 ,
        /* 326 */  42 ,  59 ,  61 ,  64 ,  20 , 100 , 100 ,
        /* 311 */   0 ,   0 ,   0 ,   0 ,   0 ,  18 ,  97 ,
        /* 309 */   0 ,   0 ,  55 ,   0 ,   0 ,   0 ,   0 ,
        /* 297 */  12 ,  58 ,  28 ,   0 ,   0 ,  28 ,   0 ,
        /* 294 */   0 ,   0 ,  38 ,   0 ,   0 ,   0 ,   0 ,
        /* 283 */   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   7 ,
        /* 282 */  63 , 100 , 100 ,  67 ,  25 ,  65 ,   0 ,
        /* 281 */   0 ,   0 ,  79 ,   0 ,   0 ,   0 ,   0 ,
        /* 267 */  11 ,  46 ,  46 ,  14 ,   0 ,  52 ,  17 ,
        /* 265 */   0 ,   0 ,   7 ,   0 ,   0 ,   0 ,   0 ,
        /* 239 */   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   6 ,
        /* 193 */  90 ,   3 ,  10 ,   0 , 100 ,  14 ,  88 ,
        /* 178 */  40 ,   0 ,   0 ,   0 ,  30 ,   0 ,  22 ,
        /* 173 */   0 ,   0 ,  32 ,   0 ,   0 ,   0 ,   0 ,
        /* 163 */   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,  34 ,
        /* 159 */   0 ,  38 ,  99 ,   0 ,   0 ,   0 ,   0 ,
        /* 149 */  23 ,   1 ,   0 ,   0 ,  11 ,   0 ,   0 ,
        /* 148 */   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,  18 ,
        /* 145 */   0 ,   0 ,  13 ,   0 ,   0 ,   0 ,   0 ,
        /* 134 */ 100 ,   2 ,   3 ,   0 ,  15 ,   0 ,  10 ,
        /* 123 */   0 ,   0 ,  45 ,   0 ,   0 ,   0 ,   0 ,
        /* 121 */   0 ,   0 ,   0 ,   0 ,   0 ,   0 ,   8
        ;



    double rms = 1.0; //
    auto covMatrix = rms*rms * (A.transpose() * A).inverse();

    auto propagatedErrors = covMatrix.diagonal().array().sqrt();
    std::cout << "proparaged rms errors: \n" << propagatedErrors << std::endl;

    auto peakd = adprocessor::PeakDecomposition< double, ndim, nprod >( A );


}
