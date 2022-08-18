#pragma once
#ifndef SNET_H
#define SNET_H

#ifndef HEXEYE_H
#include "../HexedImg/HexEye.h"
#endif

class s_Net {
public:
	s_Net();
	~s_Net();

	s_nPlate** lev;
	int N;/*number of 'levels' or plates in this struct */
protected:
	int N_mem;
};
class sNet : public Base { /* class that generates the s_Net structs 'structure' net */
public:
	sNet();
	~sNet();
protected:
};
namespace n_Net {
	void platesRootL2(s_HexEye* eye, s_HexPlate* plates[], long center_i);/*assumes geometry of lowest layer of eye is the same as geometry of plates
																	  and that the plates all have the same geometry
																	  and the eye has only 2 levels */
	bool check_platesRootL2(s_HexEye* eye, s_HexPlate* plates[], int num_plates);/* check that the geometry is good here for a root*/
}
#endif