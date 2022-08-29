#pragma once
#ifndef LUNA_H
#define LUNA_H

#ifndef SNET_H
#include "../Nets/sNet.h"
#endif


#define LUNA_WSCALE 3.f//60.f for line finder//1.f

class s_Luna : public s_CNnets {
public:
	s_Luna();
	~s_Luna();

	unsigned char init();/*number of s_Nets will be the same as number of luna patterns */
	unsigned char init(const s_Luna& other);/*makes this s_Luna a copy of the other s_Luna this s_Luna owns its own objects*/
	void release();
	/*note s_luna owns its eye*/
protected:
};
namespace n_Luna {
	bool rootEye(s_Luna& lun, s_HexBasePlate& basePlate, long plate_index);/*roots the hex eye in the luna pattern on this location of the base plate
										                                   the base plate will have the exact same dimensions as all  of the col plates
																		   and the luna plates above
																		   returns true if rooted successfully*/

	void run(s_Luna& lun, s_HexBasePlateLayer& colPlates, s_HexBasePlateLayer& lunPlates);/*runs luna at whatever location the heyEye in the luna is currently rooted*/
	//void check_run(s_Luna& lun, s_HexBasePlateLayer& colPlates, s_HexBasePlateLayer& lunPlates);

	/*helpers to run*/
	void  rootOnPlates(s_Luna& lun, s_HexBasePlateLayer& colPlates);
	float runLunaPat(s_Net* lunNet);/* returns the value of the net(the luna pattern), net must already be fully rooted */
	float runLunaPat_on_plate(s_Net* lunaNet, int plate_i);/*runs luna pattern for the selected plate*/
}
class Luna : public Base {
public:
	Luna();
	~Luna();

	unsigned char init();
	void release();

	unsigned char initLuna(s_Luna* lun);/*initializes the s_Luna and fills it with the luna patterns, the luna owns its own eye*/
	void          releaseLuna(s_Luna* lun);
protected:


	unsigned char genLunaPatterns(s_Luna* lun);
	void genHalfLunaPattern(int lunRot, s_nNode* topNd);
};

#endif
