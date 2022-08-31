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

	s_nPlate** lev;/*these may be owned*/
	int N;/*number of 'levels' or plates in this struct actually attached(or owned) */
	s_HexEye* eye;/*this is typically  not owned*/

	int hanging_plate_i;/*index of plate attached to the hanging nodes from the nodes in the bottom plate that has been selected as best 
						  signal for the net not always filled */
	float o;/*maximum output for this net*/
protected:
	int N_mem;/*number of lev pointers*/
};
namespace n_Net {
	void rootNNet(s_Net* net, s_HexPlate* eye_base, s_HexBasePlateLayer* plates);/*assumes that the eye_base is rooted in the correct spot*/
	void runRootedNNet(s_Net* net);
}
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
	inline bool rootEye(s_CNnets* nets, s_HexBasePlate& basePlate, long plate_index);
	void rootOnPlates(s_CNnets* nets, s_HexBasePlateLayer& plates);/*assumes the eye has already been rooted
																     roots each hex of the lowest layer of the net
																	 matching the lowest layer of the eye only on those
																	 hexes corressponding to the rooting hex in each of the plates
																	 number of rooting/hanging nodes must equal the number of plates*/
	void rootNNet(s_CNnets* nets, s_HexBasePlateLayer* plates);/*assumes the eye has already been rooted
															     similar to root on plates but  NNet takes all possible data as
															     input Xs therefore all nodes rooted on by any node(eye hex matched) in the bottom layer
																 of the nnet must be rooted on by all nodes in the bottom layer
																 number of hanging nodes is (total eye hexes in bottom layer)*(number of plates)
																 order of attachement of hanging nodes is determined by the index order in the bottom
																 level of the eye */
	bool runNNet(s_CNnets* nets, s_HexBasePlateLayer* plates, long plate_index);/*roots and runs the nnet at plate_index*/
}
class sNet : public Base { /* class that generates the s_Net structs 'structure' net */
public:
	sNet();
	~sNet();

	unsigned char init(int nLev, int numLevNodes[], int numHanging);/*initiates the sNet so tha it will generate a certain kind of s_Net struct
																	nLev is the number of levels in the s_Net
																	numLevelNodes is an array of len nLev giving the number of nodes in each level
																	numHanging is the number of open node pointers 'hanging' from each 
																	node in the bottom level */
	unsigned char init(HexEye* eye, int numPlates);/*initializes net to have same structure as eye
														   with number of hanging nodes equal to the number of plates*/
	void release();

	unsigned char spawn(s_Net* sn);
	unsigned char spawn(s_Net* sn, s_HexEye* eye);/*this eye should have exactly the same structure as the eye used to initialize the sNet
												  top and bottom have 1 to 1 correspondence to eye nodes
													  the hidden (middle) levels have each node connected (by hanging links)
													  to all the nodes on the next level down
													  lowest level eye num hexes = lowest num of net nodes
													  hanging nodes from lowest net level correspond to number
													  of plates that the net will connect to */
	void          despawn(s_Net* sn);

protected:
	int m_nLev;
	int* m_numLevNodes;/*num of nodes in each level + num of hanging nodes has length m_nLev+1*/
	int  m_numHanging;
	unsigned char connDownNet(s_Net* sn);/*connects the levels in the s_Net to each of their lower levels
								           assumes that the number of hanging nodes in mem is the same as the number of nodes in the lower plate*/
};


#endif