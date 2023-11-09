//
//  HPOut.h
//  min.syrinx_tilde
//
//  Created by courtney on 10/16/23.
//

#ifndef HPOut_h
#define HPOut_h

#include <math.h>

class HPFilter
{
protected:
    double a1 = 1;
    double b0 = 1;
    double lastOut = 0;
    double lastV = 0;
    
public:

    HPFilter()
    {
        lastOut = 0;
        lastV = 0;

    }
    
    void setParams(double a1_, double b0_)
    {
        a1 = a1_;
        b0 = b0_;
    }
    
    void reset()
    {
        lastOut = 0;
        lastV = 0;
    }
    
    double tick( double input )
    {
        if(  isnan(lastOut) )
        {
            lastOut = 0;
        }
        if(  isnan(lastV) )
        {
            lastV = 0;
        }

        double vin = input - b0*input;
        double output = vin + ( a1 - b0 )* lastV - a1*lastOut;
        lastOut = output;
        lastV = vin;
        return output;
    }
};

#endif /* HPOut_h */
