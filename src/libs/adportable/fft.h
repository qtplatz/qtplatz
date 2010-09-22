/*
 * FFT and related algolism for spectral peak picking
 * Originally created by T. Hondo, '90, 01/09
 */

#pragma once

#include <vector>
#include <complex>
#include <cstdlib>
#include <limits>

namespace adportable {

	class fft {
	public:
		static bool apodization(int napod, std::vector< std::complex<double> > &);
		static bool apodization(int napod_zero, int napod_slope, std::vector< std::complex<double> > &);
		static size_t zero_filling(size_t toSize, std::vector< std::complex<double> > &);

		static bool fourier_transform(std::vector< std::complex<double> > & result, 
			                          std::vector< std::complex<double> > & x, bool ist);

		class spectrum {
			double m_WLstart;
			double m_WLend;
			long m_nBinarySize;  // should be power of two
			std::vector< std::complex<double> > m_power_spectrum;
			std::vector< double > m_data;
		public:
			spectrum();
			spectrum(int n, double * data, double startX, double endX, size_t toSize, size_t napod);
			bool peak_picking(std::vector< std::pair<double, double> > & peaks, 
				std::vector< std::pair<double, double> > & valays);
			size_t get_spectrum(std::vector<double> & rdata, double & startX, double & endX);
			operator std::vector< double > & ();
		};
	};

}
