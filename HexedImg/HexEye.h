#pragma once
#ifndef HEXEYE_H
#define HEXEYE_H

#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif

#define HEXEYE_MAXNEBINDXS 6 
#define HEXEYE_RDIFFTOL 0.0001f

class s_HexEye {
public:
	s_HexEye();
	s_HexEye();

	unsigned char init(int NumLev);
	unsigned char init(const s_HexEye& other);
	void          release();

	inline s_HexPlate* get(int nLev) { return lev[nLev]; }
	inline s_HexPlate* getBottom() { return lev[N - 1]; }
	inline void set(int nLev, s_HexPlate* pLev) { lev[nLev] = pLev; }
	inline bool append(s_HexPlate* pLev) { if (N < N_mem) { lev[N] = pLev; N++; return true; } return false; }

	inline float r() { if (N > 0) { return lev[N - 1]->Rhex; } return 0.f; }
	inline float rs() { if (N > 0) { return lev[N - 1]->RShex; } return 0.f; }
	inline float R() { if (N > 0) { return lev[0]->Rhex; } return 0.f; }
	inline float RS() { if (N > 0) { return lev[0]->RShex; } return 0.f; }
	inline s_2pt* hexU() { if (N > 0) { return lev[0]->hexU; } return NULL; }

	s_HexPlate** lev;/*s_HexPlates here are owned by the s_HexEye*/
	int N;

	long width;
	long height;
protected:
	int N_mem;
};

namespace n_HexEye{
	unsigned char imgRoot(s_HexEye* eye, s_HexBasePlate* pImg, long center_i);/*roots on img, the hexEye should be one level above the img*/
	bool check_imgRoot(s_HexEye* eye, s_HexBasePlate* pImg);/*checks if the geometry is correct for the hex eye to root onthe img*/
	unsigned char imgRootL2(s_HexEye* eye, s_HexBasePlate* pImg, long center_i);/*roots on the image plate, requires the eye to have only 2 levels*/
	bool check_imgRootL2(s_HexEye* eye, s_HexBasePlate* pImg);
}

class HexEye : public Base {
public:
	HexEye();
	~HexEye();

	unsigned char init(float r, int NLevels);
	void          release();

	unsigned char spawn(s_HexEye* neye);/*spawn and put the results in neye*/
	void          despawn(s_HexEye* neye);/*cleans up eye*/

	inline int getNLevels() { return m_N_levels; }
	inline long getNHexes(int i) { return m_N_hexes[i]; }
protected:
	/*dimensions of eyes this HexEye will initiate*/
	float m_r;
	float m_R;/*largest R*/
	int   m_N_levels;
	s_2pt m_hexU[6];
	long*  m_N_hexes;

	long m_imgWidth;
	long m_imgHeight;
	/*                                            */
	unsigned char genNumHexesPerLevel();
	/*                                            */
	unsigned char initEye(s_HexEye* neye);
	void          releaseEye(s_HexEye* neye);
	/*helpers to init Eye */
	long numHexInLevel(long N_prev, int N_level);
	float sizeOfLevel(int N_level);
	/*                    */

	unsigned char genEye(s_HexEye* neye);
	/*helpers to genEye              */
	int collectNebIndexes(s_HexPlate* lev, int i_top, s_2pt_i neb[]);
	void genLowerPattern(s_Node* lev_nds[], float Rs, s_2pt& loc, s_2pt_i neb[], int num_neb, int& indx);
	/*** helpers to genEye helpers ***/
	int getNebLevIndex(s_2pt_i neb[], int num_neb, int& web_i);
	void weaveRound(s_Hex* nd);
	int foundInAr(s_2pt_i ar[], int n, int val);
	/*                               */
};
#endif
