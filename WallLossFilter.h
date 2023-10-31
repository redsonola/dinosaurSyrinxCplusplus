//
//  WallLossFilter.h
//  min.syrinx_tilde
//
//  Created by courtney on 10/16/23.
//

#ifndef WallLossFilter_h
#define WallLossFilter_h

class WallLossAttenuation //tuned to dino
{
protected:
    double L = 116.0; //in cm --divided by 2 since it is taken at the end of each delay, instead of at the end of the waveguide
    double c = 34740; // in m/s
    double freq = c/(2*L);
    double w  = wFromFreq(freq);
    //150.0*2.0*pi => float w;
    
    double a = 0.35; //  1/2 diameter of trachea, in m - NOTE this is from SyrinxMembrane --
                     //TODO:  to factor out the constants that are used across scopes: a, L, c, etc
    double propogationAttenuationCoeff = calcPropogationAttenuationCoeff(); //theta in Fletcher1988 p466
    double wallLossCoeff = calcWallLossCoeff(); //beta in Fletcher
    const double WALLLOSSFILTER_DROPOFF_CONSTANT = 5*pow(10, -5); //in calcPropogationAttenuationCoeff()
    
public:
    WallLossAttenuation()
    {
    }
    
    //update given new length & width
    void update(double L_, double a_)
    {
        L = L_;
        a = a_;
        freq = c/(2*L_) ;
        w = wFromFreq(freq);
        propogationAttenuationCoeff = calcPropogationAttenuationCoeff();
        wallLossCoeff = calcWallLossCoeff();
    }
    
    double calcWallLossCoeff()
    {
        return 1.0 - (2.0*propogationAttenuationCoeff*L);
    }
    
    double calcPropogationAttenuationCoeff()
    {
        return ( WALLLOSSFILTER_DROPOFF_CONSTANT*sqrt(w) ) / a; //changed the constant for more loss, was 2.0, intuitive tuning, TODO: implement cascade filters, or look at how to adjust constant
    }
    
    double wFromFreq(double frq)
    {
        return frq * M_PI * 2;
    }
    
    void setFreq( double f )
    {
        wFromFreq(f);
    }
    
    //the two different bronchi as they connect in1 & in2
    double getWallLossCoeff()
    {
        return wallLossCoeff;
    }

    double tick( double input )
    {
        return input * wallLossCoeff;
    }
};

#endif /* WallLossFilter_h */
