// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////
#ifndef ROLEANALYZER_H
#define ROLEANALYZER_H

#include <string>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include "device_state.h"

namespace analyzer {
    struct analyzer_data {
        analyzer_data();
        analyzer_data( const analyzer_data& ); // copy
        std::string model;
        std::string hardware_rev;
        std::string firmware_rev;
        std::string serailnumber;
        unsigned long __head__;
        unsigned long positive_polarity;  // bool
        unsigned long ionguide_bias_voltage;
        unsigned long ionguide_rf_voltage;
        unsigned long orifice1_voltage;
        unsigned long orifice2_voltage;
        unsigned long orifice4_voltage;
        unsigned long focus_lens_voltage;
        unsigned long left_right_voltage;
        unsigned long quad_lens_voltage;
        unsigned long pusher_voltage;
        unsigned long pulling_voltage;
        unsigned long supress_voltage;
        unsigned long pushbias_voltage;
        unsigned long mcp_voltage;
        unsigned long accel_voltage;  // digital value
        // unsigned long drying_gas_flow;  move to 'esi'
        // unsigned long curtin_gas_flow;
        // unsigned long nebulizer1_flow;
        unsigned long __tail__;
    private:
	    friend class boost::serialization::access;
	    template<class Archive> void serialize(Archive& ar, const unsigned int version) {
            (void)version;
            ar & BOOST_SERIALIZATION_NVP(model) 
                & BOOST_SERIALIZATION_NVP(hardware_rev) 
                & BOOST_SERIALIZATION_NVP(firmware_rev) 
                & BOOST_SERIALIZATION_NVP(serialnumber) 
                & BOOST_SERIALIZATION_NVP(positive_polarity)
                & BOOST_SERIALIZATION_NVP(ionguide_bias_voltage) 
                & BOOST_SERIALIZATION_NVP(ionguide_rf_voltage) 
                & BOOST_SERIALIZATION_NVP(orifice1_voltage) 
                & BOOST_SERIALIZATION_NVP(orifice2_voltage) 
                & BOOST_SERIALIZATION_NVP(orifice4_voltage) 
                & BOOST_SERIALIZATION_NVP(focus_lens_voltage) 
                & BOOST_SERIALIZATION_NVP(left_right_voltage)
                & BOOST_SERIALIZATION_NVP(quad_lens_voltage)
                & BOOST_SERIALIZATION_NVP(pusher_voltage)
                & BOOST_SERIALIZATION_NVP(pulling_voltage)
                & BOOST_SERIALIZATION_NVP(supress_voltage)
                & BOOST_SERIALIZATION_NVP(pushbias_voltage)
                & BOOST_SERIALIZATION_NVP(mcp_voltage)
                & BOOST_SERIALIZATION_NVP(accel_voltage)
                ;
        }
    };
    struct analyzer_actuals : public analyzer_data {
    };
    struct analyzer_setpts : public analyzer_data {
    };
}

class RoleAnalyzer : public device_state {
public:
    ~RoleAnalyzer();
    RoleAnalyzer();
    RoleAnalyzer( const RoleAnalyzer& );
    bool operator == ( const RoleAnalyzer& ) const { return true; }
    const char * deviceType() const { return "analyzer"; }
private:
    analyzer::analyzer_setpts setpts_;
    analyzer::analyzer_actuals actuals_;
};

#endif // ROLEANALYZER_H
