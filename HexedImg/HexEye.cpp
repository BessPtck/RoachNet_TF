#include "HexEye.h"
unsigned char n_HexEye::imgRoot(s_HexEye* eye, s_HexBasePlate* pImg, long center_i) {
	/*assume that center_i is inside of hex map*/
	s_HexPlate* bottom_lev = eye->getBottom();

	int next_web_i = 3;
	s_Hex* eye_nd = bottom_lev->get(0);/* zero is the center of the plate node */
	s_Hex* plt_nd = pImg->get(center_i);
	bool fullRoot = true;
	do {
		eye_nd = n_HexPlate::connLineStackedPlates(eye_nd, plt_nd, next_web_i);
		if (eye_nd != NULL)
			next_web_i = n_HexPlate::turnCornerStackedPlates(&eye_nd, &plt_nd, 3, 0);/*this resets the eye_nd pointer and plt_nd pointer to the next line*/
		else {
			next_web_i = -2;
			break;
		}
	} while (next_web_i >= 0);
	if (next_web_i < -1)
		fullRoot = false;

	next_web_i = 0;
	eye_nd = bottom_lev->get(0);
	plt_nd = pImg->get(center_i);
	do {
		eye_nd = n_HexPlate::connLineStackedPlates(eye_nd, plt_nd, next_web_i);
		if (eye_nd != NULL)
			n_HexPlate::turnCornerStackedPlates(&eye_nd, &plt_nd, 0, 3);
		else {
			next_web_i = -2;
			break;
		}
	} while (next_web_i >= 0);

	if (next_web_i < -1 || !fullRoot)
		return ECODE_ABORT;
	return ECODE_OK;
}
bool n_HexEye::check_imgRoot(s_HexEye* eye, s_HexBasePlate* pImg) {
	if (eye == NULL || pImg==NULL)
		return false;
	if (eye->N < 1)
		return false;
	if (pImg->N < 1)
		return false;
	s_HexPlate* bottom_lev = eye->getBottom();
	float eye_RDiff = HEXEYE_RDIFFTOL * bottom_lev->Rhex;
	float eye_Rmax = eye_RDiff + bottom_lev->Rhex;
	float eye_Rmin = bottom_lev->Rhex - eye_RDiff;
	if (pImg->Rhex > eye_Rmax || pImg->Rhex < eye_Rmin)
		return false;
	return true;
}
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

unsigned char s_HexEye::init(int NumLev) {
	if (lev != NULL)
		return ECODE_ABORT;
	lev = new s_HexPlate * [NumLev];
	if (lev == NULL)
		return ECODE_FAIL;
	N_mem = NumLev;
	for (int ii = 0; ii < N_mem; ii++)
		lev[ii] = NULL;
	N = 0;
}
void s_HexEye::release() {
	if (lev != NULL) {
		delete[] lev;
	}
	lev = NULL;
	N = 0;
	N_mem = 0;
}
unsigned char HexEye::init(float r, int NLevels, int n_lowestNodePtrs) {
	/*figure out what largest R is r*2^N_level */

}

