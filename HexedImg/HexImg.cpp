#include "HexImg.h"

unsigned char HexImg::Init(Img* img,
	s_HexBasePlate* plate,
	float Rhex,
	float sigmaVsR,
	float IMaskRVsR) {

	m_p->initRs(Rhex);
	m_nWH = genMeshLoc(m_p->Rhex, m_p->RShex, m_p->Shex);
	long hex_size = (long)ceilf(2.f * m_nWH.x0 * m_nWH.x1);
	m_p->init(hex_size);
	//genMeshLocFromBR(m_nWH.x0, m_nWH.x1);
}

unsigned char HexImg::Update(Img* img) {
	if (img->getWidth() != m_img->getWidth() || img->getHeight() != m_img->getHeight())
		return ECODE_ABORT;
	m_img = img;
	return ECODE_OK;
}

unsigned char HexImg::genMesh() {
	if (m_p->N_wHex < 2)
		return ECODE_FAIL;
	/*rotate counterclockwise starting from full right pos x*/
	int index = 0;
	int rHigh_index = 0;
	for (int j = 1; j < m_p->N_hHex; j += 2) {
		for (int i = 0; i < (m_p->N_wHex - 1); i++) {
			index = j * m_p->N_wHex + i;
			s_Hex* hex = m_p->get(index);
			if (hex->i >= 0) {
				long target_index= (i < (m_p->N_wHex - 2)) ? index + 1 : -1;
				m_p->setWeb(index, 0, target_index);
				if (j >= 1) {
					m_p->setWeb(index, 1, index - m_p->N_wHex + 1);
					m_p->setWeb(index, 2, index - m_p->N_wHex);
				}
				target_index = (i > 0) ? index - 1 : -1;
				m_p->setWeb(index, 3, target_index);
				if (j <= (m_p->N_hHex - 2)) {
					m_p->setWeb(index, 4, index + m_p->N_wHex);
					m_p->setWeb(index, 5, hex->web[4]->thislink + 1);
				}
			}
		}
	}
	for (int j = 0; j < m_p->N_hHex; j += 2) {
		index = j * m_p->N_wHex;
		m_p->setWeb(index, 0, index + 1);
		if (j >= 1)
			m_p->setWeb(index, 1, index - m_p->N_wHex);
		if (j <= (m_p->N_hHex - 2))
			m_p->setWeb(index, 5, index + m_p->N_wHex);
		for (int i = 1; i < (m_p->N_wHex - 1); i++) {
			index = j * m_p->N_wHex + i;
			s_Hex* hex = m_p->get(index);
			m_p->setWeb(index, 0, index + 1);
			if (j >= 1) {
				m_p->setWeb(index, 1, index - m_p->N_wHex);
				m_p->setWeb(index, 2, hex->web[1]->thislink - 1);
			}
			m_p->setWeb(index, 3, index - 1);
			if (j <= (m_p->N_hHex - 2)) {
				m_p->setWeb(index, 5, index + m_p->N_wHex);
				m_p->setWeb(index, 4, hex->web[5]->thislink - 1);
			}
		}
		index = j * m_p->N_wHex + (m_p->N_wHex - 1);
		if (j >= 1)
			m_p->setWeb(index, 2, index - m_p->N_wHex - 1);
		m_p->setWeb(index, 3, index - 1);
		if (j <= (m_p->N_hHex - 2))
			m_p->setWeb(index, 4, index + m_p->N_wHex - 1);
	}
	return ECODE_OK;
}
s_2pt HexImg::genMeshLoc(float Rhex, float RShex, float Shex) {
	float width = (float)m_img->getWidth();
	float height = (float)m_img->getHeight();
	float segLen_y = Rhex + (Shex / 2.f);
	float segLen_x = RShex * 2.f;
	float nW = floorf(width / segLen_x);
	float nH = floorf(height / segLen_y);
	nH--;
	s_2pt nWnH = { -1.f, -1.f };
	if (nW < 1.f || nH < 1.f)
		return nWnH;
	float w_margin = width - nW * segLen_x;
	w_margin /= 2.f;
	float h_margin = height - nH * segLen_y;
	h_margin /= 2.f;

	m_BR.x0 = w_margin + RShex;
	m_BR.x1 = h_margin + Rhex;
	nWnH.x0 = nW;
	nWnH.x1 = nH;
	return nWnH;
}
unsigned char HexImg::genMeshWeb() {

}
unsigned char HexImg::genMeshLocFromBR(float nW, float nH) {
	float Rhex = m_p->Rhex;
	float RShex = m_p->RShex;
	float Shex = m_p->Shex;
	float segLen_y = Rhex + (Shex / 2.f);
	float segLen_x = RShex * 2.f;
	float w_margin = m_BR.x0;
	float h_margin = m_BR.x1;
	int n_w = (int)nW;
	int n_h = (int)ceilf(nH / 2.f);

	float x = w_margin;
	float y;
	for (int j = 0; j < n_h; j++) {
		y = h_margin + 2.f * segLen_y * (float)j;
		x = w_margin;
		for (int i = 0; i < n_w; i++) {
			int index = 2 * j * n_w + i;
			s_Hex* hex = m_p->get(index);
			hex->x = x;
			hex->y = y;
			hex->i = (long)roundf(x);
			hex->j = (long)roundf(y);
			hex->thislink = index;
			x += segLen_x;
		}
	}
	m_p->N = n_w * n_h;
	m_p->N_wHex = n_w;
	m_p->N_hHex = n_h;
	n_h = (int)floorf(nH / 2.f);
	int n_w_full = n_w;
	n_w -= 1;
	for (int j = 0; j < n_h; j++) {
		y = h_margin + segLen_y + 2.f * segLen_y * (float)j;
		x = w_margin + m_p->RShex;
		for (int i = 0; i < n_w; i++) {
			int index = (2 * j + 1) * n_w_full + i;
			s_Hex* hex = m_p->get(index);
			hex->x = x;
			hex->y = y;
			hex->i = (long)roundf(x);
			hex->j = (long)roundf(y);
			hex->thislink = index;
			x += segLen_x;
		}
	}
	m_p->N += n_w_full * n_h;
	m_p->N_hHex += n_h;
	return ECODE_OK;
}