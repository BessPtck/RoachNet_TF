#pragma once
#ifndef COLPLATE_H
#define COLPLATE_H
#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif

class s_ColPlate : public s_HexBasePlate {
public:
	s_ColPlate();
	~s_ColPlate();

	float Dhue;
	float DI;
	float DSat;
	float HueFadeV;
	float I_target;
	float Hue_target_x;
	float Hue_target_y;
	float Sat_target;
	float stepSteepness;
};

class ColPlate : public Base {
public:
	ColPlate();
	~ColPlate();

	unsigned char init();
	void          release();

	unsigned char spawn(
		s_ColPlate* colPlate,
		float Dhue = 3.f,
		float DI = 0.8f,//0.3f
		float DSat = 0.5f,//3f,//0.5f,
		float hueFadeV = 0.3f,
		float I_target = 1.0f,//0.9f,
		float hue_target_x = 1.f,
		float hue_target_y = 0.f,
		float Sat_target = 0.f,
		float stepSteepness = 7.f
	);
	void          despawn(s_ColPlate* colPlate);
protected:
	float m_Dhue;
	float m_DI;
	float m_DSat;
	float m_HueFadeV;
	float m_I_target;
	float m_Hue_target_x;
	float m_Hue_target_y;
	float m_Sat_target;
	float m_stepSteepness;
};

namespace n_ColPlate {
	bool run(s_HexBasePlate* hexedImg, s_ColPlate* colPlate, long plate_index);
}
#endif
