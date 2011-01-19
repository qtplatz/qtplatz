// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include <math.h>
#include <vector>
#include <assert.h>

namespace adportable {

    struct timeFunctor {
        timeFunctor( size_t startDelay, double sampInterval ) : delay(startDelay), interval(sampInterval) {}
        double operator ()( int pos ) {
            return ( delay + pos ) * interval;
        }
        double interval;
        size_t delay;
    };

    class intersection {
    public:
        template<class T> static size_t left_bound( const T* py, const T threshold, size_t spos, size_t tpos, size_t epos ) {
            size_t x = tpos - 1;
            while ((py[x] >= threshold) && x > spos)
                --x;
            return x + 1;
        };

        template<class T> static size_t right_bound( const T* py, const T threshold, size_t spos, size_t tpos, size_t epos ) { 
            long x = tpos_ + 1;
            while ((py[x] >= threshold) && x < epos)
                ++x;
            return x - 1;
        };
    };

    template<class Fx> class Moment {
    public:
        Moment( Fx& f, const double * py ) : py_(py), fx_(f) {}
        double centerX( double threshold, size_t spos, size_t tpos, size_t epos ) {
            return 0;
        }

        double left_intersection( size_t x, double threshold ) const {
            if (py_[x - 1] >= threshold)
                return fx_( x - 1 );
            if (py_[x] < threshold)  // should not be here, it's a bug
                return fx_( x );
            return fx_( x - 1 ) + (fx_( x ) - fx_( x - 1 ) ) * (threshold_ - py_[x - 1]) / (py_[ x ] - py_[ x - 1 ]);
        };

        double right_intersection( size_t x, double threshold ) const {
            if (py_[x + 1] >= threshold_)
                return fx_( x + 1 );
            if (py_[x] < threshold_)  // should not be here, it's a bug
                return fx_( x );
            return fx_[x] + 
                (double)(fx_(x + 1) - fx_(x)) * (py_[x] - threshold) / (double)(py_[x] - py_[x + 1]);
        };

    private:
        const double * py_;
        Fx& fx_;
    };
#if 0
    template<class Tx, class Ty>
    class Moment {
    private:
        const Tx * px_;
        const Ty * py_;
        const long spos_;
        const long tpos_;
        const long epos_;
        double threshold_;
        double Xc_;
        double Xl_, Xr_;
        bool isdirty_;
    protected:
        // always finding bound from low level through top
        inline long left_bound_from_base() const {
            long x = spos_ + 1;
            while ((py_[x] < threshold_) && x <= tpos_)
                ++x;
            return x;
        };
        inline long right_bound_from_base() const {
            long x = epos_ - 1;
            while ((py_[x] < threshold_) && x >= tpos_)
                --x;
            return x;
        };
        inline long left_bound() const {
            long x = tpos_ - 1;
            while ((py_[x] >= threshold_) && x > spos_)
                --x;
            return x + 1;
        };
        inline long right_bound() const {
            long x = tpos_ + 1;
            while ((py_[x] >= threshold_) && x < epos_)
                ++x;
            return x - 1;
        };

