#include "ColPlate.h"

unsigned char ColPlate::init() {
	;
}
void ColPlate::release() {
	;
}
unsigned char ColPlate::spawn(s_HexBasePlate* hexedImg, s_ColPlate* colPlate, const s_ColWheel& Col) {
	if (hexedImg == NULL || colPlate == NULL)
		return ECODE_ABORT;
	unsigned char err=colPlate->init(hexedImg);
	if (Err(err))
		return err;
	err = setDownLinks(hexedImg, colPlate);
	if (Err(err))
		return err;
	n_ColWheel::copy(colPlate->Col, Col);
	setColWheelUnitVectors(colPlate->Col);
	colPlate->Col.DhueRes = 2.f - colPlate->Col.Dhue;
	return ECODE_OK;
}
void ColPlate::despawn(s_ColPlate* colPlate) {
	if (colPlate == NULL)
		return;
	colPlate->release();
}
void ColPlate::resetCol(s_ColPlate* colPlate, const s_ColWheel& Col) {
	if (colPlate == NULL)
		return;
	n_ColWheel::resetCol(colPlate->Col, Col);
}
unsigned char ColPlate::setDownLinks(s_HexBasePlate* hexedImg, s_ColPlate* colPlate) {
	if (hexedImg->N != colPlate->N)
		return ECODE_ABORT;
	for (long ii = 0; ii < hexedImg->N; ii++) {
		s_Hex* botHex = hexedImg->get(ii);
		s_Hex* topHex = colPlate->get(ii);
		for (int i_link = 0; i_link < topHex->N; i_link++)
			topHex->nodes[i_link] = NULL;
		topHex->nodes[0] = (s_Node*)botHex;/* 0 is ok since all s_Hex's have 7 down nodes in mem*/
		topHex->N = 1;
	}
	return ECODE_OK;
}
void ColPlate::setColWheelUnitVectors(s_ColWheel& col) {
	/*asume Ur goes out along the x0 axis*/
	col.Ur.x0 = 1.f;
	col.Ur.x1 = 0.f;
	/*the other 2 vectors are 120 degrees or 2PI/3 rad away from this one*/
	col.Ug.x0 = -cosf(PI / 3.f);
	col.Ug.x1 = sinf(PI / 3.f);
	col.Ub.x0 = col.Ug.x0;
	col.Ub.x1 = -col.Ug.x1;
}

void n_ColWheel::clear(s_ColWheel& Col) {
	Col.pixMax = 255.f;
	Col.Dhue = 1.f;
	Col.DI = 1.f;
	Col.DSat = 1.f;
	Col.HueFadeV = 1.f;
	Col.I_target = 1.f;
	utilStruct::zero2pt(Col.Hue_target);
	Col.Sat_target = 1.f;
	Col.finalScaleFactor = 1.f;
	utilStruct::zero2pt(Col.Ur);
	utilStruct::zero2pt(Col.Ug);
	utilStruct::zero2pt(Col.Ub);
	Col.DhueRes = 0.f;
}
void n_ColWheel::resetCol(s_ColWheel& col, const s_ColWheel& orig) {
	col.Dhue = orig.Dhue;
	col.DI = orig.DI;
	col.DSat = orig.DSat;
	col.HueFadeV = orig.HueFadeV;
	col.I_target = orig.I_target;
	utilStruct::copy2pt(col.Hue_target, orig.Hue_target);
	col.Sat_target = orig.Sat_target;
	//col.finalScaleFactor = orig.finalScaleFactor;
	col.DhueRes = 2.f - col.Dhue;
}
void n_ColWheel::copy(s_ColWheel& Col, const s_ColWheel& orig) {
	Col.pixMax = orig.pixMax;
	Col.finalScaleFactor = orig.finalScaleFactor;
	utilStruct::copy2pt(Col.Ur, orig.Ur);
	utilStruct::copy2pt(Col.Ug, orig.Ug);
	utilStruct::copy2pt(Col.Ub, orig.Ub);
	resetCol(Col, orig);
}

