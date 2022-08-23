#include "sNet.h"
unsigned char s_Net::init(int nLev) {
	if (nLev < 1)
		return ECODE_ABORT;
	lev = new s_nPlate * [nLev];
	if (lev == NULL)
		return ECODE_FAIL;
	N_mem = nLev;
	N = 0;
	eye = NULL;
	return ECODE_OK;
}
unsigned char s_Net::init(const s_Net& other) {
	unsigned char err = init(other.N_mem);
	if (err != ECODE_OK)
		return err;
	this->eye = other.eye;
	this->N = other.N;
	for (int ii = 0; ii < N; ii++) {
		s_nPlate* other_lev = other.lev[ii];
		if (other_lev != NULL) {
			this->lev[ii] = new s_nPlate;
			if ((this->lev[ii]) == NULL)
				return ECODE_FAIL;
			err = this->lev[ii]->init(other_lev);
			if (err != ECODE_OK)
				return err;
		}
	}
	/* fix intra-plate links */
	for (int ii = 0; ii < (N - 1); ii++) {
		s_Plate* topLev = this->lev[ii];
		s_Plate* lowLev = this->lev[ii + 1];
		n_Plate::fixStackedPlateLinks(topLev, lowLev);
	}
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
unsigned char sNet::initNet(s_Net* sn, s_HexEye* eye, int numPlates) {
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

	/*structure of bottom matches eye and top corresponds to highest center of eye
	  however the structure in between may not correspond exactly to the 
	  geometry of the eye */
	unsigned char err = sn->lev[(num_eye_lev - 1)]->init(eye->getBottom()->N, numPlates);
	if (Err(err))
		return err;
	(sn->N)++;
	/*connect bottom plate to eye*/
	for (int ii = 0; ii < eye->getBottom()->N; ii++) {
		s_nNode* net_node = sn->getBottom()->get(ii);
		s_Hex* eye_node = eye->getBottom()->get(ii);
		net_node->hex = eye_node;
	}
	/*connect top plate/node to eye */
	s_nNode* net_top_node = sn->lev[0]->get(0);
	s_Hex* eye_top_node = eye->lev[0]->get(0);
	net_top_node->hex = eye_top_node;

	sn->eye = eye;
	/*connect all layers in net to themselves*/
	return connDownNet(sn);
}
unsigned char sNet::initLuna(s_Net* sn, s_HexEye* eye, int numColPlates) {
	if (sn == NULL || eye == NULL)
		return ECODE_FAIL;
	int num_lev = eye->N;
	if (num_lev != NUM_LUNA_EYE_LEVELS)
		return ECODE_ABORT;
	int* numLevNodes = new int[num_lev + 1];
	if (numLevNodes == NULL)
		return ECODE_FAIL;
	for (int i = 0; i < num_lev; i++) {
		numLevNodes[i] = eye->lev[i]->N;
	}
	numLevNodes[num_lev] = numColPlates;
	unsigned char err = initNet(sn, num_lev, numLevNodes);
	if (Err(err))
		return err;
	/*assumes luna has 2 levels*/
	/*connect the top net to the bottom net*/
	s_Hex* luna_top_node = eye->lev[0]->get(0);
	s_nNode* net_top_node = sn->lev[0]->get(0);
	net_top_node->hex = luna_top_node;
	if (luna_top_node->N != (int)eye->lev[0]->N)
		return ECODE_FAIL;
	/*connect the bottom layer of the net directly to the hexes according to the hex indexes
	   so that the hex indexes match the net indexes*/
	for (long i_lower = 0; i_lower < eye->lev[0]->N; i_lower++) {
		s_Hex* luna_node = eye->getBottom()->get(i_lower);
		s_nNode* net_node = sn->getBottom()->get(i_lower);
		net_node->hex = luna_node;
		/*the hanging node will also point to the same bottom hex node*/
		net_node->nodes[0] = (s_Node*)luna_node;
	}
	/*connect the hanging nodes so that the order of the hanging nodes in the net
	  is the same as the order of the hanging nodes from the top level of the eye*/
	for (int hanging_i = 0; hanging_i < luna_top_node->N; hanging_i++) {
		s_Node* luna_node = luna_top_node->nodes[hanging_i];
		s_Node* net_node = sn->getBottom()->get(luna_node->thislink);
		net_top_node->nodes[hanging_i] = net_node;
	}
	sn->eye = eye;
	return ECODE_OK;
}
void sNet::releaseNet(s_Net* sn) {
	if(sn!=NULL)
		sn->release();/*if lev pointers are not null they are released here*/
}
unsigned char sNet::connDownNet(s_Net* sn) {
	int num_net_lev = sn->N;
	if (num_net_lev < 1)
		return ECODE_ABORT;
	for (int ii = 0; ii < (num_net_lev - 1); ii++) {
		int num_low_lev_nodes = (int)sn->lev[ii+1]->N;
		int num_lev_nodes = (int)sn->lev[ii]->N;
		for (int i_nd = 0; i_nd < num_lev_nodes; i_nd++) {
			s_nNode* nd = sn->lev[ii]->get(i_nd);
			int num_hanging = nd->getNmem();
			if (num_hanging != num_low_lev_nodes)
				return ECODE_FAIL;
			for (int i_hanging = 0; i_hanging < num_hanging; i_hanging++) {
				s_Node* hanging_nd = sn->lev[ii + 1]->getNd(i_hanging);
				nd->nodes[i_hanging] = hanging_nd;
			}
			nd->N = num_hanging;
		}
	}
	return ECODE_OK;
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