        inline double left_intersection(long x) const {
            if (py_[x - 1] >= threshold_)
                return px_[x - 1];
            if (py_[x] < threshold_)  // should not be here, it's a bug
                return px_[x];
            return px_[x - 1] + 
                (double)(px_[x] - px_[x - 1]) *
                (threshold_ - py_[x - 1]) / (double)(py_[x] - py_[x - 1]);
        };
        inline double right_intersection(long x) {
            if (py_[x + 1] >= threshold_)
                return px_[x + 1];
            if (py_[x] < threshold_)  // should not be here, it's a bug
                return px_[x];
            return px_[x] + 
                (double)(px_[x + 1] - px_[x]) *
                (py_[x] - threshold_) / (double)(py_[x] - py_[x + 1]);
        };
        void compute() {
            long xL = left_bound();
            long xR = right_bound();
            Xl_ = left_intersection(xL);
            Xr_ = right_intersection(xR);

            double x0 = px_[xL - 1];

            double ma = 0;
            double ta = 0;
            double baseH = threshold_;

            {
                // left triangle
                //
                //  90deg rotated triangle (h == time axis)
                //     /|
                //    / | h   A  = (1/2) a * h;
                //   /__|     Gy = (1/3) * h
                //    a
                double h = px_[xL] - Xl_;
                double a = py_[xL] - baseH;
                double area = h * a / 2;
                double Gy = h / 3;
                double cx = px_[xL] - Gy - x0;
                ta += area;
                ma += area * cx;
            }
            {
                // right triangle
                double h = Xr_ - px_[xR];
                double a = py_[xR] - baseH;
                double area = h * a / 2;
                double Gy = h / 3;
                double cx = px_[xR] + Gy - x0;
                ta += area;
                ma += area * cx;
            }
            //ta = aL + aR;
            //ma = mL + mR;

            for (long x = xL; x < xR; ++x) {
                // 90 deg rotated trapezium
                //         c
                //      ________
                //     /       |           A  = (1/2) * (a + c) * h;
                //  d /   Gy   | b = h           h(a + 2c)
                //   /_________|           Gy = ------------
                //        a                      3(a + c)
                //
                double h = px_[x + 1] - px_[x];
                double a = py_[x + 1] - baseH;
                double c = py_[x] - baseH;
                //double d = h;
                double Gy = (h * (a + 2 * c)) / (3 * (a + c));
                double area = (c + a) * h / 2;
                ta += area;
                double cx = (px_[x + 1] - Gy) - x0;
                ma += area * cx;
            };
            Xc_ = ma / ta + x0;
        };

    public:
        virtual ~Moment() { };
        Moment() : px_(0), py_(0), spos_(0), tpos_(0), epos_(0) {
            threshold_ = 0;
            Xc_ = 0;
            isdirty_ = true;
        };
        Moment(const Tx * px, const Ty * py, long spos, long tpos, long epos) : px_(px)
                                                                              , py_(py)
                                                                              , spos_(spos)
                                                                              , tpos_(tpos)
                                                                              , epos_(epos) {
        }
        void thresholdTopRatio(double r /* 0.0 .. 1.0 */) {
            Ty h = (py_[spos_] + py_[epos_]) / 2;
            thresholdLevel((py_[tpos_] - h) * r + h);
        }
        void thresholdValleyRatio(double r /* 0.0 .. 1.0 */) {
            Ty h = (py_[spos_] < py_[epos_]) ? py_[epos_] : py_[spos_];
            thresholdLevel((py_[tpos_] - h) * r);
        }
        void thresholdLevel(double h) {
            threshold_ = h;
            compute();
        }
        double thresholdLevel() const {
            return threshold_;
        }
        double centerX() const {
            return Xc_;
        }
        double centerY() const {
            return py_[tpos_];
        }
        double leftBoundX() const {
            return Xl_;
        }
        double rightBoundX() const {
            return Xr_;
        }
        double GetInterporatedResult(long ndiv, std::vector<double> & X, std::vector<double> & Y) {
            long xL = left_bound();
            long xR = right_bound();
            double Xl = left_intersection(xL);
            double Xr = right_intersection(xR);
    
            X.clear();
            Y.clear();
            double baseH = threshold_;
    
            for (long i = xL - 1; i < xR + 1; ++i) {
                for (long k = 0; k < ndiv; ++k) {
                    double x = k * (px_[i + 1] - px_[i]) / ndiv + px_[i];
                    if ((x >= Xl) && (x <= Xr)) {
                        double y = k * (py_[i + 1] - py_[i]) / ndiv + py_[i];
                        X.push_back(x);
                        Y.push_back(y - baseH);
                    }
                }
            }

            double x0 = px_[xL - 1];
            double ma = 0;
            double ta = 0;
            for (long n = 0; n < X.size(); ++n) {
                ta += Y[n];
                ma += (X[n] - x0) * Y[n];
            }
            return  ma / ta + x0;
        }
    };
#endif
}
