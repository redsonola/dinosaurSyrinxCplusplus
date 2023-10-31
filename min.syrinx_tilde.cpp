/// @file
///	@ingroup 	syrinx
///	@copyright	Copyright 2018 The Min-DevKit Authors. All rights reserved., Modifications Copyright 2023, Courtney Brown. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.
///
/// Programmer: Courtney Brown
/// Date: Oct. 2023
/// An external max object which using physically-based modeling synthesis of a bird syrinx. Parameters are adjusted for an adult Corythosaurus currently
/// In process -- parameters will be exposed to change trachea/syrinx membrane size
/// Used the phasor_tilde as a starting point for max external, but replaced almost all the code with my code

#include "c74_min.h"
#include "syrinx_membrane.h"
#include "full_syrinx.h"

#include <sstream>

using namespace c74::min;

class syrinx : public object<syrinx>, public sample_operator<0, 1> {
private:
    //lib::sync m_oscillator;    // note: must be created prior to any attributes that might set parameters below
    
protected:
    SyrinxMembraneGenerator m_syrinx;
    double sampleRate = 0;
    int count = 0;

public:
//    MIN_DESCRIPTION	{ "A non-bandlimited sawtooth oscillator (a phasor~ in MSP parlance)."
//                      "This <a href='https://en.wikipedia.org/wiki/Sawtooth_wave'>sawtooth wave</a>"
//                      "is typically used as a control signal for <a "
//                      "href='https://en.wikipedia.org/wiki/Phase_(waves)'>phase</a> ramping." };
//    MIN_TAGS		    { "audio, oscillator" };
//    MIN_AUTHOR		{ "Cycling '74" };
//    MIN_RELATED		{ "phasor~, saw~" };
    
    MIN_DESCRIPTION   { "A computational model of a syrinx based on the Fletcher (1988) model"
                        "and the Symth (2002-4) implementation. Implemeted to model a" "Corythosaurus syrinx"};
    MIN_TAGS          { "audio, physically-based modeling, dinosaur, bird" };
    MIN_AUTHOR        { "Courtney Brown" };
    
    inlet<>  in1 {this, "(number) air sac pressure (pG)"};
    inlet<>  in2 {this, "(number) tension (N/cm^3)"};

    outlet<> out1 {this, "(signal) syrinx output pressure (sound)", "signal"};


    argument<number> tension_arg { this, "tension", "Initial tension in N/cm^3.",
        MIN_ARGUMENT_FUNCTION {
            tension = arg;
            m_syrinx.setTension(arg);
            m_syrinx.setSampleRate(samplerate());
        }
    };

    message<> m_number { this, "number", "Set the tension in N/cm^3.",
        MIN_FUNCTION {
//            sampleRate = samplerate(); //this isn't set in dspinfo??
//            cout << "sampleRate: " << sampleRate << endl;
//            m_syrinx.setSampleRate(sampleRate);

            switch(inlet)
            {
                case 0:
                    m_syrinx.setPG(args[0]);
//                    if( m_syrinx.getPG()>0 )
//                        cout << "pG: " << m_syrinx.getPG() << endl;
                    break;
                case 1:
                    m_syrinx.setTension(args[0]);
                    //cout << "tension: " << m_syrinx.getTension() << endl;
                    break;
                default:
                    assert(false);
            }
            return {};
        }
    };
    
    message<> dspsetup { this, "dspsetup",
        MIN_FUNCTION {
            sampleRate = samplerate();
            m_syrinx.setSampleRate(sampleRate);
            
            //int vectorsize = args[1];

            //m_one_over_samplerate = 1.0 / samplerate;
            return {};
        }
    };

    attribute<number> tension { this, "tension", 1.0,
        description {"Tension in N/cm^3"},
        setter { MIN_FUNCTION {
            m_syrinx.setTension(args[0]);
            return args;
        }}
    };

    sample operator()() {
        sample out = m_syrinx();
//        if(count % 44100/4410 != 0){
//        if( out != 0  ){
//            cout << "sample: " << out << endl;
//            count = 1;
//            cout << m_syrinx.getPhysConstants() << "  ,  " <<
//            m_syrinx.getPreshDiff() << "  ,  " <<
//            m_syrinx.getZG() << "  ,  " <<
//            m_syrinx.getPG() << "  ,  " <<
//            m_syrinx.getDPO() <<endl;
        
//        }
//        count++;
        return out;
    }
};

MIN_EXTERNAL(syrinx);
