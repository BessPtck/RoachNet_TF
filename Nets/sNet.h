#pragma once
#ifndef SNET_H
#define SNET_H

#ifndef HEXEYE_H
#include "../HexedImg/HexEye.h"
#endif

#define NUM_LUNA_PATTERNS 8
#define NUM_LUNA_EYE_LEVELS 2

class s_Net {
public:
	s_Net();
	~s_Net();

	unsigned char init(int nLev);/*initializes the array does not own the levels */
	unsigned char init(const s_Net& other);/*assumes that if the lev pointer is non-null the copy should own the lev*/
	void          release();/*assumes that if lev ptr is not null it is owned*/

	inline s_nPlate* getBottom() { return lev[N - 1]; }

	s_nPlate** lev;
	int N;/*number of 'levels' or plates in this struct */
	s_HexEye* eye;
protected:
	int N_mem;
};

class sNet : public Base { /* class that generates the s_Net structs 'structure' net */
public:
	sNet();
	~sNet();

	unsigned char initNet(s_Net* sn, int nLev, int numLevNodes[]);/*num lev nodes has dim of nLev+1 the final is the number of hanging node pointers nLowerNodes */
	unsigned char initNet(s_Net* sn, s_HexEye* eye, int numPlates=1);/*top and bottom have 1 to 1 correspondence to eye nodes
													  the hidden (middle) levels have each node connected (by hanging links)
													  to all the nodes on the next level down
													  lowest level eye num hexes = lowest num of net nodes 
													  hanging nodes from lowest net level correspond to number
													  of plates that the net will connect to */
	unsigned char initLuna(s_Net* sn, s_HexEye* eye, int numColPlates=1);/*initiates net but with each of the hanging on the bottom
													   set up to connect to the plates
													   the number of hanging for the lowest level equals the number
													   of luna plates */

	void releaseNet(s_Net* sn);
protected:
	unsigned char connDownNet(s_Net* sn);/*connects the levels in the s_Net to each of their lower levels
								           assumes that the number of hanging nodes in mem is the same as the number of nodes in the lower plate*/
};

//namespace n_Net {
//	void platesRootL2(s_HexEye* eye, s_HexPlate* plates[], long center_i);
	/*assumes geometry of lowest layer of eye is the same as geometry of plates
																	  and that the plates all have the same geometry
																	  and the eye has only 2 levels */
//	bool check_platesRootL2(s_HexEye* eye, s_HexPlate* plates[], int num_plates);
	/* check that the geometry is good here for a root*/
//}
#endif