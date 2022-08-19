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

	unsigned char init(int nLev);/*initializes the array does not own the levels */
	void          release();/*assumes that if lev ptr is not null it is owned*/

	inline s_nPlate* getBottom() { return lev[N - 1]; }

	s_nPlate** lev;
	int N;/*number of 'levels' or plates in this struct */
protected:
	int N_mem;
};
class sNet : public Base { /* class that generates the s_Net structs 'structure' net */
public:
	sNet();
	~sNet();

	unsigned char initNet(s_Net* sn, int nLev, int numLevNodes[]);/*num lev nodes has dim of nLev+1 the final is the number of hanging node pointers nLowerNodes */
	unsigned char initNetLuna(s_Net* sn, s_HexEye* eye);/*lowest level hannging connects to all of the eye hanging node,
													      lowest level eye num hexes * number of lowest hanging per eye hex */

	void releaseNet(s_Net* sn);
protected:
};
namespace n_Net {
	void platesRootL2(s_HexEye* eye, s_HexPlate* plates[], long center_i);/*assumes geometry of lowest layer of eye is the same as geometry of plates
																	  and that the plates all have the same geometry
																	  and the eye has only 2 levels */
	bool check_platesRootL2(s_HexEye* eye, s_HexPlate* plates[], int num_plates);/* check that the geometry is good here for a root*/
}
#endif