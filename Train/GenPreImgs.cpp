#include "GenPreImgs.h"

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