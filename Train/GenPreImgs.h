#pragma once
#ifndef GENPREIMGS_H
#define GENPREIMGS_H
#ifndef STAMP_H
#include "Stamp.h"
#endif

using namespace std;

/*run the base that generates the raw images,
then run a screen test for the level to see if there is too much overlap, setting some of the train values to 0
then run this function generating the statistics necessary for the training set*/
class GenPreImgs : public Base {
public:
	GenPreImgs();
	~GenPreImgs();

	unsigned char init(string& stamp_dir, string& stampsKey_dir); /*these are the directories AFTER the base directory*/
	void release();

	bool spawn(Img* stamp, s_stampKey& key);/*stamp image must be a non-null initiallized image
						                      stamp will be cleared */
	void despawn(Img* stamp);

	inline int getN() { return m_len_smudgeStampKey; }
	inline int getNSig() { return m_N_total_sig; }
	inline int getNBak() { return m_N_total_bak; }
protected:
	ParseTxt*    m_parse;
	CTargaImage* m_tga;
	string m_Dir;
	string m_stampKey_Dir;


	s_stampsKey         m_masterKey;
	s_stampKey*         m_stampKey;
	int                 m_len_stampKey;
	s_stampKey*         m_smudgeStampKey;/* key including both the stampKey's and the RotBakKeys appropriately smudged
										      this is the key that is ready to use to generate the preTrain dataset*/
	int                 m_len_smudgeStampKey;
	int                 m_keyIndx;/*index of key currently being processed, is used at diffent stages of the program for different key sets*/

	int                 m_N_total_sig;
	int                 m_N_total_bak;
	int                 m_N_smudge_bak;/*number of times to smudge the background for any given background*/
	int                 m_N_smudge_sig;/*number of times to smudge the signal for a given signal*/
	int                 m_N_extra_smudge_sig;/*overflow of signal that needs to be added if sigal doesn't divide evenly into bak*/
	/* num smudge for the rotated signal to bak imgs is right now just 1 */

	/*gaussian integrals used to calculate different values*/
	s_gaussianInt       m_ang_jitter_I;
	s_gaussianInt       m_offset_I;

	unsigned char getStampKeys();/*reads in stampkeys and generates the m_stampKey array*/
	void          clearStampKeys();
	unsigned char statCalcSmudgeKeys();/*takes the stamps from Key and from BakFromSigRotKey and smears these stamps appropriately to generate
									     the keys int smudgeStampKey each of which generates a stamp to be used to generate the training data set*/
	void          clearSmudgeKeys();/*reverses mem aloc from statCalcSmudgeKeys*/

	/*helpers to getStampKeys*/
	bool readMasterKey();/*reads the values into m_masterKey, check that signal is 1 or greater*/
	bool getKeyFromLine(const s_datLine& dline, s_stampKey& key);
	bool addKey(const s_stampKey& key);/*checks against preRot whether key should be added*/
	/*helpers to statCalcSmudeKeys*/
	bool setupGaussIntegrals();
	void releaseGaussIntegrals();
	bool genSmudgedKeysFromKey(const s_stampKey& key, int N_smudge);

	/******************/
	/*image processing done after initialization an per spawn*/
	bool getImgFromTGA(Img* img, int ID);/*image will be the one indicated by the id, img points to a non initalized Img object*/
	void releaseImgFromTGA(Img* img);
};
#endif