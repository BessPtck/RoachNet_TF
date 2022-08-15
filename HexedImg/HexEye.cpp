#include "HexEye.h"

unsigned char HexEye::init(float r, int NLevels, int n_lowestNodePtrs) {
	/*figure out what larges R is r*2^N_level */
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

}