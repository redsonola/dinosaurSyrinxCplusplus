//
//  full_syrinx.h
//  min.syrinx_tilde
//
//  Created by courtney on 10/13/23.
//

#ifndef full_syrinx_h
#define full_syrinx_h

#include "syrinx_membrane.h"
#include "ReflectionFilter.h"
#include "WallLossFilter.h"
#include "HPOut.h"
#include "ScatteringJunction.h"
#include "DelayLine.h"

#include <limits>


using namespace c74::min;

template<typename T>
struct GenerateFunctionType {
    double (T::*generateFunction)();     // Pointer to additional initialization method
};

class SyrinxMembraneGenerator {
    
protected:
    
    std::stringstream messages;
    
    double lastSample = 0; //last sample from the output
    double lastSample2 = 0; //from 2nd membrane
    double lastTracheaSample = 0;
    
    double tension =0;
    double pG = 0;
    
    //max pressure allowed, used as a scale to normalize the values coming out
    double max;
    int count=0; //used to output debugging information at a lower rate, counts # of render frames
    
    //parameters of the syrinx membrane
    
//    double pG; //air sac pressure
//    double tension; //membrane tension, and left tension when 2 membranes
    
    double rightTension; //only used with independent membranes

    //syrinx membranes
    double membraneCount = 2;
    SyrinxMembrane membrane;
    SyrinxMembrane membrane2;
    
    //filters for wall loss, trachea reflection, and out
    WallLossAttenuation wallLoss;
    HPFilter hpOut;
    
    //scattering junction for the joining of 2 tubes to 1 (eg, brochi to trachea)
    ScatteringJunction scatteringJunction;
    
    double c = membrane.getSoundSpeed();
    ReflectionFilter bronch1Filter{};
    ReflectionFilter bronch2Filter{};
    ReflectionFilter tracheaFilter{};
    
    
    bool independentMembranes = false;
    
    double delayTimeBronchi = 0; //delay of each of the bronchi sides
    double delayTime = 0; //the trachea delay time
    
    double sampleRate = 44100;
    
    double L = 116; //trachea length
    
    //get a channel count...?
    //int channelCount = options.channelCount;
    
    //one for each way   -- //delay(std::pair<size_t, number> capacity_and_size)
    lib::delay bronch1Delay1{};
    lib::delay bronch1Delay2{};
    
    lib::delay bronch2Delay1{};
    lib::delay bronch2Delay2{};
    
    lib::delay tracheaDelay1{};
    lib::delay tracheaDelay2{};
    
    //generate function
    GenerateFunctionType<SyrinxMembraneGenerator> genFunction;
    
    void hadrosaurInit()
    {
        L = 116; //trachea length
        
        //in cm
        membrane.set_a(4.5);
        membrane.set_h(4.5);
        membrane.set_L(L); //dummy, but just in case I use a length in the membrane? length is handled here.
        membrane.set_d(5);
        
        //in various
        membrane.set_modT(10000.0);
        membrane.set_modPG(80);
        
        membrane.initTension();
        //membrane.initZ0();
        
        //in cm
        membrane2.set_a(4.5);
        membrane2.set_h(4.5);
        membrane2.set_L(L); //dummy, but just in case I use a length in the membrane? length is handled here.
        membrane2.set_d(5);
        
        //in various
        membrane2.set_modT(10000.0);
        membrane2.set_modPG(80);
        
        membrane2.initTension();
        //membrane2.initZ0();
        
    }
    
    //setMembraneCount
    void setMembraneCount(double mcount)
    {
        if( membraneCount != mcount)
        {
//            messages << "changing membrane count to: " << mcount << endl;
//            messages << " membraneCount: " <<  membraneCount << endl;
            
            membraneCount = mcount;
//            messages << " membraneCount: " <<  membraneCount << endl;
            
            if(  membraneCount <= 1 )
            {
                setDelayTime( L );
//                genFunction.generateFunction = &SyrinxMembraneGenerator::generateTrachealSyrinx;
                
            }
            else
            {
                setDelayTime( L );
//                genFunction.generateFunction = &SyrinxMembraneGenerator::generateTracheobronchial;
            }
            initFunc(); //reinitialize
        }
    }
    
