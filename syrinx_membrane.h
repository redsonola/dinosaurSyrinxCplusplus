//
//  syrinx_membrane.h
//  min.syrinx_tilde
//
//  Created by courtney on 10/5/23.
//

#ifndef syrinx_membrane_h
#define syrinx_membrane_h

#define _USE_MATH_DEFINES
#include <math.h>

class SyrinxMembrane
{
protected:
    //time steps -- probably get this from Max later? look
    double SRATE = 44100;
    double T = 1.0/SRATE; //to make concurrent with Smyth paper
    
    //1.204 => float p; //air density (value used for 68F & atmospheric pressure 101.325 kPa (abs), 1.204 kg/m3
    //0.00118 => float p; //air density from Smyth diss., 0.00118cm, 0---- NOTE: Needs to match the input -- meters
    //1.18 => float p; //over m, air density from Smyt4 diss., 0.00118cm, 0---- NOTE: Needs to match the input -- meters
    //1.18 * Math.pow(10, -3) => float p; //air density from Smyth diss., 0.00118cm, 0---- NOTE: Needs to match the input -- meters
    double p = 0.001225; //air density from Smyth diss., 0.00118cm, 0---- NOTE: Needs to match the input -- meters
    
    //speed of sound (cm/s)
    double c = 34740; //m //speed of sound from Smyth, 34740 cm/s, so 347.4 m/s,
    
    double pG = 0.0; //pressure from the air sac ==> change to create sound, 0.3 or 300 used in Flectcher 1988, possibly 3 in cm units.
    
    
    //main values for differential equation, except for x, defined below in it's own block
    double p0 = 0.0 ; //brochial side pressure
    double p1 = 0.0 ; //tracheal side pressure -- **output that is the time-varying signal for audio
    double U = 0.0 ; //volume velocity flowing out
    
    //the rate of change that we find at each tick
    double dp0 = 0.0 ; //change in p0, bronchial pressure
    double dU = 0.0 ; //chage in U, volume velocity
    
    //NOTE: fix;
    std::vector<double> d2x = {0.0, 0.0}; //accel of the membrane per mode
    std::vector<double>  dx = {0.0, 0.0}; //velocity of the membrane per mode
    
    //initialized to prevent x going to 0
    std::vector<double> x = {0.0, 0.0};  //displacement of the membrane per mode
    double totalX = 0.0; // all the x's added for total displacement
    double F = 0.0; //force driving fundamental mode of membrane
    double x0 = 0.0; //equillibrium opening, in cm -- if this is 0.3, close to a -- it does oscillate, but incorrectly
    //if Force is not added when x is 0, then it will eventually stabilize into a high tone.
    
    //biological parameters *******
    double V = 1.0; //volume of the bronchius - in cm^3 //oh, yikes, I might change this for the dinosaur....
    //[150.0*2.0*pi, 250.0*2.0*pi] @=> float w[]; //radian freq of mode n, freq. of membranes: 1, 1.6, 2 & higher order modes are not needed Fletcher, Smyth
    
    //this one worked for a dino-like sound
    std::vector<double> wFreq = {200*2.0*M_PI*.75, 1.6*200*M_PI*.75}; //radian freq of mode n, freq. of membranes: 1, 1.6, 2 & higher order modes are not needed Fletcher, Smyth
    double initW = wFreq[0]; //can I do this in C++ I forget but prob. not
    
    
    //coefficients for d2x updates
    double q1 = 2.0;
    double k = wFreq[1]/(q1*2.0); //damping coeff. -- k = w1/2*Q1
    //300 => float k; //damping coeff. -- k = w1/2*Q1 //related to freq. & Q instead
    
    double E = 15; //a number of order 10 to 100 to represent stickiness
    double membraneNLCoeff = 10.0; //membrane non-linear coeff. for masses, etc.
    std::vector<double>  epsilonForceCouple = {2.0, 1.0}; //try -- mode 1 should be dominant.
    
    
    //3.5 => float a; //  1/2 diameter of trachea, in 3.5mm        ??area of membrane at a point
    double a = 0.35; //  1/2 diameter of trachea, in cm        ??area of membrane at a point
    
    // 3.5 => float h; //1/2 diameter of the syringeal membrane, 3.5mm, 0.35cm
    double h = 0.35; //1/2 diameter of the syringeal membrane, 3.5mm, 0.35cm
    
    //100 => float d; //thickness of the membrane, 100 micrometers, 1mm, 0.01cm -- leave in micrometers
    double d = 0.01; //thickness of the membrane, 100 micrometers, 1mm, 0.01cm -- leave in micrometers, 1mm, decreasing it to 0.01 shows same behavior as 3000pa
    
    double L = 7.0; //length of the trachea, in 7cm -- HOLD for this one
    
