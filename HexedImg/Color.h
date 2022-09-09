#pragma once
#ifndef COLOR_H
#define COLOR_H

#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif


class s_ColPlate : public s_HexBasePlate {
public:
	s_ColPlate();
	~s_ColPlate();
protected:
};

class Col : public Base {
public:
	Col();
	~Col();

	unsigned char init();
	void          release();

	unsigned char spawn(s_ColPlate* colPlate);
	void          despawn(s_ColPlate* colPlate);
protected:
};

namespace n_Col {
	bool run(s_HexBasePlate* hexedImg, s_ColPlate* colPlate, long plate_index);
}
#endif