//
//  radian_bramp.h
//  
//
//  Created by Stephen Hibbs on 9/27/12.
//  Revised June 26 2014 
//

#ifndef _radian_bramp_h
#define _radian_bramp_h



#include <../RadianIncludes/RadianIncludes.h>

class Radian_bramp
{
public: 
    Radian_bramp(int dummy);
    void SetBrampSettings(byte* valuesIn, unsigned int intervalSeconds, unsigned long startShutterMs);
    unsigned long UpdateBulbSettings(unsigned long maxShutterT);
};
    
#endif