    //c => float pM; //material density of the syrinx membrane, from During, et. al, 2017, itself from mammalian not avian folds, kg/m
    //0.000102 => float pM; //material density in kg/m^3
    double pM = 1; //values from Smyth 1000.0 kg/m^3, but it should be g/cm3 to match the rest of the units, so 1 g/cm^3
    
    std::vector<double>  m = {0.0, 0.0} ; //the masses involved in vibration of each mode
    std::vector<double>  A3 = {0.3, 0.5} ; // constant in the mass equation -- changed bc there was an NAN after membrane density was put into
    //the correct units from kg/m^3 to g/cm^3
    
    //***** end biological parameters
    
    //tension
    //217.6247770440203 => float initT; //this is the tension to produce 150Hz, the example w in the research articles
    //Math.sqrt( (5*curT) / (pM*a*h*d) ) => w[0]
    double initT; //this is the tension to produce 150Hz, the example w in the research articles
    double curT; //tension to produce the default, in N/cm^3
    double goalT = 0.0;//the tension to increase or decrease to
    double dT = 0.0;
    double modT = 10.0; //how fast tension can change
    double diff = 0.0;
    
    //changing pG
    double goalPG  = 0.0 ;//the tension to increase or decrease to
    double dPG  =  0.0; //change in tension per sample
    double modPG = 100.0 ; //a modifier for how quickly dx gets added
    
    
    //impedences
    double z0;  //--small compared to trachea, trying this dummy value until more info
    double zG; //an arbitrary impedance smaller than z0, Fletcher
    
    double inP1 = 0.0; //to output for debugging
    double forceComponent = 0.0 ;
    double stiffness = 0.0;
    double moreDrag = 0.0;
    
    double testAngle = 0.0 ;
    
    double timeStep;
    
    //were local, now class variables to print them out
    double physConstants;
    double preshDiff;
    
public:
    SyrinxMembrane(double sr = 44100)
    {
        setSampleRate(sr);
        z0 = (p*c)/(M_PI*a*a); //impedence of bronchus
        zG = z0/12 ;
        timeStep = (T/2.0);
        initT = (wFreq[0]*wFreq[0]*pM*a*h*d)/5;
        curT = initT;
        A3 = {0.3, 0.5};
        modPG = 100.0;
        m = {0.0, 0.0};
        epsilonForceCouple = {2.0, 1.0};
        k = wFreq[1]/(q1*2.0);
        wFreq = {200*2.0*M_PI*.75, 1.6*200*M_PI*.75};
        initW = wFreq[0];
    }
    
    void setSampleRate(double sr)
    {
        SRATE = sr;
    }
    
    void set_a(double a_ = 4.5)
    {
        a = a_;
    }
    
    double get_a()
    {
        return a;
    }
    
    double get_h()
    {
        return h;
    }
    
    double get_L()
    {
        return L;
    }
    
    double get_T()
    {
        return T;
    }
    
    void set_h(double h_ = 4.5)
    {
        h = h_;
    }
    
    void set_L(double L_ = 116)
    {
        L = L_;
    }

    void set_d(double d_ = 5)
    {
        d = d_;
    }
    
    void set_modT(double modT_ = 10000.0)
    {
        modT = modT_;
    }
    
    void set_modPG(double modPG_ = 80)
    {
        modPG = modPG_;
    }
    
    //get the constant for the speed of sound that the membrane uses
    double getSoundSpeed()
    {
        return c; 
    }
    
    double get_z0()
    {
        return z0; 
    }
    
    double getTension()
    {
        return curT; 
    }
    
    double getPG()
    {
        return pG;
    }
    
    double getTotalX()
    {
        return totalX;
    }
    
  void initZ0()
 {
    z0 = (p*c)/(M_PI*a*a);
    //zG = z0/12; uncomment to fix
 }
    
           
 void initTension()
 {
     initT = (initW*initW*pM*a*h*d)/5;
     goalT = initT;
 }
      
 void updateForce()
 {
     //set the force -- the limit of the pU^2/7(ax^3)^(1/2) term is 0 when U is 0 -- this is the only way there would
     //be force with x=0 -- this is not clear in the texts.
     
     //need to consider the force -- at y=0, upstream (p1), F = 2ahp1
     //and downstream -- integrate pressure along y
     // p(y) = p0 + p/2(U/pi*a^2)^2 - (U/2az(y^2)^2)
     // z(y) = x + (a-x)(y/h)^2
     
     //totalX + x0 => float x;
     float x = totalX;
     
     if( x > 0.0 )
     {
        F = a*h*(p0 + p1) - (2.0*p*U*U*h)/(7.0*pow(a*x, 1.5)); //-- Smyth (just reduced from Fletcher)
        //memArea* ( ( pressureDiff ) - ( UFactor/overArea  ) ) => F; //fletcher
      }
      else F = 0.5*a*h*(p0 + p1); //not sure, but divided by 2 produced the results
      
  }
    
