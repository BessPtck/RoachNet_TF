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
void n_ColWheel::clear(s_ColWheel& Col) {

}