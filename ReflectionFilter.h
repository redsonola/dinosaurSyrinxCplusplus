//
//  ReflectionFilter.h
//  min.syrinx_tilde
//
//  Created by courtney on 10/16/23.
//

#ifndef ReflectionFilter_h
#define ReflectionFilter_h


#include <math.h>
#include <cmath>
#include <complex>

using namespace std::complex_literals;
//using namespace c74::min;

class ReflectionFilter
{
protected:
    double a1 = 1.0;
    double b0 = 1.0;
    double lastOut = 0.0 ;
    double lastV = 0.0 ;
    double c = 0; //speed of sound
    double T = 0; //sample period
    
public:
    //get some constants from the enclosing membrane class
    ReflectionFilter( double c_, double T_ )
    {
        c = c_;
        T = T_;
        a1 = 0;
        b0 = 0;
        lastOut = 0;
        lastV = 0;
    }
    
    ReflectionFilter()
    {
        a1 = 0;
        b0 = 0;
        lastOut = 0;
        lastV = 0;
    }
    
    void reset()
    {
        lastOut = 0;
        lastV = 0;
    }
    
    void setCT(double c_, double T_)
    {
        c = c_;
        T = T_;
    }
    
    double tick( double input )
    {
        if(  isnan( lastOut ) )
        {
            lastOut = 0;
        }
        if(  isnan( lastV ) )
        {
            lastV = 0;
        }
        double vin = b0 * input;
        double out1 = vin + lastV - a1 * lastOut;
        lastOut = out1;
        lastV = vin;
        
        return out1;
    }

    void setParamsForReflectionFilter( double a )
    {
   
        double ka = 1.8412; //the suggested value of 0.5 by Smyth by ka & cutoff does not seem to work with given values (eg. for smaller a given) & does not produce
        double wT = ka*( c/a )*T;
             
        //magnitude of -1/(1 + 2s) part of oT from Smyth, eq. 4.44
        double s = ka; //this is ka, so just the coefficient to sqrt(-1), s to match Smyth paper
        
        //TODO: implement complex numbers >_<
        std::complex<double> numerator(-1, 0);
        std::complex<double> denominator(1, 2*s);
        std::complex<double> complexcHr_s =  numerator / denominator ;
        double oT = sqrt( std::real(complexcHr_s)*std::real(complexcHr_s) + std::imag(complexcHr_s)*std::imag(complexcHr_s) ); //magnitude of Hr(s)
        
        double alpha = ( 1 + cos(wT) - 2*oT*oT*cos(wT) ) / ( 1 + cos(wT) - 2*oT*oT ); //to determine a1
        a1 = -alpha + sqrt( alpha*alpha - 1 );
        b0 = (1 + a1 ) / 2;
    }
    
    double get_a1()
    {
        return a1;
    }
    
    double get_b0()
    {
        return b0;
    }
   
};


#endif /* ReflectionFilter_h */
