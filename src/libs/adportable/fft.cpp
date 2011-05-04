/*
 * FFT and related algolism for spectral peak picking
 * Originally created by T. Hondo, for DP-L340	'90, 01/09
 *
 * Rewrite to C++ for Chromix in 2000
 *
 * Rewrite for MS data noize reduction trial program, December 2002
 *
 */

#include "fft.hpp"

using namespace adportable;

fft::spectrum::spectrum()
{
}

fft::spectrum::spectrum(int n, double * data, double WLstart, double WLend, size_t toSize, size_t napod)
{
  m_WLstart = WLstart;
  m_WLend = WLend;
  if (n <= 32)
    return;

  for (int i = 0; i < n; ++i)
    m_power_spectrum.push_back( std::complex<double>(data[i]) );
  int m_nBinarySize = 2;
  while (m_nBinarySize < n)
    m_nBinarySize *= 2;
  if (m_nBinarySize > n) {
    while (size_t(m_nBinarySize) > m_power_spectrum.size())
      m_power_spectrum.push_back( std::complex<double>(data[n - 1]) );
    m_WLend = (((m_WLend - m_WLstart) / (n - 1)) * m_nBinarySize) + m_WLstart;
  }
  std::vector< std::complex<double> > interferrogram;

  fft::fourier_transform(interferrogram, m_power_spectrum, true);
  fft::apodization(int(napod), interferrogram);
  fft::zero_filling(toSize, interferrogram);

  fft::fourier_transform(m_power_spectrum, interferrogram, false);

  std::vector< std::complex<double> >::iterator pos;
  for (pos = m_power_spectrum.begin(); pos != m_power_spectrum.end(); ++pos)
    m_data.push_back(pos->real() * (toSize / m_nBinarySize));
}

fft::spectrum::operator std::vector< double > & ()
{
  return m_data;
}

size_t
fft::spectrum::get_spectrum(std::vector<double> & spc, double & WLstart, double & WLend)
{
  spc = m_data;
  WLstart = m_WLstart;
  WLend = m_WLend;
  return spc.size();
}

bool
fft::apodization(int napod, std::vector< std::complex<double> > & x)
{
  if (x.empty())
    return false;
  if (napod == 0)
    return false;
  int c = int(x.size() / 2) - 1;
  for (int i = 0; i < int(napod); ++i) {
    x[c - i] *= (double)i / napod;
    x[c + i + 1] *= (double)i / napod;
  }
  return true;
}

bool
fft::apodization(int napod_zero, int napod_slope, std::vector< std::complex<double> > & x)
{
  if (x.empty())
    return false;
  if ((napod_zero == 0) && (napod_slope == 0))
    return false;
  int c = int(x.size() / 2) - 1;
  int i;
  for (i = 0; i < int(napod_zero); ++i) {
    x[c - i] = x[c + i + 1] = 0;
  }
  if ((napod_slope - napod_zero) <= 0)
    return true;
  for (; i < napod_slope; ++i) {
    x[c - i] *= (double)(i - napod_zero) / (napod_slope - napod_zero);
    x[c + i + 1] *= (double)(i - napod_zero) / (napod_slope - napod_zero);
  }
  return true;
}

size_t
fft::zero_filling(size_t toSize, std::vector< std::complex<double> > & x)
{
  if (x.empty())
    return 0;
  if (toSize < x.size())
    return 0;
  int nInsert = int(toSize - x.size());
  std::vector< std::complex<double> >::iterator pos = x.begin();
  size_t n = x.size();
  for (size_t i = 0; i < (n / 2); ++i)
    ++pos;
  x.insert(pos, nInsert, std::complex<double>(0,0));
  return x.size();
}

/*******************************************
FFT.C	FAST FOURIER TRANSFORM (REVISED)
SUBROUTINE fourier_transform(result, x, ist)
y(N)...OUTPUT DATA
x(N)...INPUT DATA
ist = false.. FORWARD TRANSFORM
ist = true.. INVERSE TRANSFORM
RETURN false.. ERROR RETURN
Translated from FORTRAN IV to Lattice C
	by T. Hondo		Apr. 27, 1986
Translated from Lattice C to STL/C++
        by T. Hondo             Sep. 05, 2001
********************************************/
#include	<math.h>

#if !defined M_PI
//# if defined _PI
//#    define	M_PI	_PI
//# else
#    define	M_PI	3.14159265358979323846
//# endif
#endif

using namespace std;

bool 
fft::fourier_transform( std::vector< std::complex<double> > & Y, 
					   std::vector< std::complex<double> > & X, bool ist)
{
  // int
  // FFT(double xr[], double xi[], double yr[], double yi[], int n, int ist)
  long IL, NS;
  long N2, IS, IS2, J, JA, JB, KA, KB, KS;
  long i;
  double WARG;
  const long n = static_cast<long>(X.size());

  Y.clear();
  for (NS = n, IL = 0; (NS /= 2) > 0; ++IL)
    ;
  for (i = 2; i < n; i *= 2)
    ;
  if (n != i)
    return false;

  Y.resize(n);
  WARG = -2.0 * M_PI / (double)n;
  if (ist == 1)
    WARG = (-WARG);
  
  N2 = n / 2;
  KS = N2;
  IS = 1;
  for (long l = 0; l < IL; ++l) {
    IS2 = IS * 2;
    JA = 0;
    for (i = 0; i < n; i += IS2)
      for (J = 1; J <= IS; ++J) {
        double WS = WARG * (double)((J - 1) * KS);
        std::complex<double> W(cos(WS), sin(WS));
        ++JA;
        JB = JA + N2;
        KA = i + J;
        KB = KA + IS;
        Y[KA - 1] = X[JA - 1] + X[JB - 1] * W;     /* Y[KA]=X[JA]+X[JB]*W */
        Y[KB - 1] = X[JA - 1] - X[JB - 1] * W;     /* Y[KB]=X[JA]-X[JB]*W */
      }
      IS = IS2;
      KS = KS / 2;
      for (i = 0; i < n; ++i)
        X[i] = Y[i];
  }
  if (ist == false) { // forward transform
    std::complex<double> R(1.0 / n);
    for (i = 0; i < n; ++i)
      X[i] *= R;
  }
  for (i = 0; i < n; ++i)
    Y[i] = X[i];
  return true;
}

