kATOMIC_MASS_CONSTANT = 1.660538921e-27
kELEMENTAL_CHARGE    = 1.60217733e-19
K = 2.0 * kELEMENTAL_CHARGE / kATOMIC_MASS_CONSTANT
kK = kATOMIC_MASS_CONSTANT / kELEMENTAL_CHARGE
LT = 0.66273
Lh = 0.06626 * 2 + 0.30512
Lk = Lh + 0.08806
Ld = 0.08806

m129Xe = 128.9047794
m130Xe = 129.9035080
m131Xe = 130.9050824
m132Xe = 131.9041535
m134Xe = 133.9053945

length( n_ ) = Lk + LT * n_

vacc( m_, t_, l_, t0_ ) = (m_ * (l_ * l_) * kK ) / ( 2 * (t_ - t0_)*(t_ - t0_) )

tof( m_, L_, V_, t0_ ) = sqrt( (m_*(L_*L_)) / (K * V_) ) + t0_

tof2( m_, L_, V_, t0_ ) = ( sqrt( (m_*(L_*L_)) / (K * V_) ) + t0_ ) * 1e6

time_n( m, n, v, t0 ) = sqrt( (m*(length(n)*length(n))) / (K * v) ) + t0

mass_t( t, l, v, t0 ) = ((K * v)/(l*l)) * ((t-t0) * (t-t0))
mass_n( t, n, v, t0 ) = ((K * v)/(length(n)*length(n))) * ((t-t0) * (t-t0))

linear_length( m, v, t, t0 ) = (t - t0)*sqrt((2*v*kELEMENTAL_CHARGE)/(m*kATOMIC_MASS_CONSTANT))