    void clearDelays()
    {
        bronch1Delay1.clear();
        bronch1Delay2.clear();
        bronch2Delay1.clear();
        bronch2Delay2.clear();
    
        tracheaDelay1.clear();
        tracheaDelay2.clear();
        
    }
    
    void createDelays()
    {
        clearDelays();
        
        //one for each way
        bronch1Delay1.size( delayTimeBronchi );
        bronch1Delay2.size( delayTimeBronchi );

        bronch2Delay1.size( delayTimeBronchi );
        bronch2Delay2.size( delayTimeBronchi );

        tracheaDelay1.size( delayTime );
        tracheaDelay2.size( delayTime );
    }
    
    void initFunc()
    {
        
        lastSample = 0;
        lastSample2 = 0;
        lastTracheaSample = 0;
        
        //the delay of comb filter for the waveguide
        //which syrinx
        //            if( membraneCount >= 2)
        //                generateFunction =  generateTracheobronchial;
        //            else
        //                generateFunction =  generateTrachealSyrinx;
        //            setDelayTime(  getDelayPeriod(L) );
        
        //hadrosaur values for now
        hadrosaurInit();
        
        //one for each way
        createDelays();
        
        //set new refelection values
        bronch1Filter.setCT( c, membrane.get_T() );
        bronch2Filter.setCT( c, membrane2.get_T() );
        tracheaFilter.setCT( c, membrane.get_T() );
        
        //the reflection filters for each tube: bronchi & trachea
        bronch1Filter.setParamsForReflectionFilter( membrane.get_a());
        bronch2Filter.setParamsForReflectionFilter( membrane.get_a());
        tracheaFilter.setParamsForReflectionFilter( membrane.get_a());
        
        //we'll need separate wall losses as well...
        wallLoss.update( membrane.get_L(),  membrane.get_a());
        hpOut.setParams( tracheaFilter.get_a1(),  tracheaFilter.get_b0());
        
        scatteringJunction.updateZ0(membrane.get_z0());
        
        max = 51520; //for some simple scaling into more audio-like numbers -- output from this should still be passed on to limiter
        //TODO: perhaps implementing something custom for this, we'll see.
        
        
        count = 0; //for outputting values at a lower rate
    }
    
public:
    explicit SyrinxMembraneGenerator(double sampleRate_ = 44100) :
    bronch1Delay1(std::pair<size_t, number>(getBronchiDelay()*2, getBronchiDelay())),
    bronch1Delay2(std::pair<size_t, number>(getBronchiDelay()*2, getBronchiDelay())),
    bronch2Delay1(std::pair<size_t, number>(getBronchiDelay()*2, getBronchiDelay())),
    bronch2Delay2(std::pair<size_t, number>(getBronchiDelay()*2, getBronchiDelay())),
    tracheaDelay1(std::pair<size_t, number>(getTracheaDelay()*2, getTracheaDelay())),
    tracheaDelay2(std::pair<size_t, number>(getTracheaDelay()*2, getTracheaDelay()))
    {
        sampleRate = sampleRate_;
        
        hadrosaurInit();
        
        bronch1Filter.setCT(c, membrane.get_T());
        bronch2Filter.setCT(c, membrane.get_T());
        tracheaFilter.setCT(c, membrane.get_T());
        
        initFunc();
        
        //setMembraneCount(1);
    }
    
    void setSampleRate(double sampleRate_ = 44100)
    {
        if( sampleRate != sampleRate_ )
        {
            sampleRate = sampleRate_;
            initFunc();
        }
    }
    
    //collect from messages to be printed to max console and clear after.
    std::string getMessages()
    {
        std::string str = messages.str();
        messages.clear();
        return str;
    }
    
