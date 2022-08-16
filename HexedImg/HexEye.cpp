#include "HexEye.h"

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
	lhxs[indx].x = loc.x0;
	lhxs[indx].y = loc.x1;
	lhxs[indx].thislink = indx;
	int cindx = indx;
	for (int i = 0; i < 6; i++) {
		int nebLev_i = getNebLevIndex(nebi, num_neb, i);
		int oplink = Math::loop(i + 3, 6);
		if (nebLev_i >= 0) {
			/*this neighbor is alredy generated*/
			lhxs[nebLev_i].web[oplink] = &(lhxs[cindx]);
			lhxs[cindx].web[i] = &(lhxs[nebLev_i]);
		}
		else {/*only fill if these have not previously been filled*/
			indx++;/*add new hex*/
			lhxs[indx].x = m_hexU[i].x0 * Rs + loc.x0;
			lhxs[indx].y = m_hexU[i].x1 * Rs + loc.x1;
			lhxs[indx].thislink = indx;

			lhxs[indx].web[oplink] = &(lhxs[cindx]);
			lhxs[cindx].web[i] = &(lhxs[indx]);
		}
	}
	weaveRound(lhxs[cindx]);
	for (int i = 0; i < 6; i++) {
		int nebLev_i = getNebLevIndex(nebi, num_neb, i);
		if (nebLev_i >= 0)
			weaveRound(lhxs[nebLev_i]);
	}
	return ECODE_OK;
}