#pragma once
#ifndef TRAINBASE_H
#define TRAINBASE_H

#ifndef HEXIMG_H
#include "../HexedImg/HexImg.h"
#endif
#ifndef COLOR_H
#include "../HexedImg/Color.h"
#endif
#ifndef LUNA_H
#include "../Luna/Luna.h"
#endif
#ifndef NNET_H
#include "../NNet/NNet.h"
#endif
#ifndef PARSETXT_H
#include "../FileIO/ParseTxt.h"
#endif
#ifndef CTARGAIMAGE_H
#include "../FileIO/CTargaImage.h"
#endif

#define TRAINBASE_OUTFILETAIL "_TFPre.csv"
#define TRAINBASE_STRUCTFILE "_NetStruct.txt"
#define TRAINBASE_TRAILVALNUM 2
using namespace std;

struct s_trainIO {
	Img* img;
	int imgNum;/*image number*/
	int col;/*color plate number*/
	int net;
	float target;
	bool rotate;
};
/* TrainBase is the common code that is used by all the TFpre train classes
   reads in a set of images numbered L#_### where L# is the level and ### is the image number
   reads in a txt file L#.txt which contains the codes instructing what to do with the images
   image number, image id code, col plate set number, net number, net target value, rotation flag 
   if col plate set number is 0 this means the col plate does not need to be run

   the structure of the nets is saved in the file L#_NetStruct.txt
   inputShape, LayerSize, Number of Layers including top but not hanging
   outputs are dumped into file L#_TFPre.csv
   net number, net target value, Xs...... , pre-set weights....., rot angle, id code
   */
class TrainBase : public Base {
public:
	TrainBase();
	~TrainBase();

	unsigned char init(
		string& L
	);
	void release();

	virtual unsigned char run();
protected:
	/*owned*/
	ParseTxt* m_parse;
	CTargaImage* m_tga;
	/*     */
	string m_L;

	s_trainIO* m_codes;
	long       m_N_codes;
	s_datLine* m_dump;
	long       m_N_dump;

	/*data for netStruct*/
	int        m_inputShape;
	int* m_LayerSize;
	int  m_Num_Layers;

	int        m_N_Xs;/*number of input values to train the nnet*/
	int        m_N_weights;/*number of pre weights to be dumped*/

	/***                                                       ***/
	/*chain used to run the net for the sample train Xs*/
	/*data plate objects */
	Img* m_img;
	s_HexBasePlate* m_hexedImg;
	s_HexBasePlateLayer* m_ColPlates;/* the pointer here point to s_ColPlate instead of HexBasePlate */
	s_HexBasePlateLayer* m_lunPlates;/* currently there will be only one layer of luna plates*/
	s_HexBasePlateLayer* m_L1Plates;/* output of running 1st layer of mini net */
	s_HexBasePlateLayer* m_L2Plates;

	/*data net type objects*/
	s_Luna* m_lunaNets;
	s_CNnets* m_L1Nets;
	s_CNnets* m_L2Nets;

	/*gen objects*/
	HexImg* m_genHexImg;
	Col* m_genCol;
	Luna* m_genLuna;
	NNet* m_genL1NNet;
	NNet* m_genL2NNet;
	/*** some of the above may remain NULL only used by inherited ***/
	/***                                                       ***/

	unsigned char readInCodes();
	unsigned char genImgs();
	void          releaseImgs();

	unsigned char dumpNetStruct();
	unsigned char dumpOutXs();
};

#endif
