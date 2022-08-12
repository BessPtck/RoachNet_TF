#pragma once
#ifndef HEXIMG_H
#define HEXIMG_H
#ifndef CONVOLHEX_H
#include "ConvolHex.h"
#endif
/*class takes an image from a pointer that it does NOT OWN and uses it to first create a hex structure which is stored in 
  the hex base plate
  each pass is then used to update the rgb values of the hexes in the HexBasePlate from the current image values */
class HexImg : public Base {
public:
	HexImg();
	~HexImg();

	virtual unsigned char init(
		Img* img,
		s_HexBasePlate* plate,
		float Rhex = 9.f,//7.f,//3.f,//5.f, //3.f,  7 for debug of eye
		float sigmaVsR = 0.75f,
		float IMaskRVsR = 1.5f
	);

	virtual unsigned char update(Img* img);/*image must have same dimensions as original*/

	virtual unsigned char run();
	virtual void release();
	inline Img* getImg() { return m_img; }

	inline int getNHex() { return m_p->N; }
	inline s_Hex getHexCopy(long i) { return m_p->getCopy(i); }
	inline s_Hex* getHex(long i) { return m_p->get(i); }
	inline s_Node** getHexes() { return m_p->nodes; }

	inline float getRhex() { return m_p->Rhex; }
	inline float getShex() { return m_p->Shex; }
	inline float getRShex() { return m_p->RShex; }
	inline s_2pt* getHexUs() { return m_p->hexU; }

protected:
	/*not owned */
	Img* m_img;
	s_HexBasePlate* m_p;
	/* owned */
	ConvolHex* m_Convol;


	/*construction coord variables*/
	s_2pt   m_nWH;
	s_2pt   m_BR;/*lower right center start*/
	s_2pt_i m_hexMaskBL_offset;
	float   m_toppixHex;

	unsigned char genMesh();
	/***************************************************************/
	/* helpers to gen Mesh                                         */
	s_2pt genMeshLoc(float Rhex, float RShex, float Shex);
	unsigned char genMeshWeb();
	/** helpers to helpers **/
	unsigned char genMeshLocFromBR(float nW, float nH);
	/****                  ***/
	/***************************************************************/
	unsigned char genPlateRowStart();
	void          releasePlateRowStart();
};

#endif