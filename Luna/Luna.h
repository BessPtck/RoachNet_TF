#pragma once
#ifndef LUNA_H
#define LUNA_H

#ifndef SNET_H
#include "../Nets/sNet.h"
#endif


#define LUNA_WSCALE 3.f//60.f for line finder//1.f

class s_Luna : public s_CNnets {
public:
	s_Luna() {	; }
	~s_Luna() { ; }

	unsigned char init();/*number of s_Nets will be the same as number of luna patterns */
	unsigned char init(const s_Luna& other);/*makes this s_Luna a copy of the other s_Luna this s_Luna owns its own objects*/
	void release();
	/*note s_luna owns its eye*/
protected:
};
namespace n_Luna {
	bool run(s_Luna* lun, s_HexBasePlateLayer& colPlates, s_HexBasePlateLayer& lunPlates, long plate_index);/*runs luna at the plate_index location */
	/*helpers to run*/
	inline bool rootEye(s_Luna* lun, s_HexBasePlate& basePlate, long plate_index)
	  {  return n_CNnets::rootEye(lun, basePlate, plate_index);	}/* roots the hex eye in the luna pattern on this location of the base plate
										                                    the base plate will have the exact same dimensions as all  of the col plates
																		    and the luna plates above
																		    returns true if rooted successfully*/

	//void check_run(s_Luna& lun, s_HexBasePlateLayer& colPlates, s_HexBasePlateLayer& lunPlates);

	inline void  rootOnPlates(s_Luna* lun, s_HexBasePlateLayer& colPlates) { return n_CNnets::rootOnPlates(lun, colPlates); }/*assumes that eye in luna has already been rooted*/
	/** helpers that run after rooting **/
	void runLunaPatterns(s_Luna* lun);/* set the value of the nets(the luna patterns), luna must already be fully rooted
									     at the end of this run each s_Net in the s_luna now has a o value corresponding 
										 to the current plate location */
	float runLunaPat_on_plate(s_Net* lunaNet, int plate_i);/*runs luna pattern for the selected plate*/
}
class Luna : public Base {
public:
	Luna();
	~Luna();

	unsigned char init(float r, int num_color_plates=1);
	void release();

	unsigned char spawn(s_Luna* lun);/*initializes the s_Luna and fills it with the luna patterns, the luna owns its own eye*/
	void          despawn(s_Luna* lun);
protected:
	/*owned modules used to initiate their respective structs*/
	sNet* m_NetMaster;
	HexEye* m_EyeMaster;
	/*                                                       */
	int m_num_color_plates;

	unsigned char connLunaInterLinks(s_Net* sn, s_HexEye* eye);/* sets up net with each of the hanging on the bottom
													   set up to connect to the plates
													   the number of hanging for the lowest level equals the number
													   of luna plates */

	unsigned char genLunaPatterns(s_Luna* lun);
	void genHalfLunaPattern(int lunRot, s_nNode* topNd);
};

#endif
