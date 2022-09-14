#pragma once
#ifndef COLOR_H
#define COLOR_H

#ifndef COLPLATE_H
#include "ColPlate.h"
#endif

class s_ColPlateLayer : public s_HexPlateLayer {
public:
	s_ColPlateLayer();
	~s_ColPlateLayer();
	inline s_ColPlate* get(int indx) { return (s_ColPlate*)p[indx]; }
};

class Col : public Base {
public:
	Col();
	~Col();

	unsigned char init(HexImg* genHexImg, int nCols);
	void          release();

	unsigned char addCol(s_ColWheel* col);

	unsigned char spawn(s_HexBasePlate* hexedImg, s_ColPlateLayer* colPlates);/*this layer will own its plates*/
	void          despawn(s_ColPlateLayer* colPlates);

	unsigned char samLayer(s_ColPlateLayer* colPlates, int plate_i[], s_HexBasePlateLayer* sLayer);/* assembles col plates with indexs as indicated by plate_i[] into the sLayer; 
																								      sLayer does not own its indexes*/
	void          desamLayer(s_HexBasePlate* sLayer);/*sets the indexes back to null so no release is attempted*/
protected:
	/*not owned*/
	HexImg* m_genHexImg;

	/*owned*/
	ColPlate*  m_genCol;
	s_ColWheel* m_Cols;
	int        m_N_Cols;
	int        m_mem_Cols;/*number of pointer in mem*/
};

namespace n_Col {
	bool run(s_HexBasePlate* hexedImg, s_ColPlateLayer* colPlates, long plate_index);
}
#endif