#pragma once
#ifndef TESTSTAMPS_H
#define TESTSTAMPS_H

#ifndef TRAINSTAMPS_H
#include "../Train/TrainStamps.h"
#endif

class TestStamps : public Base {
public:
	TestStamps();
	~TestStamps();


	void release();

	inline float getImgDim() { return m_imgDim; }
protected:
	/*owned*/
	Stamp* m_genStamps;

	float m_imgDim;/*dim of stamp image*/

	unsigned char setImgDim();
};
#endif
