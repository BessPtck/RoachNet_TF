#include "TracksRun.h"

unsigned char n_colTrack::genNewDataPlateObjects(s_colTrack& t) {
	t.lunBasePlates = new s_HexBasePlateLayer;
	t.lunPlates = new s_HexBasePlateLayer;
	t.L1Plates = new s_HexBasePlateLayer;
	t.L1PoolPlates = new s_HexBasePlateLayer;
	t.L2Plates = new s_HexBasePlateLayer;
	return ECODE_OK;
}
unsigned char n_colTrack::genDataNetObjects(s_colTrack& t) {
	t.lunaNets = new s_Luna;
	t.L1Nets = new s_CNnets;
	t.L2Nets = new s_CNnets;
	return ECODE_OK;
}

void n_colTrack::delDataNetObjects(s_colTrack& t) {
	if (t.lunaNets != NULL)
		delete t.lunaNets;
	t.lunaNets = NULL;
	if (t.L1Nets != NULL)
		delete t.L1Nets;
	t.L1Nets = NULL;
	if (t.L2Nets != NULL)
		delete t.L2Nets;
	t.L2Nets = NULL;
}
void n_colTrack::delDataPlateObjects(s_colTrack& t) {
	if (t.L2Plates != NULL)
		delete t.L2Plates;
	t.L2Plates = NULL;
	if (t.L1PoolPlates != NULL)
		delete t.L1PoolPlates;
	t.L1PoolPlates = NULL;
	if (t.L1Plates != NULL)
		delete t.L1Plates;
	t.L1Plates = NULL;
	if (t.lunPlates != NULL)
		delete t.lunPlates;
	t.lunPlates = NULL;
	if (t.lunBasePlates != NULL)
		delete t.lunBasePlates;
	t.lunBasePlates = NULL;
}

unsigned char TracksRun::genNewDataPlateObjects() {
	if (m_num_tracks >= TRACKSRUN_MAXNUMTRACKS)
		return ECODE_FAIL;
	m_hexedImg = new s_HexBasePlate;
	m_colPlates = new s_ColPlateLayer;
	for (int i_trk = 0; i_trk < m_num_tracks; i_trk++) {
		n_colTrack::genNewDataPlateObjects(m_ts[i_trk]);
	}
	return ECODE_OK;
}
unsigned char TracksRun::genGenObjects() {
	m_genHexImg = new HexImg;
	m_genCol = new Col;
	m_genLuna = new Luna;
	m_genL1NNet = new NNet;
	return ECODE_OK;
}
unsigned char TracksRun::InitGenObjs() {
	if (m_img == NULL)
		return ECODE_ABORT;
	if (m_genHexImg == NULL || m_genCol == NULL || m_genLuna == NULL || m_genL1NNet == NULL)
		return ECODE_ABORT;
	if (m_hexedImg == NULL)
		return ECODE_ABORT;
	if (Err(m_genHexImg->init(m_img, m_hexedImg)))/*initializes the hexedImg*/
		return ECODE_FAIL;
	if (Err(m_genCol->init(m_genHexImg, TRACKSRUN_MAXNUMCOLPLATES)))/*max number of color plates allowed in memory for the gen object*/
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char TracksRun::SpawnInitGenObjs() {
	float lowest_Rhex = m_genHexImg->getRhex();
	if (Err(m_genLuna->init(lowest_Rhex, Num_luna_col_plates)))/**/
		return ECODE_FAIL;
	if (Err(m_genL1NNet->init(lowest_Rhex, LUNA_NUM_LUNAS, m_num_L1NNets)))
		return ECODE_FAIL;
	float L2_Rhex = 2.f * lowest_Rhex;
	if (Err(m_genL2NNet->init(L2_Rhex, m_num_L1NNets, m_num_L2NNets)))
		return ECODE_FAIL;
	if (m_file_L1NNet.length() > 1)
		if (Err(m_genL1NNet->setFile(m_file_L1NNet)))
			return ECODE_FAIL;
	if (m_file_L2NNet.length() > 1)
		if (Err(m_genL2NNet->setFile(m_file_L2NNet)))
			return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char TracksRun::AddColPlate(s_ColWheel* cols) {
	if (Err(m_genCol->addCol(cols)))
		return ECODE_FAIL;
	return ECODE_OK;
}
unsigned char TracksRun::AddTrack(int track_indexes[], int num_indexes) {
	if (num_indexes > m_genCol->getNCols())
		return ECODE_ABORT;
	
}
unsigned char TracksRun::SpawnDataObjs(s_ColWheel cols[], int num_colwheels) {
	for(int i_cols=0; i_cols<num_colwheels; i_cols++)

	if (Err(m_genCol->spawn(m_hexedImg, m_colPlates)))
		return ECODE_FAIL;
	int col_plate_is[1];
	col_plate_is[0] = 0;
	if (Err(m_genCol->samLayer(m_colPlates, col_plate_is, 1, m_lunBasePlates_t1)))
		return ECODE_FAIL;
}