    void setPG(double pG_)
    {
        pG = pG_;
        membrane.changePG( pG_ );
    }
    
    void setTension(double tension_)
    {
        tension = tension_;
        membrane.changeTension( tension_ );
    }
    
    double getTension()
    {
        return membrane.getTension();
    }
    
    double getPG()
    {
        return membrane.getPG();
    }
    
    double getDelayPeriod(double tubeLength)
    {
        double c = membrane.getSoundSpeed();
        double LFreq = c/(2.0*tubeLength); //- the resonant freq. of the tube
        double period =  ( (0.5 * sampleRate) / (LFreq) - 1); //in samples
        return period;
    }
    
    //set delay time
    void  setDelayTime(double tubeLength)
    {
        delayTime = getDelayPeriod(tubeLength);
        
        //values from Smyth 2002, in mm
        //large bird - bronchus to trachea - 30/35.6 -- taking the large bird radius (still a small bird)
        //medium bird - bronchus to trachea - 14/23
        //small bird - bronchus to trachea - 5/17.1

        //let's say the MTM bronchi to the trachea length is 1/15 of the trachea
        //I got this by looking at diagrams
        delayTimeBronchi = delayTime*(30/35.6);
        
    }
    
    //init delay first, then give delay time
    double getTracheaDelay()
    {
        setDelayTime(L);
        return delayTime;
    }
    
    double getBronchiDelay()
    {
        setDelayTime(L);
        return delayTimeBronchi;
    }

    void setIndependentMembranes(bool isIndependent)
    {
        if( independentMembranes != isIndependent)
        {
            independentMembranes = isIndependent;
            initFunc(); //reinitialize
        }
    }
    
    //generate the sound using selected algorithm for the syrinx membrane
    //using min-devkit standard, operator(), for the call to process /tick() (tick heldover from chuck)
    sample operator()() {
        /***** change membrane count if needed *****/
        
        //NOTE: can do this via messages now in Max 8, but not yet implemented
        //         setMembraneCount(parameters.membraneCount);
        //         setIndependentMembranes(parameters.independentMembranes);
        //return  ((*this).*(genFunction.generateFunction))() ;  //default is Tracheobronchial for now, membraneCount = 2
    
        return generateTrachealSyrinx(); //test for now
    }
    
    double getPhysConstants(){ return membrane.getPhysConstants(); }
    double getPreshDiff(){ return  membrane.getPreshDiff();}  ;
    double getDPO() { return membrane.getDPO(); }
    double getZG() {return membrane.getZG();}
    
    //generate one sample
    sample generateTrachealSyrinx()
    {
        
        if(  isnan( lastSample) )
        {
            lastSample = 0;
        }
        
        if(  isnan( lastSample2) )
        {
            lastSample2 = 0;
        }
        
        if(  isnan( lastTracheaSample) )
        {
            lastTracheaSample = 0;
        }
        
        //*****implementing this chuck code.....
        
        //from membrane to trachea and part way back
        //SyrinxMembrane mem => DelayA delay => lp => Flip flip => DelayA delay2 => WallLossAttenuation wa; //reflection from trachea end back to bronchus beginning
        
        //the feedback from trachea reflection
        //Gain p1;
        //wa => p1;
        //mem => p1;
        ////p1 => DelayA oz =>  mem; //the reflection also is considered in the pressure output of the syrinx
        //p1 =>  mem; //the reflection also is considered in the pressure output of the syrinx
        //p1 => delay;
        //************
        
        membrane.changeTension(tension);
        membrane.changePG(pG);

        double pOut =  membrane.tick( lastSample ); //the syrinx membrane

        //********1st delayLine => tracheaFilter => flip => last sample  *********
        double curOut =  delayLineGenerate( pOut + lastSample, tracheaDelay1 );
        lastSample =  tracheaFilter.tick( curOut ); //low-pass filter
        lastSample =  lastSample * -1; //flip
        
        //********2nd delayLine, going back *********
        lastSample =  delayLineGenerate( lastSample, tracheaDelay2 );
        lastSample =  wallLoss.tick( lastSample );
        
        //******** Add delay lines ==> High Pass Filter => out  *********
        curOut = curOut +  lastSample;
        double fout =  hpOut.tick(curOut);
        
        //****** simple scaling into more audio-like values, sigh  *********
        fout = fout / max;
        
        if( isnan( fout ) )
        {
            //messages << " NaN detected.... relaunching " << endl;
            initFunc();
        }

        //****** output w/high pass *********
        return fout;
    }
    