bool n_ColPlate::run(s_HexBasePlate* hexedImg, s_ColPlate* colPlate, long plate_index) {
	/*hexedImg->N must equal colPlate->N*/
	s_Hex* imgHex = hexedImg->get(plate_index);
	float col_val = findColDistances(colPlate->Col, imgHex->rgb);
	s_Hex* colHex = colPlate->get(plate_index);
	colHex->o = col_val;
	return true;
}
float n_ColPlate::findColDistances(s_ColWheel& cw, float rgb[]) {
	s_2pt hueV = findColWheelHue(cw,rgb);
	float hueLen = vecMath::len(hueV);
	float hDist = findHueDistance(cw,hueV,hueLen);
	float IDist = findIDistance(cw,rgb);
	float SatDist = findSatDistance(cw,rgb,hueLen);
	float col_dist = (hDist * IDist * SatDist) - 0.5f;
	return Math::StepFunc(cw.finalScaleFactor * col_dist);
}
s_2pt n_ColPlate::findColWheelHue(s_ColWheel& cw, float rgb[]) {
	float r = rgb[0] / cw.pixMax;
	float g = rgb[1] / cw.pixMax;
	float b = rgb[2] / cw.pixMax;
	s_2pt Rv = { r * cw.Ur.x0, r * cw.Ur.x1 };
	s_2pt Gv = { g * cw.Ug.x0, g * cw.Ug.x1 };
	s_2pt Bv = { b * cw.Ub.x0, b * cw.Ub.x1 };
	s_2pt hueV = vecMath::add(Rv, Gv, Bv);
	return hueV;
}
float n_ColPlate::findHueDistance(s_ColWheel& cw, s_2pt& hueV, float hueLen) {
	if (cw.DhueRes <= 0.f)
		return 1.f;
	float fadeMult = 1.f;
	if (hueLen < cw.HueFadeV) {
		fadeMult = hueLen / cw.HueFadeV;
	}
	/*find rot dist in color wheel*/
	s_2pt hueU = { hueV.x0 / hueLen, hueV.x1 / hueLen };
	float hueDot = vecMath::dot(hueU, cw.Hue_target);/*this should range from 1 to -1*/
	float hueDiff = 1.f - hueDot;

	float hueRet = 0.f;
	if (hueDiff <= cw.Dhue) {
		float sepval = hueDiff / cw.Dhue;
		sepval = 1.f - sepval;
		hueRet = sepval * fadeMult;
	}
	else if (hueDiff > cw.Dhue) {
		float resid = 2.f - hueDiff;
		float sepval = resid / cw.DhueRes;
		sepval = 1.f - sepval;
		sepval = -sepval;
		hueRet = sepval * fadeMult;
	}
	return (hueRet >= 0.f) ? hueRet : 0.f;
}
float n_ColPlate::findIDistance(s_ColWheel& cw, float rgb[]) {
	float r = rgb[0] / cw.pixMax;
	float g = rgb[1] / cw.pixMax;
	float b = rgb[2] / cw.pixMax;
	float totI = (r + g + b) / 3.f;
	float Idist = fabsf(totI - cw.I_target);
	float norDist = Idist / cw.DI;
	norDist -= 1.f;
	norDist = -norDist;
	return (norDist >= 0.f) ? norDist : 0.f;
}
float n_ColPlate::findSatDistance(s_ColWheel& cw, float rgb[], float hueLen) {
	if (cw.DSat > 1.f)
		return 1.f;
	float maxVal = 0.f;
	for (int i = 0; i < 3; i++) {
		if (rgb[i] > maxVal)
			maxVal = rgb[i];
	}
	maxVal /= cw.pixMax;
	float satVal = (maxVal > 0.f) ? hueLen / maxVal : 0.f;
	float valDiff = fabsf(satVal - cw.Sat_target);
	float NormValDiff = (cw.DSat > 0.f) ? valDiff / cw.DSat : valDiff;
	float satDist = (1.f - NormValDiff);
	return (satDist >= 0.f) ? satDist : 0.f;
}