#pragma once
#ifndef LUNA_H
#define LUNA_H

#ifndef SNET_H
#include "../Nets/sNet.h"
#endif

class s_Luna {
public:
	s_Luna();
	~s_Luna();

	unsigned char init();/*number os s_Nets will be the same as number of luna patterns */
	unsigned char init(const s_Luna& other);/*makes this s_Luna a copy of the other s_Luna this s_Luna owns its own objects*/
	void release();

	/* owned */
	s_Net** nets;
	s_HexEye* eye;
	float lunas[NUM_LUNA_PATTERNS];
protected:
};
class Luna : public Base {
public:
	Luna();
	~Luna();

	unsigned char init();
	void release();

	unsigned char gen(s_Luna* lun);
protected:

	unsigned char genLuna(s_HexEye* eye, s_Net* net);

	unsigned char genNodeStructure();
	void genNodePattern();
};
namespace n_Luna {
	void run(s_Luna& lun, s_HexPlateLayer& colPlates, s_HexPlateLayer& lunPlates);
	void check_run(s_Luna& lun, s_HexPlateLayer& colPlates, s_HexPlateLayer& lunPlates);

	/*helpers to run*/
	
}
#endif
