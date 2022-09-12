#include "Stamp.h"
bool Stamp::stampFinalAngSpread(const s_2pt& center, float ang, float circle_scale, float opening_ang_start) {

}
bool Stamp::stampsWHoles(const s_2pt& center, float ang, float circle_scale, float opening_ang) {
	m_cosFalloff = false;
	m_linearFalloff = false;
	m_gaussFalloff = false;
	m_sharpFalloff = false;
	//if (hexMath::inHex(R, r_scale, hexU, smudge_center, center)) 
	stampImg(center, ang, circle_scale, opening_ang);
	m_linearFalloff = true;
	stampImg(center, ang, circle_scale, opening_ang);
	m_linearFalloff = false;
	m_cosFalloff = true;
	stampImg(center, ang, circle_scale, opening_ang);
	m_cosFalloff = false;
	m_gaussFalloff = true;
	stampImg(center, ang, circle_scale, opening_ang);
	m_gaussFalloff = false;
	m_sharpFalloff = true;
	stampImg(center, ang, circle_scale, opening_ang);
	return true;
}
bool Stamp::stampImg(const s_2pt& center, float ang, float circle_rad, float opening_ang) {
	setBasisFromAng(ang);
	setRoundedCorner(center, circle_rad, opening_ang);
	renderStampToImg(m_stampImgs[m_stampN]);
	m_Keys[m_stampN].ID = m_stampN;
	m_Keys[m_stampN].ang = ang;
	m_Keys[m_stampN].R = circle_rad;
	m_Keys[m_stampN].opening_ang = opening_ang;
	m_stampN++;
	return true;
}
unsigned char Stamp::setBasisFromAng(float ang) {
	vecMath::setBasis(ang, m_UBasis0, m_UBasis1);
	vecMath::revBasis(m_UBasis0, m_UBasis1, m_UrevBasis0, m_UrevBasis1);
	return ECODE_OK;
}
unsigned char Stamp::setRoundedCorner(const s_2pt& center, float radius, float ang_rad) {
	if (ang_rad > stamp_max_ang_rad)
		return ECODE_ABORT;
	/*assumes that the m_UcenterIn points along the x direction*/
	m_circle_radius = radius;
	m_UcenterIn.x0 = -1.f;
	m_UcenterIn.x1 = 0.f;
	s_2pt offset = vecMath::mul(radius, m_UcenterIn);
	m_circle_center = vecMath::add(center, offset);
	float halfAng = ang_rad / 2.f;
	s_2pt l1 = { cosf(halfAng), sinf(halfAng) };
	if (l1.x0 <= 0.f)
		return ECODE_FAIL;
	s_2pt l2 = { l1.x0, -l1.x1 };
	s_2pt circle_half_pt = { radius * l1.x0, 0.f };/*l1.x0=l2.x0 this is the point on the center line between the enclosing line intercepts*/
	m_circle_half_pt = vecMath::add(circle_half_pt, offset);/*correct since coord system is centered at circle edge*/
	m_Uline_perp1.x0 = -l1.x0;/*these are the lines perp to the enclosing lines*/
	m_Uline_perp1.x1 = -l1.x1;
	m_Uline_perp2.x0 = -l2.x0;
	m_Uline_perp2.x1 = -l2.x1;
	float Ltanvec = radius * l1.x1 / l1.x0;
	float OutToPt = radius * l1.x0 + Ltanvec * l1.x1;
	m_line_intersect.x1 = center.x1;
	m_line_intersect.x0 = center.x0 + OutToPt - radius;
	return ECODE_OK;
}

float Stamp::stampIntensity(const s_2pt& pt) {
	s_2pt convPt = vecMath::convBasis(m_UrevBasis0, m_UrevBasis1, pt);
	float intensity = RoundedCornerIntensityNoRot(convPt);
	if (!isInRoundedCornerNoRot(convPt))
		return stamp_zero_intensity;
	return intensity;
}

unsigned char Stamp::renderStampToImg(Img* img) {
	s_rgb img_set_col = { 0xFF, 0xFF, 0xFF };
	const s_2pt startPt = { m_stampImgCenter.x0 - m_imgDim, m_stampImgCenter.x1 - m_imgDim };
	long WH = (long)ceilf(2.f * m_imgDim);
	s_2pt pt = { 0.f, 0.f };
	for (long j = 0; j < WH; j++) {
		for (long i = 0; i < WH; i++) {
			pt.x0 = startPt.x0 + (float)i;
			pt.x1 = startPt.x1 + (float)j;
			float intensity = stampIntensity(pt);
			s_rgb curCol = imgMath::mulIntensity(img_set_col, intensity);
			s_2pt_i img_pt = { 0,0 };
			if (stampCoordToImgCoord(img, pt, img_pt))
				img->SetRGB(img_pt.x0, img_pt.x1, curCol);
		}
	}
	return ECODE_OK;
}

bool Stamp::stampCoordToImgCoord(Img* img, const s_2pt& pt, s_2pt_i& img_pt) {
	img_pt.x0 = (long)floorf(pt.x0);
	img_pt.x1 = (long)floorf(pt.x1);
	img_pt.x0 += m_stampImgCenter.x0;
	img_pt.x1 += m_stampImgCenter.x1;
	if (img_pt.x0 < 0 || img_pt.x1 < 0)
		return false;
	if (img_pt.x1 >= img->getHeight() || img_pt.x0 >= img->getWidth())
		return false;
	return true;
}