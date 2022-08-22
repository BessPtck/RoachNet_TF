#pragma once
#ifndef LUNA_H
#define LUNA_H

#ifndef SNET_H
#include "../Nets/sNet.h"
#endif

class s_Luna {
public:
	s_Net** nets;
	s_HexEye* eye;
	float lunas[NUM_LUNA_PATTERNS];
protected:
};
class Luna : public Base {

};

#endif