    //this is the delayLine generate -- this did not really have to be a separate method --
    //this is leftover from javascript implementation where more had to be done.
    //note: could use operator()
    sample delayLineGenerate( double input, lib::delay &delayLine )
    {
        auto delayedSample = delayLine(input);
        return delayedSample;
    }
    
        //2 membranes, as in passerines
    sample generateTracheobronchial()
    {
    
///FIX
//        if( independentMembranes )
//        {
//            membrane2.changeTension(rightTension);
//        }
//        else
//        {
//            membrane2.changeTension(tension);
//        }
//        membrane2.changePG(pG);
    
        //  count++;
        // if( count >50)
        // {
        //    // console.log( membrane.pG + ", " + parameters.tension);
        //      count = 0;
        //     let res = parameters.rightTension - parameters.tension;
        //     console.log("tension different: " + res);
        // }
    
        //******** Sound is generated and travels up each bronchi *********
        double pOut =  membrane.tick( lastSample); //the syrinx membrane  //Math.random() * 2 - 1;
        double curOut =  delayLineGenerate(pOut+lastSample,  bronch1Delay1);
        lastSample =  bronch1Filter.tick(curOut); //low-pass filter
        lastSample =  lastSample * -1; //flip
    
        double pOut2 =  membrane2.tick( lastSample2); //the syrinx membrane  //Math.random() * 2 - 1;
        double curOut2 =  delayLineGenerate(pOut2+lastSample2, bronch2Delay1);
        lastSample2 =  bronch2Filter.tick(curOut2); //low-pass filter
        lastSample2 =  lastSample2 * -1; //flip
    
        //******** Sound encounters Junction *********
        PostScatteringPressures scatterOut =  scatteringJunction.scatter(curOut, curOut2,  lastTracheaSample);
    
        //******** Sound travels through trachea *********
        double trachOut =  delayLineGenerate(scatterOut.trach+lastTracheaSample,  tracheaDelay1);
        lastTracheaSample =  tracheaFilter.tick(trachOut); //low-pass filter
        lastTracheaSample =  lastTracheaSample * -1; //flip
    
        //******** Sound is reflected from bronchi & trachea *********
        lastSample =  delayLineGenerate(scatterOut.b1, bronch1Delay2);
        lastSample =  wallLoss.tick( lastSample);
    
        lastSample2 =  delayLineGenerate(scatterOut.b2,bronch2Delay2);
        lastSample2 =  wallLoss.tick( lastSample2);
    
        lastTracheaSample =  delayLineGenerate( lastTracheaSample, tracheaDelay2);
        //lastTracheaSample =  wallLoss.tick( lastTracheaSample);
    
        //******** Add delay lines ==> High Pass Filter => out  *********
        trachOut = trachOut +  lastTracheaSample;
        double fout =  hpOut.tick(trachOut);
    
        //****** simple scaling into more audio-like values, sigh  *********
        fout = fout/max;
    
        if( isnan(fout) )
        {
            //messages << " NaN detected.... relaunching " << endl;
            initFunc();
        }
        
//        if( fout <= std::numeric_limits<float>::min() )
//        {
//            fout = 0;
//        }
//        else if (fout > 1.0)
//        {
//            max = fout;
//            fout = fout / max;
//        }
        return fout;
    }
};


#endif /* full_syrinx_h */
