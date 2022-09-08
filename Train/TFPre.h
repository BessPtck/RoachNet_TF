#pragma once
#ifndef TFPRE_H
#define TFPRE_H

#ifndef TRAINBASE_H
#include "TrainBase.h"
#endif

/* class reads in the images setup for training and trains all the levels
 images are in dDump
 images associated with L1 have the syntax L1_###.tga, with L2 ... L2_###.tga etc...
 the files in dDump L1.txt, L2.txt ect.. are the code files
 the code files give for each image indicated by ### the following
 ### - image number, ###-id code, ### - col plate set number, ### - net number, [float]- net target value, 0 - requires rotation or 1 - do not rotate this last part is left off for background
 all of the above are seperated by commas
*/
class TFPre : public Base {
public:
protected:
};

#endif
