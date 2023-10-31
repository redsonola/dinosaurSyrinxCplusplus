//
//  ScatteringJunction.h
//  min.syrinx_tilde
//
//  Created by courtney on 10/16/23.
//

#ifndef ScatteringJunction_h
#define ScatteringJunction_h

struct PostScatteringPressures {
    double b1;
    double b2;
    double trach;
};

class ScatteringJunction
{
protected:
    float z0;
    float tubeZ;
    float zSum;
    
public:
    ScatteringJunction()
    {
        z0 = 0;
        tubeZ = 0;
        zSum = 0;
        updateZ0(0);
    }
    

    void updateZ0(double impedence)
    {
        z0 = impedence;
        zSum = (1.0/z0 + 1.0/z0 + 1.0/z0);
        tubeZ = 1.0/z0;
    }
    
    
    PostScatteringPressures scatter( double bronch1, double bronch2, double trachea )
    {
        double add = bronch1*tubeZ + bronch2*tubeZ +  trachea*tubeZ;
        add *= 2;

        //assume they have the same tube radius for now
        double pJ = add / zSum;
        
        PostScatteringPressures pressureOut;
        pressureOut.b1 = pJ-bronch1;
        pressureOut.b2 = pJ-bronch2;
        pressureOut.trach = pJ-trachea;

        return pressureOut;
    }
};


#endif /* ScatteringJunction_h */
