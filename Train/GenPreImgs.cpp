#include "GenPreImgs.h"

unsigned char GenPreImgs::fillSmudgeKeys() {
	if (!readMasterKey())
		return ECODE_ABORT;
	m_smudgeKey = new s_preImgsSmudgeKey[m_masterKey.N];
	unsigned char err = ECODE_OK;
	for (int i = 0; i < m_masterKey.N; i++) {
		s_datLine dline;
		s_stampKey stamp_key;
		if (!importStampKey(dline, stamp_key)) {
			err = ECODE_ABORT;
			break;
		}

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
	return (int)celf(sigRotBak + numBakafterSmudge);
}
