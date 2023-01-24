#pragma once
#ifndef TRACKSRUN_H
#define TRACKSRUN_H

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
#ifndef GENPREIMGS_H
#include "GenPreImgs.h"
#endif

using namespace std;

#define TRACKSRUN_MAXNUMTRACKS 3
#define TRACKSRUN_MAXNUMCOLPLATES 3
#define TRACKSRUN_MAXNUMCOLSAMPLATES 9

/*Class runs all the tracks that are to be run on an image, the image can be replaced with an image of the same dimensions and the tracks run again*/

struct s_colTrack {
	/*array that pics out of the color plates the set of plates to be used as the base of the track*/
	int                  color_plate_indexes[TRACKSRUN_MAXNUMCOLSAMPLATES];
	int                  num_sam_plates;/*number of plates assembled together to for base for track*/

	/*data plate objects for this track*/
	s_HexBasePlateLayer* lunBasePlates;/*assembly of col plates(or later filter plates) that will be used as the base to run the luna's on */
	s_HexBasePlateLayer* lunPlates;/* currently there will be only one layer of luna plates*/
	s_HexBasePlateLayer* L1Plates;/* output of running 1st layer of mini net */
	s_HexBasePlateLayer* L1PoolPlates;/*max pooling of l1 reducing resolution by a factor of 2*/
	s_HexBasePlateLayer* L2Plates;

	/*data net type objects for this track*/
	s_Luna* lunaNets;
	s_CNnets* L1Nets;
	s_CNnets* L2Nets;
};

namespace n_colTrack {
	/******* utility ******/
	void clear(s_colTrack& t);

	/******* init helpers *****/
	unsigned char genNewDataPlateObjects(s_colTrack& t);
	unsigned char genDataNetObjects(s_colTrack& t);

	/******* release helpers ****/
	void delDataPlateObjects(s_colTrack& t);
	void delDataNetObjects(s_colTrack& t);

	/*******              *****/
}

class TracksRun : public Base {
public:
	TracksRun();
	~TracksRun();

	unsigned char init(Img* dim_img, 
		int num_L1_nets, 
		string& inFile_L1NNet, 
		int num_L2_nets, 
		string& inFile_L2NNet,
		int num_luna_col_plates);/* the dim_img image has the correct dimensions and col mode for all other images 
						       */
	unsigned char AddColPlate(s_ColWheel* cols);
	unsigned char AddTrack(int track_indexes[], int num_indexes);/*all tracks must have the same number of sam_plates
																 and the number of sam_plates must be equal to the number of
																 luna col plates, num_luna_col_plates
																   so that the luna does not require different number of color plates*/

	void release();


protected:
	int m_num_luna_col_plates;
	/*not owned*/
	Img* m_img;
	/*         */
	string m_file_L1NNet;
	string m_file_L2NNet;
	int    m_num_L1NNets;
	int    m_num_L2NNets;

	/*chain used to run the net for the sample train Xs*/
	/*data plate objects which are used as a base for all tracks */

	s_HexBasePlate* m_hexedImg;
	s_ColPlateLayer* m_colPlates;/*plates each with a range of colors isolated*/
	
	/*gen objects*/
	HexImg* m_genHexImg;
	Col* m_genCol;
	Luna* m_genLuna;
	NNet* m_genL1NNet;
	NNet* m_genL2NNet;

	/*track struct objects set up to run a particular track, each track has a specific collection of colors*/
	s_colTrack m_ts[TRACKSRUN_MAXNUMTRACKS];
	int    m_num_tracks;

	/*** some of the above may remain NULL only used by inherited ***/

   /******* init helpers *****/
	unsigned char genNewDataPlateObjects();
	unsigned char genGenObjects();

	unsigned char InitGenObjs();
	unsigned char SpawnInitGenObjs_Col();
	unsigned char SpawnInitGenObjs();
	unsigned char SpawnDataObjs(s_ColWheel cols[], int num_colwheels);
	unsigned char SpawnTrackRun(s_colTrack& trk);/* does the spawn of the net objects for the track*/
	/******* release helpers ****/
	void delDataPlateObjects();
	void delGenObjects();
	void releaseTrackRun(s_colTrack& trk);

	void releaseGenObjs();
	void deSpawnDataObjs();
	void deSpawnTrackRun(s_colTrack& trk);
	/*******              *****/

	/*******              ******/
	/*       run               */
	unsigned char runTrack(int i_track);
	/*******              ******/
};

#endif