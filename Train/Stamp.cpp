#include "Stamp.h"

void n_stampKey::clear(s_stampKey& key) {
	key.ID = -1;
	key.r = 0.f;
	key.Dim = 0.f;
	key.ang = 0.f;
	key.y = -1.f;
	key.matchRot = false;
	key.smudge = false;
}
void n_rCornKey::clear(s_rCornKey& key) {
	n_stampKey::clear(key.key);
	key.R = 0.f;
	key.opening_ang = 0.f;
}
unsigned char Stamp::stampRoundedCornerImgs() {
	s_2pt center = { 0.f, 0.f };
	float DAng = 2.f * PI / m_numAngDiv;
	int n_ang = (int)floorf(m_numAngDiv);
	float cur_ang = 0.f;
	int n_circleRadii = (int)floorf(m_numCircleRadii);

	for (int i_ang = 0; i_ang < n_ang; i_ang++) {
		float cur_circleRadius = m_minCircleRadius;
		for (int i_rad = 0; i_rad < n_circleRadii; i_rad++) {			
			if (!stampsWHoles(center, cur_ang, cur_circleRadius, m_cornerOpeningAng))
				break;
			if (!stampFinalAngSpread(center, cur_ang, cur_circleRadius, m_cornerOpeningAng))
				break;
			cur_circleRadius *= m_radCmul;
		}
		if (cur_circleRadius > m_max_circle_radius)
			m_max_circle_radius = cur_circleRadius;
		cur_ang += DAng;
	}
	return ECODE_OK;
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
bool Stamp::stampFinalAngSpread(const s_2pt& center, float ang, float circle_scale, float opening_ang_start) {
	if (circle_scale > m_maxFinalOpeningAng)
		return true;
	float opening_ang_end = m_maxFinalOpeningAng;

	float dAng = (opening_ang_end - opening_ang_start) / ((float)m_NFinalOpeningAngs);
	for (int i = m_NFinalOpeningAngs - 1; i >= 0; i--) {
		float opening_ang = opening_ang_end - ((float)i) * dAng;
		stampsWHoles(center, ang, circle_scale, opening_ang);
	}
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

bool Stamp::isUnderLine(const s_2pt& pt, const s_2pt& Uline_perp) {
	s_2pt VtoPt = vecMath::v12(m_line_intersect, pt);
	return vecMath::dot(VtoPt, Uline_perp) >= 0;
}
bool Stamp::isInsideCurveHalf(const s_2pt& pt) {
	s_2pt VtoPt = vecMath::v12(m_circle_half_pt, pt);
	return vecMath::dot(VtoPt, m_UcenterIn) >= 0;
}

float Stamp::distFromRoundedCorner(const s_2pt& pt) {
	float dist = 0.f;
	if (isInsideCurveHalf(pt)) {
		dist = distFromClosestLine(pt);
	}
	else
		dist = distFromCircle(pt);
	return dist;
}
float Stamp::distFromClosestLine(const s_2pt& pt) {
	float dist1 = distFromLine(pt, m_Uline_perp1);
	float dist2 = distFromLine(pt, m_Uline_perp2);
	return (dist1 >= dist2) ? dist2 : dist1;
}

float Stamp::stampIntensity(const s_2pt& pt) {
	s_2pt convPt = vecMath::convBasis(m_UrevBasis0, m_UrevBasis1, pt);
	float intensity = RoundedCornerIntensityNoRot(convPt);
	if (!isInRoundedCornerNoRot(convPt))
		return stamp_zero_intensity;
	return intensity;
}

bool Stamp::isInRoundedCornerNoRot(const s_2pt& pt) {
	bool isInLines = isUnderLine(pt, m_Uline_perp1) && isUnderLine(pt, m_Uline_perp2);
	if (!isInLines)
		return false;
	if (isInsideCurveHalf(pt))
		return true;
	return isInCircle(pt);
}
float Stamp::RoundedCornerIntensityNoRot(const s_2pt& pt) {
	float dist = distFromRoundedCorner(pt);
	if (dist < m_thickness)
		return 1.f;
	float in_lin = 1.f;
	float in_cos = 1.f;
	float in_gau = 1.f;
	float in_line = 1.f;
	float scale = m_falloffScale;
	if (scale < 0.f)
		return 1.f;
	if (m_linearFalloff) {
		in_lin = 1.f - (dist - m_thickness) / scale;
		if (in_lin < 0.f)
			in_lin = 0.f;
		if (in_lin > 1.f)
			in_lin = 1.f;
	}
	if (m_cosFalloff) {
		float delta_dist = dist - m_thickness;
		if (delta_dist < 0.f)
			delta_dist = 0.f;
		float cmet = (PI / 2.f) * (delta_dist / scale);
		in_cos = cosf(cmet);
		if (in_cos < 0.f)
			in_cos = 0.f;
	}
	if (m_gaussFalloff) {
		float delta_dist = dist - m_thickness;
		if (delta_dist < 0.f)
			delta_dist = 0.f;
		float gmet = delta_dist / scale;
		in_gau = Math::GaussianOneMax(gmet, m_gaussSigma / scale);
	}
	if (m_sharpFalloff) {
		if (dist > m_thickness)
			in_line = 0.f;
	}
	return in_lin * in_cos * in_gau * in_line;
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