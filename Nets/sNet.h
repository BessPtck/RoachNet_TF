#pragma once
#ifndef SNET_H
#define SNET_H

#ifndef HEXEYE_H
#include "../HexedImg/HexEye.h"
#endif

#define NUM_LUNA_PATTERNS 8 /* 6 luna half patterns one for each corner hex, 2 full moons one black the other white */
#define NUM_LUNA_EYE_LEVELS 2
#define NUM_LUNA_FOOTS 7 /* number of hexes in the lower level of the luna */

class s_Net {
public:
	s_Net();
	~s_Net();

	unsigned char init(int nLev);/*initializes the array does not own the levels */
	unsigned char init(const s_Net& other);/*assumes that if the lev pointer is non-null the copy should own the lev*/
	void          release();/*assumes that if lev ptr is not null it is owned*/

	inline s_nPlate* getTop() { return lev[0]; }
	inline s_nPlate* getBottom() { return lev[N - 1]; }

	s_nPlate** lev;
	int N;/*number of 'levels' or plates in this struct */
	s_HexEye* eye;
protected:
	int N_mem;
};

class s_CNnets {/*cluster of CNNs that read from the same geometric location
				  the geometric location is where the eye is rooted
				  the nets each find a specific pattern they are programed for, the number of pats the nets find is the number of nets
				  each net typically has it's hanging nodes root on multiple plates*/
public:
	s_CNnets();
	~s_CNnets();
	virtual unsigned char init(int nNets);/*initializes the net array does not fill pointers or own anything*/
	virtual unsigned char init(const s_CNnets& other);/*assumes that if pointers to nets(but not eye) are non-null the objects are owned and should be copied to owned objects*/
	virtual void          release();/*assumes that if the net pointers are non-null they are owned by this object (does not assuem ownership of eye)*/
	s_Net** net;/*considered owned if non-null when released*/
	s_HexEye* eye;/*not considered owned by this object but may be owned in some iherited classes*/
	int N;/* number of nets actually pointing to something */

	s_nNode* trigger_node;/*this node is setup to link to all the lowest plates,
						   if the plates do not pass the threshold value
						   of this node this net cluster will not try to root the eye or seed 
						   considered owned if non-null when released*/
protected:
	int N_mem;
};
namespace n_CNnets {
	inline bool rootEye(s_CNnets& nets, s_HexBasePlate& basePlate, long plate_index);
	void rootOnPlates(s_CNnets& nets, s_HexBasePlateLayer& plates);/*assumes the eye has already been rooted
																     roots each hex of the lowest layer of the net
																	 matching the lowest layer of the eye only on those
																	 hexes corressponding to the rooting hex in each of the plates
																	 number of rooting/hanging nodes must equal the number of plates*/
	void rootNNet(s_CNnets& nets, s_HexBasePlateLayer* plates);/*similar to root on plates but  NNet takes all possible data as
															     input Xs therefore all nodes rooted on by any node(eye hex matched) in the bottom layer
																 of the nnet must be rooted on by all nodes in the bottom layer
																 number of hanging nodes is (total eye hexes in bottom layer)*(number of plates)
																 order of attachement of hanging nodes is determined by the index order in the bottom
																 level of the eye */
}
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