#pragma once
#ifndef COLOR_H
#define COLOR_H

#ifndef COLPLATE_H
#include "ColPlate.h"
#endif

class s_ColPlateLayer : public s_HexBasePlateLayer {
public:
	s_ColPlateLayer();
	~s_ColPlateLayer();
	inline s_ColPlate* get(int indx) { return (s_ColPlate*)p[indx]; }
};

class Col : public Base {
public:
	Col();
	~Col();

	unsigned char init(int nCols);
	void          release();

	unsigned char addCol(s_ColWheel* col);/*this col is copied object does not need to continue existance*/


	unsigned char spawn(s_HexBasePlate* hexedImg, s_ColPlateLayer* colPlates);/*this layer will own its plates
																			    colplates is a new object but one that is NOT init*/
	void          despawn(s_ColPlateLayer* colPlates);

	unsigned char samLayer(s_ColPlateLayer* colPlates, int plate_i[], int plate_N, s_HexBasePlateLayer* sLayer);/* assembles col plates with indexs as indicated by plate_i[] into the sLayer; 
																												and initializes the sLayer
																								      sLayer does not own its indexes*/
	void          desamLayer(s_HexBasePlateLayer* sLayer);/*sets the indexes back to null and releases the sLayer*/

	inline int    getNCols() { return m_N_Cols; }
protected:
	/*not owned*/

	/*owned*/
	ColPlate*  m_genCol;/*gen helper*/
	s_ColWheel* m_Cols;
	int         m_N_Cols;
	int         m_mem_Cols;/*number of pointer in mem*/

	/*helpers to spawn and despawn*/
	unsigned char initPlateLayer(s_ColPlateLayer* colPlates);/*this should be passeded a non-null but UNintialized colPlatelayer struct
														   this function should be called after all the color wheels have been added*/
	void          releasePlateLayer(s_ColPlateLayer* colPlates);
};

namespace n_Col {
	bool run(s_HexBasePlate* hexedImg, s_ColPlateLayer* colPlates, long plate_hex_index);/*plate_hex_index is index of hex on each plate not the index of the plate in the layer*/
	bool runPlate(s_HexBasePlate* hexedImg, s_ColPlateLayer* colPlates, long layer_index, long plate_hex_index);
}
#endif