#include "sNet.h"
unsigned char s_Net::init(int nLev) {
	lev = new s_nPlate * [nLev];
	if (lev == NULL)
		return ECODE_FAIL;
	N_mem = nLev;
	return ECODE_OK;
}
void s_Net::release() {
	if (lev != NULL) {
		for (int ii = 0; ii < N_mem; ii++) {
			if (lev[ii] != NULL) {
				lev[ii]->release();
				delete lev[ii];
			}
			lev[ii] = NULL;
		}
		delete[] lev;
	}
	lev = NULL;
	N_mem = 0;
	N = 0;
}
unsigned char sNet::initNet(s_Net* sn, int nLev, int numLevNodes[]) {
	if (sn == NULL)
		return ECODE_ABORT;
	if (Err(sn->init(nLev)))
		return ECODE_FAIL;
	for (int ii = 0; ii < nLev; ii++) {
		unsigned char err = sn->lev[ii]->init((long)numLevNodes[ii], numLevNodes[ii + 1]);
		if (Err(err))
			return err;
	}
	return ECODE_OK;
}
unsigned char sNet::initNetLuna(s_Net* sn, s_HexEye* eye) {
	if (sn == NULL || eye == NULL)
		return ECODE_ABORT;
	int num_eye_lev = eye->N;
	if (num_eye_lev < 1)
		return ECODE_ABORT;
	if (Err(sn->init(num_eye_lev)))
		return ECODE_FAIL;
	(sn->N) = 0;
	for (int ii = 0; ii < (num_eye_lev-1); ii++) {
		unsigned char err = sn->lev[ii]->init(eye->lev[ii], eye->lev[ii+1]->N);
		if (Err(err))
			return err;
		(sn->N)++;
	}

	unsigned char err = sn->lev[(num_eye_lev - 1)]->init(eye->getBottom()->N);
	if (Err(err))
		return err;
	(sn->N)++;
	for (int ii = 0; ii < eye->getBottom()->N; ii++) {
		s_nNode* net_node = sn->getBottom()->get(ii);
		s_Hex* eye_node = eye->getBottom()->get(ii);
		net_node->hex = eye_node;
	}

	return err;
}
/*
void n_HexEye::platesRootL2(s_HexEye* eye, s_HexPlate* plates[], long center_i)
{
	s_HexPlate* bottom_lev = eye->getBottom();
	int N = bottom_lev->nodes[0]->getNmem();
	s_Hex* eye_center_hex = bottom_lev->get(0);
	for (int i = 0; i < N; i++) {
		s_Hex* plate_center_node = plates[i]->get(center_i);
		eye_center_hex->nodes[i] = (s_Node*)plate_center_node;
		for (int i_web = 0; i_web < 6; i_web++) {
			s_Hex* eye_web_node = (s_Hex*)eye_center_hex->web[i_web];
			s_Node* plate_web_node = plate_center_node->web[i_web];
			eye_web_node->nodes[i] = plate_web_node;
		}
	}
	for (int i = 0; i < bottom_lev->N; i++) {
		if (bottom_lev->nodes[i]->nodes[0] != NULL)
			bottom_lev->nodes[i]->N = N;
		else
			bottom_lev->nodes[i]->N = 0;
	}
}
bool n_HexEye::check_platesRootL2(s_HexEye* eye, s_HexPlate* plates[], int num_plates) {
	if (eye == NULL || plates == NULL)
		return false;
	if (num_plates < 1)
		return false;
	if (eye->N != 2)
		return false;
	s_HexPlate* bottom_lev = eye->getBottom();
	if (bottom_lev == NULL)
		return false;
	if (bottom_lev->N < 1)
		return false;
	int N = bottom_lev->nodes[0]->getNmem();
	if (N != num_plates)
		return false;
	long size_plate = plates[0]->N;
	if (size_plate < 1)
		return false;
	for (int i_plate = 0; i_plate < N; i_plate++) {
		if (plates[i_plate] == NULL)
			return false;
		if (plates[i_plate]->N != size_plate)
			return false;
	}
	return true;
}
*/