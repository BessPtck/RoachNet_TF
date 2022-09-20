#pragma once
#ifndef GENPREIMGS_H
#define GENPREIMGS_H
#ifndef STAMP_H
#include "Stamp.h"
#endif

using namespace std;

struct s_preImgsSmudgeKey {
	int ID;
	s_2pt* offset;
	s_2pt* ang_offset;
};

class GenPreImgs : public Base {
public:
	GenPreImgs();
	~GenPreImgs();

	unsigned char init(string& stamp_dir, bool doPreRot_this_pass); /* if true this is a pre-rot pass trying to only use the un-rotated signal, the angled signal will be ignored*/

	bool spawn(Img* stamp, s_stampKey& key);/*stamp image must be a non-null initiallized image
						                      stamp will be cleared */
protected:
	ParseTxt*    m_parse;
	CTargaImage* m_tga;
	string m_Dir;
	bool   m_preRot;/*true if training without signal that is not all aligned, uses only 1st signal set, aligned signal set, to train*/

	s_stampsKey         m_masterKey;
	s_stampKey*         m_stampKey;/*len will be masterKey.N*/
	int                 m_keyIndx;/*index of key currently being processed*/
	s_preImgsSmudgeKey* m_smudgeKey;
	int                 m_len_smudgeKey;
	int                 m_index_smudgeKey;/*current index in the m_smudgeKey array*/

	int                 m_N_total_sig;
	int                 m_N_total_bak;
	int                 m_N_smudge_bak;/*number of times to smudge the background for any given background*/
	int                 m_N_smudge_sig;/*number of times to smudge the signal for a given signal*/
	/* num smudge for the rotated signal to bak imgs is right now just 1 */

	unsigned char fillStampKeys();
	void          clearStampKeys();
	unsigned char statCalcSmudgeKeys();/*requires fillStampKeys to have been run first*/
	void          clearSmudgeKeys();/*reverses mem aloc from statCalcSmudgeKeys*/

	bool readMasterKey();/*reads the values into m_masterKey, check that signal is 1 or greater*/
	/*run after readMasterKey*/
	float sigRotBack();
	float bakAfterSmudge();
	int sigSmudge(float sigRotBak, float numBakafterSmudge);/*get signal smudge from masterKey*/
	int bakTotal(float sigRotBak, float numBakafterSmudge);

	bool importStampKey(const s_datLine& dline, s_stampKey& key);

	unsigned char processStampKey(const s_stampKey& key);
	/*helpers to processStampKey */
	unsigned char processSignalStampKey(const s_stampKey& key);
	unsigned char processRotSignalStampKey(const s_stampKey& key);/*setup smudge keys for a signal key that requires rotation*/
	unsigned char processBakStampKey(const s_stampKey& key);
};
#endif