unsigned char HexEye::initEye(s_HexEye* neye) {
	/* m_R should already have been set along with m_N_levels and N_lowetNodePtrs*/
	if (neye == NULL)
		return ECODE_FAIL;
	if (Err(neye->init(m_N_levels)))
		return ECODE_FAIL;
	neye->width = m_imgWidth;
	neye->height = m_imgHeight;
	float R_lev = m_R;
	long N_hex = 0;
	for (int i = 0; i < m_N_levels; i++) {
		long N_hex_thisLevel = numHexInLevel(N_hex, i);
		if (neye->lev[neye->N] != NULL)
			return ECODE_FAIL;
		neye->lev[neye->N] = new s_HexPlate;
		if (neye->lev[neye->N] == NULL)
			return ECODE_FAIL;
		if (Err(neye->lev[neye->N]->init(N_hex_thisLevel)))/*this creates the s_Hex nodes and initializes them in the plate*/
			return ECODE_FAIL;
		neye->lev[neye->N]->initRs(R_lev);
		float levelDim = sizeOfLevel(i);
		neye->lev[neye->N]->height = levelDim;
		neye->lev[neye->N]->width = levelDim;
		(neye->N)++;
		R_lev /= 2.f;
		N_hex = N_hex_thisLevel;
	}
	return ECODE_OK;
}
void HexEye::releaseEye(s_HexEye* neye) {
	if (neye != NULL) {
		for (int i = 0; i < neye->N; i++) {
			if (neye->lev[i] != NULL) {
				neye->lev[i]->release();
				delete neye->lev[i];
				neye->lev[i] = NULL;
			}
		}
		neye->release();
	}
}
long HexEye::numHexInLevel(long N_prev, int N_level) {
	if (N_level < 1L)/*for level 0 there is only one hex*/
		return 1L;
	int N_prev_level = N_level - 1;
	if (N_prev_level < 1)
		return 7L;
	/*the total number of hexes along a side, including the end hexes*/
	float numFullSide = floorf(Math::power(2.f, N_prev_level));
	/*the total number of hexes excluding the endpoints along a side*/
	float numSide = numFullSide - 2.f;
	float numEnds = 6.f;
	float numMiddle = N_prev - (numEnds + numSide);
	/*the number of hexes that will be spawned per middle hex is 1+6*1/2 or 4 */
	float numLevelHexes = numMiddle * 4.f;
	/*the number of hexes that will be spawned by each edge hex is 1 + 4*1/2+2 or 5*/
	numLevelHexes += numSide * 5.f;
	/*the number of hexes that will be spawned by each end hex is 1 + 3*1/2+3 or 5.5 */
	numLevelHexes += numEnds * 5.5f;
	return (long)floorf(numLevelHexes);
}
float HexEye::sizeOfLevel(int N_level) {
	/* 1+x+x^2 +.. = x^(n+1)-1*/
	float sideEx = Math::powerXseries(0.5f, N_level);/* 1+(1/2)+(1/2)^2+..(1/2)^N_level */
	float mini_side = Math::power(0.5f,N_level);
	sideEx *= m_R * sqrt(3.f) / 2.f;
	mini_side *= m_R;
	return sqrtf(sideEx * sideEx + mini_side * mini_side);
}
unsigned char HexEye::genEye(s_HexEye* neye) {
	s_HexPlate** levels = neye->lev;
	s_Hex* nd = (s_Hex*)levels[0]->nodes[0];
	nd->x = 0.f;
	nd->y = 0.f;
	nd->thislink = 0;
	levels[0]->N = 1;
	float curRs = levels[0]->RShex;
	int numToGen = m_N_levels - 1;
	for (int i = 0; i < numToGen; i++) {
		curRs /= 2.f;
		int indx = 0;
		/*loop over all the hexes in the current level and generate the hexes for the next lower level*/
		for (int i_top = 0; i_top < levels[i]->N; i_top++) {
			nd = (s_Hex*)levels[i]->nodes[i_top];
			int sub_hex_start_indx = indx;
			s_2pt loc = { nd->x, nd->y };
			s_2pt_i nebi[HEXEYE_MAXNEBINDXS];
			int num_neb = collectNebIndexes(levels[i], i_top, nebi);
			genLowerPattern(levels[i + 1]->nodes, curRs, loc, nebi, num_neb, indx);/*index advances to final pattern filled but not beyond*/
			indx++;/*get ready to fill the next one*/
			nd->nodes[6] = (levels[i + 1]->nodes[sub_hex_start_indx]);
			for (int down_i = 0; down_i < 6; down_i++) {
				s_Hex* lo_center_nd = (s_Hex*)levels[i + 1]->nodes[sub_hex_start_indx];
				nd->nodes[down_i] = lo_center_nd->web[down_i];
			}
		}
		levels[i + 1]->N = indx;
	}
	return ECODE_OK;
}
int HexEye::collectNebIndexes(s_HexPlate* lev, int i_top, s_2pt_i neb[]) {
	int numi = 0;
	for (int i_webtop = 0; i_webtop < 6; i_webtop++) {
		s_Hex* TopNd = (s_Hex*)((s_Hex*)lev->nodes[i_top])->web[i_webtop];// (s_fNode*)topLevel.m_fhex[curTopIndex].web[i_webtop];
		if (TopNd != NULL) {
			int rev_webtopi = Math::loop(i_webtop + 3, 6);
			/*check if down links for this top hex have been generated*/
			if (TopNd->nodes[6] != NULL) {
				/*if the centerlink has been generated then so to have the web links,
				which means the shared hex is already filled*/
				neb[numi].x0 = i_webtop;//index direction from new top to half overlaped hex below
				neb[numi].x1 = TopNd->nodes[rev_webtopi]->thislink;//this is the index of the node in the bottom that is shared half in the new top
				numi++;
			}
		}
	}
	return numi;
}
void HexEye::genLowerPattern(s_Node* lev_nds[], float Rs, s_2pt& loc, s_2pt_i neb[], int num_neb, int& indx) {
	/*
	lhxs is this level of hexes,
	Rs is the current hex short radius
	loc is the center of this 7 pack pattern
	indx is the index of the current center in its level
	*/
	// lhxs=m_levels[m_N_levels].m_hexes;
	s_Hex* lev_node = (s_Hex*)lev_nds[indx];
	lev_node->x = loc.x0;
	lev_node->y = loc.x1;
	lev_node->thislink = indx;
	int cindx = indx;
	for (int i = 0; i < 6; i++) {
		int nebLev_i = getNebLevIndex(neb, num_neb, i);
		int oplink = Math::loop(i + 3, 6);
		if (nebLev_i >= 0) {
			/*this neighbor is alredy generated*/
			s_Hex* neb_lev_node = (s_Hex*)lev_nds[nebLev_i];
			neb_lev_node->web[oplink] = (s_Node*)lev_node;
			lev_node->web[i] = (s_Node*)neb_lev_node;
		}
		else {/*only fill if these have not previously been filled*/
			indx++;/*add new hex*/
			s_Hex* rot_lev_node = (s_Hex*)lev_nds[indx];
			rot_lev_node->x = m_hexU[i].x0 * Rs + loc.x0;
			rot_lev_node->y = m_hexU[i].x1 * Rs + loc.x1;
			rot_lev_node->thislink = indx;

			rot_lev_node->web[oplink] = (s_Node*)lev_node;
			lev_node->web[i] = (s_Node*)rot_lev_node;
		}
	}
	weaveRound(lev_node);
	for (int i = 0; i < 6; i++) {
		int nebLev_i = getNebLevIndex(neb, num_neb, i);
		if (nebLev_i >= 0) {
			s_Hex* neb_lev_node = (s_Hex*)lev_nds[nebLev_i];
			weaveRound(neb_lev_node);
		}
	}
	return;
}
int HexEye::getNebLevIndex(s_2pt_i neb[], int num_neb, int& web_i) {
	int indxFound = foundInAr(neb, num_neb, web_i);
	if (indxFound < 0)
		return -1;
	return neb[indxFound].x1;
}
void HexEye::weaveRound(s_Hex* nd) {
	for (int i = 0; i < 6; i++) {
		s_Hex* prev_nd = (s_Hex*)nd->web[Math::loop(i - 1, 6)];
		s_Hex* cur_nd = (s_Hex*)nd->web[i];
		s_Hex* next_nd = (s_Hex*)nd->web[Math::loop(i + 1, 6)];
		cur_nd->web[Math::loop(4 + i, 6)] = prev_nd;
		cur_nd->web[Math::loop(2 + i, 6)] = next_nd;
	}
	return;
}
int HexEye::foundInAr(s_2pt_i ar[], int n, int val) {
	for (int i = 0; i < n; i++) {
		if (ar[i].x0 == val)
			return i;
	}
	return -1;
}