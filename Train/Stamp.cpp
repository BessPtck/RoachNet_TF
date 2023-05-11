#include "Stamp.h"

void n_stampsKey::clear(s_stampsKey& key) {
	key.r = 0.f;
	key.Dim = 0.f;
	key.maxDim = 0.f;

	key.N = 0;
	key.N_sig = 0;
	key.N_bak = 0;
	key.N_pre_sig = 0;
	key.N_sig_bak_rot = 0;
	key.min_sig_bak_rotang = 0.f;
	key.sig_bak_rotang_jitter = 0.f;
	key.N_bak_smudge = 0;
	key.N_sig_smudge = 0;
	
	key.y_sig_bak_rot = 0.f;
	key.smudge_sigma_divisor = 0.f;
}
unsigned char n_stampsKey::dumpToDatLine(const s_stampsKey& key, s_datLine& dl) {
	if(n_stampsKey::len>PARSETXT_MAXAR)
		return ECODE_FAIL;
	int n = 0;
	dl.v[n] = key.r;
	n++;
	dl.v[n] = key.Dim;
	n++;
	dl.v[n] = key.maxDim;
	n++;
	dl.v[n] = (float)key.N;
	n++;
	dl.v[n] = (float)key.N_sig;
	n++;
	dl.v[n] = (float)key.N_bak;
	n++;
	dl.v[n] = (float)key.N_pre_sig;
	n++;
	dl.v[n] = (float)key.N_sig_bak_rot;
	n++;
	dl.v[n] = key.min_sig_bak_rotang;
	n++;
	dl.v[n] = key.sig_bak_rotang_jitter;
	n++;
	dl.v[n] = (float)key.N_bak_smudge;
	n++;
	dl.v[n] = (float)key.N_sig_smudge;
	n++;
	dl.v[n] = key.y_sig_bak_rot;
	n++;
	dl.v[n] = key.smudge_sigma_divisor;
	n++;
	dl.n = n;
	return ECODE_OK;
}
int n_stampsKey::datLineToKey(const s_datLine& dl, s_stampsKey& key) {
	if (n_stampsKey::len > PARSETXT_MAXAR)
		return -1;
	int n=0;
	key.r = dl.v[n];
	n++;
	key.Dim = dl.v[n];
	n++;
	key.maxDim = dl.v[n];
	n++;
	key.N = (int)roundf(dl.v[n]);
	n++;
	key.N_sig = (int)roundf(dl.v[n]);
	n++;
	key.N_bak = (int)roundf(dl.v[n]);
	n++;
	key.N_pre_sig = (int)roundf(dl.v[n]);
	n++;
	key.N_sig_bak_rot = (int)roundf(dl.v[n]);
	n++;
	key.min_sig_bak_rotang = dl.v[n];
	n++;
	key.sig_bak_rotang_jitter = dl.v[n];
	n++;
	key.N_bak_smudge = (int)roundf(dl.v[n]);
	n++;
	key.N_sig_smudge = (int)roundf(dl.v[n]);
	n++;
	key.y_sig_bak_rot = dl.v[n];
	n++;
	key.smudge_sigma_divisor = dl.v[n];
	n++;
	return n;
}
void n_stampKey::clear(s_stampKey& key) {
	key.ID = -1;
	key.r = 0.f;
	key.Dim = 0.f;
	key.ang = 0.f;
	key.y = -1.f;

	key.maxDim = 0.f;
	key.preRot = 0.f;
	utilStruct::zero2pt(key.offset);
	key.train = 0;
}
void n_stampKey::copy(s_stampKey& key, const s_stampKey& orig) {
	key.ID = orig.ID;
	key.r = orig.r;
	key.Dim = orig.Dim;
	key.ang = orig.ang;
	key.y = orig.y;

	key.maxDim = orig.maxDim;
	key.preRot = orig.preRot;
	utilStruct::copy2pt(key.offset, orig.offset);
	key.train = orig.train;
}
unsigned char n_stampKey::dumpToDatLine(const s_stampKey& key, s_datLine& dl) {
	if (n_stampKey::len > PARSETXT_MAXAR)
		return ECODE_FAIL;
	int n = 0;
	dl.v[n] = (float)key.ID;
	n++;
	dl.v[n] = key.ang;
	n++;
	dl.v[n] = key.y;
	n++;
	dl.v[n] = key.preRot;
	n++;
	dl.v[n] = key.offset.x0;
	n++;
	dl.v[n] = key.offset.x1;
	n++;
	dl.v[n] = (float)key.train;
	n++;
	dl.n = n;
	return ECODE_OK;
}
int n_stampKey::datLineToKey(const s_datLine& dl, s_stampKey& key) {
	if (dl.n < n_stampKey::len)
		return -1;
	int n = 0;
	key.ID = (int)roundf(dl.v[n]);
	n++;
	key.ang = dl.v[n];
	n++;
	key.y = dl.v[n];
	n++;
	key.preRot = dl.v[n];
	n++;
	key.offset.x0 = dl.v[n];
	n++;
	key.offset.x1 = dl.v[n];
	n++;
	key.train = (int)roundf(dl.v[n]);
	n++;
	return n;
}
void n_rCornKey::clear(s_rCornKey& key) {
	n_stampKey::clear(key.key);
	key.R = 0.f;
	key.opening_ang = 0.f;
}
void n_rCornKey::copy(s_rCornKey& key, const s_rCornKey& orig) {
	n_stampKey::copy(key.key, orig.key);
	key.R = orig.R;
	key.opening_ang = orig.opening_ang;
}
unsigned char n_rCornKey::dumpToDatLine(const s_rCornKey& key, s_datLine& dl) {
	if (n_rCornKey::len+n_stampKey::len > PARSETXT_MAXAR)
		return ECODE_FAIL;
	if (Err(n_stampKey(key.key, dl)))
		return ECODE_FAIL;
	int n = dl.n;
	dl.v[n] = key.R;
	n++;
	dl.v[n] = key.opening_ang;
	n++;
	dl.n = n;
	return ECODE_OK;
}
int n_rCornKey::datLineToKey(const s_datLine& dl, s_rCornKey& key) {
	int n = n_stampKey(dl, key.key);
	if (n < 1)
		return -1;
	key.R = dl.v[n];
	n++;
	key.opening_ang = dl.v[n];
	n++;
	return n;
}
unsigned char Stamp::calcNumOfStamps() {
	if (m_numAngDiv <= 0.f)
		return ECODE_FAIL;
	m_DAng = 2.f * PI / m_numAngDiv;
	int m_n_ang = (int)floorf(m_numAngDiv);
	int m_n_circleRadii = (int)floorf(m_numCircleRadii);
	m_max_total_num_of_stamps = m_n_ang * m_n_circleRadii;
	/*for each ang/radi combo there is at most a solid stamp and then a series of closing angle stamps*/
	m_max_total_num_of_stamps *= (1 + m_NFinalOpeningAngs);
	/*if the stamps are stamped with holes then there are several stamps per each stamp shape*/
	m_max_total_num_of_stamps *= number_of_stamps_per_shape;
	m_max_total_num_of_stamps *= 2; /*for the inverse shapes*/
	return ECODE_OK;
}
unsigned char Stamp::initParse() {
	string parseInFileDir("dummy");
	string parseOutFileDir(STAMP_DIR);
	parseOutFileDir += "/";
	parseOutFileDir += STAMP_ROUNDCORN_DIR;
	parseOutFileDir += "/";
	parseOutFileDir += STAMP_KEY;
	m_parse = new ParseTxt;
	if (Err(m_parse->init(parseInFileDir, parseOutFileDir)))
		return ECODE_FAIL;
	return ECODE_OK;
}
void Stamp::releaseParse() {
	if (m_parse != NULL) {
		m_parse->release();
		delete m_parse;
	}
	m_parse = NULL;
}
unsigned char Stamp::initStampKeys() {
	if (m_Keys != NULL)
		return ECODE_FAIL;
	m_Keys = new s_rCornKey[m_max_total_num_of_stamps];
	for (int ii = 0; ii < m_max_total_num_of_stamps; ii++)
		n_rCornKey::clear(m_Keys[ii]);
	return ECODE_OK;
}
void Stamp::releaseStampKeys() {
	if (m_Keys != NULL) {
		delete[] m_Keys;
	}
	m_Keys = NULL;
}
unsigned char Stamp::stampRoundedCornerImgs() {
	s_2pt center = { 0.f, 0.f };
	float cur_ang = 0.f;
	/* m_DAng, m_n_ang, m_n_circleRadii are calculated by calcNumOfStamps*/

	for (int i_ang = 0; i_ang < m_n_ang; i_ang++) {
		float cur_circleRadius = m_minCircleRadius;
		for (int i_rad = 0; i_rad < m_n_circleRadii; i_rad++) {			
			if (!stampsSolid(center, cur_ang, cur_circleRadius, m_cornerOpeningAng))
				break;
			if (!stampFinalAngSpread(center, cur_ang, cur_circleRadius, m_cornerOpeningAng))
				break;
			cur_circleRadius *= m_radCmul;
		}
		if (cur_circleRadius > m_max_circle_radius)
			m_max_circle_radius = cur_circleRadius;
		cur_ang += m_DAng;
	}
	return ECODE_OK;
}
unsigned char Stamp::dumpStampKeys() {
	/*assumes that the output file is already set correctly for the general dump*/
	n_datLine* dlines= new n_datLine[m_stampN];
	for (int i_stamp = 0; i_stamp < m_stampN; i_stamp++) {
		n_datLine dl;
		if (Err(n_rCornKey::dumpToDatLine(m_Keys[i_stamp], dlines[i_stamp])))
			return ECODE_FAIL;
	}
	if (Err(m_parse->writeCSV(dlines, m_stampN)))
		return ECODE_FAIL;
	/*now write the master stamp(s)Key file*/
	string parseOutFileDir(STAMP_DUMPBASE_DIR);
	parseOutFileDir += "/";
	parseOutFileDir += STAMP_ROUNDCORN_DIR;
	parseOutFileDir += "/";
	parseOutFileDir += STAMP_MASTER_KEY;
	m_parse->setOutFile(parseOutFileDir);
	n_datLine mdl[2];
	if (Err(n_stampsKey::dumpToDatLine(m_masterStampsKey, mdl[0])))
		return ECODE_FAIL;
	if (Err(m_parse->writeCSV(mdl, 1)))
		return ECODE_FAIL;
	return ECODE_OK;
}
bool Stamp::stampsSolid(const s_2pt& center, float ang, float circle_scale, float opening_ang) {
	m_cosFalloff = false;
	m_linearFalloff = false;
	m_gaussFalloff = false;
	m_sharpFalloff = false;
	//if (hexMath::inHex(R, r_scale, hexU, smudge_center, center)) 
	stampImg(center, ang, circle_scale, opening_ang);
	return true;
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
		stampsSolid(center, ang, circle_scale, opening_ang);
	}
	return true;
}
bool Stamp::stampImg(const s_2pt& center, float ang, float circle_rad, float opening_ang) {
	setBasisFromAng(ang);
	setRoundedCorner(center, circle_rad, opening_ang);
	renderStampToImg(m_stampImgs[m_stampN]);
	m_Keys[m_stampN].key.ID = m_stampN;
	m_Keys[m_stampN].key.ang = ang;
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