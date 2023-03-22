#include "RenderBase.h"

unsigned char RenderBase::spawnHexPlateImg(s_HexBasePlate* plt, Img* iimg) {
	if (plt == NULL || iimg == NULL)
		return ECODE_FAIL;
	if (IsImgInit(iimg)) {
		if (!(IsImgDimMatch((s_HexPlate*)plt, iimg)))
			return ECODE_ABORT;
	}else
		if (Err(InitImg_for_HexPlate((s_HexPlate*)plt, iimg)))
			return ECODE_FAIL;
	return RenderHexPlate_to_Img((s_HexPlate*)plt, iimg);
}
bool RenderBase::IsImgInit(Img* iimg) {
	if (iimg == NULL)
		return false;
	unsigned char* internal_image_ptr = iimg->getImg();
	if (internal_image_ptr == NULL)
		return true;
	else return false;
}
bool RenderBase::IsImgDimMatch(s_HexPlate* plt, Img* iimg) {
	if (plt == NULL || iimg == NULL)
		return false;
	if (plt->width != iimg->getWidth())
		return false;
	if (plt->height != iimg->getHeight())
		return false;
	return true;
}
unsigned char RenderBase::InitImg_for_HexPlate(s_HexPlate* plt, Img* iimg) {
	return iimg->init(plt->width, plt->height, 3L);
}
unsigned char RenderBase::RenderHexPlate_to_Img(s_HexPlate* plt, Img* iimg) {

}
unsigned char RenderBase::InitMask_for_hex_dim(float r, float rs) {
	if (m_hex_mask == NULL)
		return ECODE_FAIL;
	if (m_hex_mask->getWidth() >= 1 || m_hex_mask->getHeight() >= 1)
		m_hex_mask->release();
	long longest_possible_dim = (long)ceil(r);
	longest_possible_dim += longest_possible_dim;
	longest_possible_dim += 2L;
	if(Err(m_hex_mask->init(longest_possible_dim, longest_possible_dim, 3L)))
		return ECODE_FAIL;
	m_hex_mask_center.x0 = longest_possible_dim + 1L;
	m_hex_mask_center.x1 = longest_possible_dim + 1L;
	m_hex_mask->clearToChar(0x00);
	return ECODE_OK;
}
unsigned char RenderBase::FillMask_for_U_of_HexPlate(s_HexPlate* plt) {
	s_rgb rgb_filled = { 0xff, 0xff, 0xff };
	for (long i_row = 0; m_hex_mask->getHeight(); i_row++) {
		for (long i_col = 0; m_hex_mask->getWidth(); i_col++) {
			long cur_index = i_row * m_hex_mask->getWidth() + i_col;
			s_2pt cur_loc = { (float)i_col, (float)i_row };
			s_2pt hex_mask_center = { (float)m_hex_mask_center.x0, (float)m_hex_mask_center.x1 };
			s_2pt pt = vecMath::v12(hex_mask_center, cur_loc);
			/*try rotating around the U's defined by the plate*/
			bool is_found_in_hex_slice = false;
			for (int u_i = 0; u_i < 5; u_i++) {
				is_found_in_hex_slice = do_FillBetweenUs(pt, plt->hexU[u_i], plt->hexU[u_i + 1], plt->RShex);
				if (is_found_in_hex_slice)
					break;
			}
			if (!is_found_in_hex_slice)
				is_found_in_hex_slice = do_FillBetweenUs(pt, plt->hexU[5], plt->hexU[0], plt->RShex);
			if (is_found_in_hex_slice)
				m_hex_mask->SetRGB(i_col, i_row, rgb_filled);
		}
	}
	return ECODE_OK;
}
bool RenderBase::do_FillBetweenUs(const s_2pt& pt, const s_2pt& U0, const s_2pt& U1, float rs) {
	/*find bisector*/
	s_2pt bi_long = vecMath::add(U0, U1);
	float len_bi = vecMath::len(bi_long);
	if (len_bi <= 0.f)
		return false;
	bi_long.x0 /= len_bi;
	bi_long.x1 /= len_bi;
	float cos_proj = vecMath::dot(bi_long, pt);
	if (cos_proj<0.f || cos_proj>rs)
		return false;
	float pt_len = vecMath::len(pt);
	if (pt_len < 0.f)
		return false;
	if (pt_len == 0.f)
		return true;
	float cos_norm = cos_proj/pt_len;
	float half_ang_cos = sqrtf(3.f) / 2.f;
	if (cos_norm < half_ang_cos)
		return false;
	return true;
}