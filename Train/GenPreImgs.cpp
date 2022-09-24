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
}
bool GenPreImgs::genRotBakFromSigStamp(s_stampKey& key) {

}

unsigned char GenPreImgs::fillStampKeys() {
	if (!readMasterKey())
		return ECODE_ABORT;
	unsigned char err = ECODE_OK;
	m_stampKey = new s_stampKey[m_masterKey.N];
	for (int i = 0; i < m_masterKey.N; i++) {
		s_datLine dline;
		if (!importStampKey(dline, m_stampKey[i])) {
			err = ECODE_ABORT;
			break;
		}
	}
	return ECODE_OK;
}
unsigned char GenPreImgs::statCalcSmudgeKeys() {
	float sigRotBak = sigRotBack();
	float numBakafterSmudge = bakAfterSmudge();
	m_N_total_sig = sigSmudge(sigRotBak, numBakafterSmudge);
	m_N_total_bak = bakTotal(sigRotBak, numBakafterSmudge);
	m_N_smudge_bak = m_masterKey.N_bak_smudge;
	m_N_smudge_sig = m_masterKey.N_sig_smudge;
	int m_len_smudgeKey = m_N_total_bak + m_N_total_sig;
	m_smudgeKey = new s_preImgsSmudgeKey[m_len_smudgeKey];
	for (int i_raw = 0; i_raw < m_masterKey.N; i_raw++) {
		s_stampKey& stamp_key = m_stampKey[i_raw];

	}

}
float GenPreImgs::sigRotBack() {
	float N_per_sig_rotBak = (float)m_masterKey.N_sig_bak_rot;
	float N_sigRotBak = m_preRot ? N_per_sig_rotBak * m_masterKey.N_pre_sig : N_per_sig_rotBak * m_masterKey.N_sig;
	return N_sigRotBak;
}
float GenPreImgs::bakAfterSmudge() {
	float N_per_smudge_Bak = (float)m_masterKey.N_bak_smudge;
	if (N_per_smudge_Bak < 1.f)
		return (float)m_masterKey.N_bak;
	float pre_smudge_N_bak = (float)m_masterKey.N_bak;
	float total_bak_after_smudge = pre_smudge_N_bak * N_per_smudge_Bak;
	return total_bak_after_smudge;
}
int GenPreImgs::sigSmudge(float sigRotBak, float numBakafterSmudge) {
	if (m_masterKey.N_sig_smudge > 0.f)
		return (float)m_masterKey.N_sig_smudge;
	float N_bak = numBakafterSmudge;
	N_bak += sigRotBak;
	float N_sig = m_preRot ? (float)m_masterKey.N_pre_sig : (float)m_masterKey.N_sig;
	float bak_to_sig = N_bak / N_sig;
	return (int)floorf(bak_to_sig);
}
int GenPreImgs::bakTotal(float sigRotBak, float numBakafterSmudge) {
	return (int)ceilf(sigRotBak + numBakafterSmudge);
}

unsigned char GenPreImgs::processStampKey(const s_stampKey & key) {
	if (key.y > 0.f) {
		if (key.matchRot < 0.0000001f)
			processSignalStampKey(key);
		else if (!m_preRot)
			processRotSignalStampKey(key);
	}
	else
		processBakStampKey(key);
	return ECODE_OK;
}
unsigned char GenPreImgs::processSignalStampKey(const s_stampKey& key) {

}