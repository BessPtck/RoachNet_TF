#include "Luna.h"

unsigned char Luna::genLunaPatterns(s_Luna* lun) {
	/*since this hexEye has only one level, the indexes of the nodes hanging from the top node should be the same as their indexes in the bottom plate*/
	for (int i_rot = 0; i_rot < 6; i_rot++) {
		genHalfLunaPattern(i_rot, lun->net[i_rot]->getTop()->get(0));
	}
	/*create full white/black patterns*/
	float patWeight = LUNA_WSCALE / ((float)NUM_LUNA_FOOTS);
	s_nNode* top_white_moon = lun->net[6]->getTop()->get(0);
	s_nNode* top_black_moon = lun->net[7]->getTop()->get(0);
	for (int i_ft = 0; i_ft < NUM_LUNA_FOOTS; i_ft++) {
		top_white_moon->w[i_ft] = patWeight;
		top_black_moon->w[i_ft] = -patWeight;
	}
	top_black_moon->N = NUM_LUNA_FOOTS;
	top_white_moon->N = NUM_LUNA_FOOTS;

	return ECODE_OK;

}

void Luna::genHalfLunaPattern(int lunRot, s_nNode* topNd) {
	int arcLen = 3;
	float posFracWeight = 1.f / (3.5f);
	float negFracWeight = -0.5f;//-3.5f / 2.f;//-1.f / arcLen;
	for (int i = 0; i < NUM_LUNA_FOOTS; i++)
		topNd->w[i] = negFracWeight;
	for (int i = 0; i <= arcLen; i++) {
		int i_Nd = Math::loop(lunRot + i, 6);
		i_Nd++;/*center node is 0, arc nodes are 1->6*/
		topNd->w[i_Nd] = posFracWeight;
	}
	int i_line;
	i_line = 0;
	float lineWeight = 0.5f / 3.5f;
	topNd->w[i_line] = lineWeight;
	i_line = Math::loop(lunRot, 6);
	topNd->w[i_line+1] = lineWeight;
	i_line = Math::loop(lunRot + 3, 6);
	topNd->w[i_line+1] = lineWeight;

	for (int i = 0; i < NUM_LUNA_FOOTS; i++)
		topNd->w[i]= LUNA_WSCALE * (topNd->w[i]);
	topNd->N = NUM_LUNA_FOOTS;
}