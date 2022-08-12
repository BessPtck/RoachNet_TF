#pragma once
#ifndef HEXEYE_H
#define HEXEYE_H

#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif

class s_HexEye {
public:
	s_HexEye::s_HexEye();
	s_HexEye::s_HexEye();

	unsigned char init(int NumLev);
	void          release();

	inline s_HexPlate* get(int nLev) { return lev[nLev]; }
	inline void set(int nLev, s_HexPlate* pLev) { lev[nLev] = pLev; }
	inline bool append(s_HexPlate* pLev) { if (N < N_mem) { lev[N] = pLev; N++; return true; } return false; }

	inline float r() { if (N > 0) { return lev[N - 1]->Rhex; } return 0.f; }
	inline float rs() { if (N > 0) { return lev[N - 1]->RShex; } return 0.f; }
	inline float R() { if (N > 0) { return lev[0]->Rhex; } return 0.f; }
	inline float RS() { if (N > 0) { return lev[0]->RShex; } return 0.f; }
	inline s_2pt* hexU() { if (N > 0) { return lev[0]->hexU; } return NULL; }

	s_HexPlate** lev;
	int N;

	long width;
	long height;
protected:
	int N_mem;
};

namespace n_HexEye{
	void imgRoot(s_HexEye* eye, s_HexBasePlate* pImg, long center_i);/*roots on img, the hexEye should be one level above the img*/
	bool check_imgRoot(s_HexEye* eye, s_HexBasePlate* pImg);/*checks if the geometry is correct for the hex eye to root onthe img*/

	void platesRoot(s_HexEye* eye, s_HexPlate* plates[], long center_i);/*assumes geometry of lowest layer of eye is the same as geometry of plates
														                  and that the plates all have the same geometry */
	bool check_platesRoot(s_HexEye* eye, s_HexPlate* plates[]);/* check that the geometry is good here for a root*/
}

class HexEye : public Base {
public:
	HexEye();
	~HexEye();

	unsigned char init(float r, int NLevels, int N_lowestNodePtrs = 1);
	void          release();

	unsigned char spawn(s_HexEye* neye);/*spawn and put the results in neye*/
	void          despawn(s_HexEye* neye);/*cleans up eye*/
protected:
};
#endif