    double getPhysConstants(){ return physConstants; }
    double getPreshDiff(){ return  preshDiff;}  ;
    double getDPO() { return dp0; }
  
 void updateBrochialPressure()
 {
    physConstants = (p*c*c) / V  ;
    preshDiff = (pG - p0) / zG ;

     dp0 = timeStep*( dp0 + (physConstants * (preshDiff-U) )) ;
     
     p0 += dp0 ; //this is correct, below is incorrect, but keep this way for now to prevent blow-up --> note: not sure what this means anymore. there is nothing below now except for the end of the function.
 }
    
double getZG()
{
    return zG;
}
 
 void updateU()
 {
     //<<< "before U: " + "x: "  + totalX + " p0: " + p0 + " p1: " + p1 + " U: " + U + " dU " + dU >>>;
  
     if( totalX > 0.0 )
     {
         float C = p/(8.0*a*a*totalX*totalX); //from Fletcher
         float D = ( 2.0 *sqrt( a*totalX ) )/p; //actually inverse

         //from Fletcher and Smyth -- mas o menos
         dU = timeStep*( dU + ( D * ( p0-p1-( C*U*U ) ) ) ) ; //0 < x <= a
         
          //integrate
          U += dU ;
     }
     else
     {
         dU = 0.0 ;
         U = 0.0 ;
     }
     
     //is this a fudge, or is this real? I think it is real because there is no negative U physically
     if ( U < 0 )
     {
         U = 0.0 ;
     }


     //<<< "after U: " + "x: "  + totalX + " p0: " + p0 + " p1: " + p1 + " U: " + U + " dU " + dU >>>;


 }
 
 void updateMass()
 {
     for( int i=0; i < x.size() ; i++ )
     {
         double square = (x[i]-x0)/h;
         double squared = square*square ;
         
         m[i] = (A3[i]*pM*M_PI*a*h*d)/4 ;
         m[i] = m[i] * ( 1 + ( membraneNLCoeff*squared) ) ;
     }
 }
    
 void updateX()
 {
     totalX = 0.0;

     for( int i =0 ; i < x.size() ; i++ )
     {
         double modifiedK = k;
         if( x[i] <= 0 )
         {
             modifiedK = k*E;
         }

         //update d2x
         //epsilon is taken as unity
         forceComponent  = ( F*epsilonForceCouple[i] ) / m[i]  ;
         stiffness = ( - 2.0*modifiedK*dx[i]  ) ;
         moreDrag = (- wFreq[i]*wFreq[i]*(x[i]-x0) );
        
         double nextDx2 = forceComponent + stiffness + moreDrag ;
         d2x[i] = timeStep*(d2x[i] + nextDx2);
                     
         //update dx, integrate
         double dxPrev = dx[i] ;
         dx[i] += d2x[i];
         
         //update x, integrate again
         x[i] += timeStep*(dxPrev + dx[i]);
         totalX += x[i];

     }
     totalX += x0;
 }
 
 void updateP1()
 {
     p1 = U*z0;
 }
    
double getU()
{
     return U;
}
    
double getP0()
{
    return p0;
}

 double tick(double inSample)
 {
     if(isnan(inSample))
     {
         inSample = 0 ;
     }

     p1 = inSample ;
                      
     updateBrochialPressure();
     updateU();
    
     //update x & params needed for x
     updateForce();
     updateMass();
     updateX();

     updateP1();
     
     //user changing parameters
     updateTensionAndW(); //-->TODO: need to uncomment when ready to implement tension
     updatePG();
      

     return p1;
 
 }
 
 //changes tension thus, frequency
 void changeTension(double tens)
 {
     goalT = tens;
 }
 
 void updateTensionAndW()
 {
     double diff = goalT - curT;
     if(diff != 0)
     {
         //so, try this?
         dT = (dT + diff)*T*modT;
         curT = curT + dT ;
         
         wFreq[0] = sqrt( (5*curT) / (pM*a*h*d) ) ; //Smyth diss.
         // w[0]*1.6 => w[1]; //Fletcher1988
         wFreq[1] = wFreq[0]*1.6; //this is essentially the same thing, but nevertheless, as this is how I discovered frequencies that
         //better fit the dimensions
     }
 }
 
 //changes the air flow
 void changePG(double npG)
 {
     goalPG = npG ;
 }
 
 void updatePG()
 {
     double diff = goalPG - pG ;
     dPG = (dPG + diff)*T*modPG ;
     pG += dPG ;
 }

};



#endif /* syrinx_membrane_h */
