#include "GenPreImgs.h"

unsigned char GenPreImgs::getStampKeys() {
	if (!readMasterKey())
		return ECODE_ABORT;
	/*read in the data*/
	s_datLine* dlines = new s_datLine[m_masterKey.N];
	int num_lines = m_parse->readCSV(dlines, m_masterKey.N);
	if (num_lines < 1)
		return ECODE_ABORT;
	/*now put the lines in the stampkeys*/
	int num_sig = m_preRot ? m_masterKey.N_pre_sig : m_masterKey.N_sig;
	m_len_stampKey = num_sig + m_masterKey.N_bak;
	m_stampKey = new s_stampKey[m_len_stampKey];
	int key_i = 0;
	s_stampKey curKey;
	for (int i = 0; i < num_lines; i++) {
		n_stampKey::clear(curKey);
		if (getKeyFromLine(dlines[i], curKey)) {
			if (addKey(curKey)) {
				n_stampKey::copy(m_stampKey[key_i], curKey);
				key_i++;
			}
		}
	}
	delete[] dlines;
	m_len_stampKey = key_i;
	if (key_i < 1)
		return ECODE_ABORT;
	return ECODE_OK;
}
void GenPreImgs::clearStampKeys() {
	if (m_stampKey != NULL) {
		delete[] m_stampKey;
	}
	m_stampKey = NULL;
	m_len_stampKey = 0;
}
unsigned char GenPreImgs::genBakFromSigRot() {
	m_keyIndx = 0;/*this will be used for m_stampBakFromSigRotKey*/
    /*figure out how many background from rotated signal there will be total*/
	int num_sig = m_preRot ? m_masterKey.N_pre_sig : m_masterKey.N_sig;
	int total_num_sig_bak_rot = m_masterKey.N_sig_bak_rot * num_sig;
	m_stampBakFromSigRotKey = new s_stampKey[total_num_sig_bak_rot];
	m_len_stampBakFromSigRotKey = total_num_sig_bak_rot;
	/*                                                                      */
	for (int i_stamps = 0; i_stamps < m_len_stampKey; i_stamps++) {
		if (m_stampKey[i_stamps].y > 0.f) {
			if (!genRotBakFromSigStamp(m_stampKey[i_stamps]))
				return ECODE_FAIL;
		}
	}
	m_len_stampBakFromSigRotKey = m_keyIndx;/*this should already be identical*/
	return ECODE_OK;
}
void GenPreImgs::clearBakFromSigRot() {
	if (m_stampBakFromSigRotKey != NULL) {
		delete[] m_stampBakFromSigRotKey;
	}
	m_stampBakFromSigRotKey = NULL;
	m_len_stampBakFromSigRotKey = 0;
}
unsigned char GenPreImgs::statCalcSmudgeKeys() {
	setupGaussIntegrals();
	int num_bak_pre_smudge = m_masterKey.N_bak + m_len_stampBakFromSigRotKey;
	m_N_total_bak = num_bak_pre_smudge * m_masterKey.N_bak_smudge;
	if (m_N_total_bak < 1)
		return ECODE_ABORT;
	m_N_smudge_bak = m_masterKey.N_bak_smudge;
	int num_sig_pre_smudge = m_preRot ? m_masterKey.N_pre_sig : m_masterKey.N_sig;
	if (m_masterKey.N_sig_smudge > 1) {
		num_sig_pre_smudge *= m_masterKey.N_sig_smudge;
		m_N_total_sig = num_sig_pre_smudge;
		m_N_smudge_sig = m_masterKey.N_sig_smudge;
		m_N_extra_smudge_sig = 0;
	}
	else  if (num_sig_pre_smudge < m_N_total_bak) {
		/*find how much sig must be smudged per background to make the sig and the background roughly equal*/
		float num_bak_per_sig = ((float)m_N_total_bak) / ((float)num_sig_pre_smudge);
		int num_bak_per_sig = (int)floorf(num_bak_per_sig);
		m_N_smudge_sig = num_bak_per_sig;
		m_N_extra_smudge_sig = m_N_total_bak - (m_N_smudge_sig * num_sig_pre_smudge);
		m_N_total_sig = m_N_total_bak;
	}
	m_len_smudgeStampKey = m_N_total_sig + m_N_total_bak;
	m_smudgeStampKey = new s_stampKey[m_len_smudgeStampKey];
	m_keyIndx = 0;
	/*smudge the signal*/
	s_stampKey* firstSigKey = NULL;
	for (int i_stamp = 0; i_stamp < m_len_stampKey; i_stamp++) {
		s_stampKey& key = m_stampKey[i_stamp];
		if (key.y > 0.f) {
			if (firstSigKey == NULL)
				firstSigKey = &(m_stampKey[i_stamp]);
			genSmudgedKeysFromKey(key, m_N_smudge_sig);
		}
	}
	if (firstSigKey == NULL)
		return ECODE_ABORT;
	genSmudgedKeysFromKey(*firstSigKey, m_N_extra_smudge_sig);
	/* now smudge the background */
	/*once for the reg back*/
	for (int i_stamp = 0; i_stamp < m_len_stampKey; i_stamp++) {
		s_stampKey& key = m_stampKey[i_stamp];
		if (key.y < 0.f)
			genSmudgedKeysFromKey(key, m_N_smudge_bak);
	}
	/*once for the sig rot bak*/
	for (int i_stamp = 0; i_stamp < m_len_stampKey; i_stamp++) {
		s_stampKey& key = m_stampBakFromSigRotKey[i_stamp];
		genSmudgedKeysFromKey(key, m_N_smudge_bak);
	}
	m_len_smudgeStampKey = m_keyIndx;
	return ECODE_OK;
}
void GenPreImgs::clearSmudgeKeys() {
	if (m_smudgeStampKey != NULL) {
		delete[] m_smudgeStampKey;
	}
	m_smudgeStampKey = NULL;
	releaseGaussIntegrals();
}
bool GenPreImgs::genRotBakFromSigStamp(s_stampKey& key) {
	int N_rot_div = m_masterKey.N_sig_bak_rot;
	N_rot_div -= 1;
	if (N_rot_div < 1)
		return false;
	float N_sig_bak_rot = (float)m_masterKey.N_sig_bak_rot;
	float ang_range = 2.f * PI - 2.f * m_masterKey.min_sig_bak_rotang;
	if (ang_range <= 0.f)
		return false;
	ang_range = Math::Ang2PI(ang_range);
	float dang = ang_range / ((float)N_rot_div);
	float ang = m_masterKey.min_sig_bak_rotang + key.preRot;

	for (int i_div = 0; i_div < m_masterKey.N_sig_bak_rot; i_div++) {
		float smudged_ang = smudgedRotBakFromSigAng(ang);
		ang = Math::Ang2PI(smudged_ang);
		n_stampKey::copy(m_stampBakFromSigRotKey[m_keyIndx], key);
		m_stampBakFromSigRotKey[m_keyIndx].preRot = ang;
		m_stampBakFromSigRotKey[m_keyIndx].y = m_masterKey.y_sig_bak_rot;
		m_keyIndx++;
		ang += dang;
	}
	return true;
}
bool GenPreImgs::setupGaussIntegrals() {
	if (m_masterKey.smudge_sigma_divisor <= 0.f)
		return false;
	float biggestAllowed_offset = m_masterKey.maxDim - m_masterKey.Dim;
	if (biggestAllowed_offset <= 0.f)
		return false;
	n_gaussianInt::init(m_offset_I, m_masterKey.r/m_masterKey.smudge_sigma_divisor,biggestAllowed_offset);

	n_gaussianInt::init(m_ang_jitter_I, m_masterKey.sig_bak_rotang_jitter / m_masterKey.smudge_sigma_divisor, PI);
	return true;
}
void GenPreImgs::releaseGaussIntegrals() {
	n_gaussianInt::release(m_offset_I);
	n_gaussianInt::release(m_ang_jitter_I);
}
bool GenPreImgs::genSmudgedKeysFromKey(const s_stampKey& key, int N_smudge) {
	if (N_smudge < 1)
		return false;
	/*generate the first identical key*/
	n_stampKey::copy(m_smudgeStampKey[m_keyIndx], key);
	m_keyIndx++;
	int Num = N_smudge - 1;
	for (int i = 0; i < Num; i++) {
		n_stampKey::copy(m_smudgeStampKey[m_keyIndx], key);
		s_2pt gaus_offset = Math::randGaus2D(key.offset, m_offset_I);
		utilStruct::copy2pt(m_smudgeStampKey[m_keyIndx].offset, gaus_offset);
		m_keyIndx++;
	}
	return true;
}

