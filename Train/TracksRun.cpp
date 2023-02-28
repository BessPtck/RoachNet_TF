#include "TracksRun.h"

unsigned char n_colTrack::genNewDataPlateObjects(s_colTrack& t) {
	t.lunBasePlates = new s_HexBasePlateLayer;
	t.lunPlates = new s_HexBasePlateLayer;
	t.L1Plates = new s_HexBasePlateLayer;
	t.L1PoolPlates = new s_HexBasePlateLayer;
	t.L2Plates = new s_HexBasePlateLayer;
	return ECODE_OK;
}

unsigned char n_colTrack::AddTrackIndexes(int track_indexes[], int num_indexes, s_colTrack& t) {
	if (num_indexes > TRACKSRUN_MAXNUMCOLSAMPLATES)
		return ECODE_FAIL;
	for (int ii = 0; ii < num_indexes; ii++) {
		t.color_plate_indexes[ii] = track_indexes[ii];
	}
	t.num_sam_plates = num_indexes;
	return ECODE_OK;
}
bool n_colTrack::checkMaxTrackIndex(int max_index, s_colTrack& t) {
	for (int ii = 0; ii < t.num_sam_plates; ii++) {
		if (t.color_plate_indexes[ii] < 0)
			return false;
		if (t.color_plate_indexes[ii] > max_index)
			return false;
	}
	return true;
}

unsigned char n_colTrack::spawnTrack(s_colTrack& t, s_ColPlateLayer* colPlates, int num_L1_plates, int num_L2_plates) {
	if (colPlates == NULL)
		return ECODE_FAIL;
	if (colPlates->N < 1)
		return ECODE_FAIL;
	if (t.num_sam_plates < 1)
		return ECODE_FAIL;
	if (t.lunBasePlates == NULL || t.lunPlates == NULL || t.L1Plates == NULL || t.L1PoolPlates == NULL || t.L2Plates == NULL)
		return ECODE_FAIL;
	t.lunBasePlates->init(t.num_sam_plates);
	/*the sam plates do not own their plates just link*/
	for (int ii = 0; ii < t.num_sam_plates; ii++) {
		int colPlate_index = t.color_plate_indexes[ii];
		if (colPlate_index < colPlates->N && t.lunBasePlates->N<t.lunBasePlates->getNmem()) {
			t.lunBasePlates->p[t.lunBasePlates->N] = colPlates->p[colPlate_index];
			(t.lunBasePlates->N)++;
		}
	}
	unsigned char errc = ECODE_OK;
	/*create the luna plates that run off of all of the sam plates*/
	errc=t.lunPlates->init(LUNA_NUM_LUNAS);
	if (errc != ECODE_OK)
		return errc;
	for (int ii = 0; ii < LUNA_NUM_LUNAS; ii++) {
		t.lunPlates->p[ii] = (s_HexPlate*)new s_HexBasePlate;
		/*make the luna plate be a copy of the col plate, the down links will point to what the down links of the color plate pointed to*/
		errc|=((s_HexBasePlate*)t.lunPlates->p[ii])->init((s_HexBasePlate*)colPlates->p[0]);
	}
	if (errc != ECODE_OK)
		return ECODE_FAIL;
	/*L1 plates sit on top of the col plates*/
	errc=t.L1Plates->init(num_L1_plates);
	if (errc != ECODE_OK)
		return errc;
	for (int ii = 0; ii < num_L1_plates; ii++) {
		t.L1Plates->p[ii] = (s_HexPlate*)new s_HexBasePlate;
		errc|=((s_HexBasePlate*)t.L1Plates->p[ii])->init((s_HexBasePlate*)t.lunPlates->p[ii]);
	}
	if (errc != ECODE_OK)
		return ECODE_FAIL;
	errc = t.L1PoolPlates->init(num_L1_plates);
	if (errc != ECODE_OK)
		return errc;
	for (int ii = 0; ii < num_L1_plates; ii++) {
		t.L1PoolPlates->p[ii] = (s_HexPlate*)new s_HexBasePlate;
		errc != n_HexBasePlate::pool2init((s_HexBasePlate*)t.L1Plates->p[ii], (s_HexBasePlate*)t.L1PoolPlates->p[ii]);
	}
	if (errc != ECODE_OK)
		return ECODE_FAIL;
	errc = t.L2Plates->init(num_L2_plates);
	if (errc != ECODE_OK)
		return ECODE_FAIL;
	for (int ii = 0; ii < num_L2_plates; ii++) {
		t.L2Plates->p[ii] = new s_HexBasePlate;
		errc != ((s_HexBasePlate*)(t.L2Plates->p[ii]))->init((s_HexBasePlate*)t.L1PoolPlates->p[ii]);
	}
	if (errc != ECODE_OK)
		return ECODE_FAIL;
	return ECODE_OK;
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
void n_colTrack::despawnTrack(s_colTrack& t) {
	if (t.L2Plates != NULL) {
		int num_L2_plates = t.L2Plates->N;
		for (int ii = 0; ii < num_L2_plates; ii++) {
			if (t.L2Plates->p[ii] != NULL) {
				t.L2Plates->p[ii]->release();
				delete t.L2Plates->p[ii];
				t.L2Plates->p[ii] = NULL;
			}
		}
		t.L2Plates->release();
	}
	if (t.L1PoolPlates != NULL) {
		int num_L1_plates = t.L1PoolPlates->N;
		for (int ii = 0; ii < num_L1_plates; ii++) {
			if (t.L1PoolPlates->p[ii] != NULL) {
				t.L1PoolPlates->p[ii]->release();
				delete t.L1PoolPlates->p[ii];
				t.L1PoolPlates->p[ii] = NULL;
			}
		}
		t.L1PoolPlates->release();
	}
	if (t.L1Plates != NULL) {
		int num_L1_plates = t.L1Plates->N;
		for (int ii = 0; ii < num_L1_plates; ii++) {
			if (t.L1Plates->p[ii] != NULL) {
				t.L1Plates->p[ii]->release();
				delete t.L1Plates->p[ii];
				t.L1Plates->p[ii] = NULL;
			}
		}
		t.L1Plates->release();
	}
	if (t.lunPlates != NULL) {
		int num_lun_plates = t.lunPlates->N;
		for (int ii = 0; ii < num_lun_plates; ii++) {
			if (t.lunPlates->p[ii] != NULL) {
				t.lunPlates->p[ii]->release();
				delete t.lunPlates->p[ii];
				t.lunPlates->p[ii] = NULL;
			}
		}
		t.lunPlates->release();
	}
	if (t.lunBasePlates != NULL) {
		int num_lun_plates = t.lunBasePlates->N;
		/*lun base plates do NOT own their plates*/
		for (int ii = 0; ii < num_lun_plates; ii++) {
			t.lunBasePlates->p[ii] = NULL;/*make sure this is not an attempted release*/
		}
		t.lunBasePlates->release();
	}
}
s_TracksRun::s_TracksRun():m_img(NULL), m_hexedImg(NULL), m_colPlates(NULL), m_num_col_plates(0), m_num_tracks(0), m_num_luna_col_plates(0),
 m_num_L1_plates_per_lunaPlate(0), m_num_L2_plates_per_L1Plate(0)
{
	for (int ii = 0; ii < TRACKSRUN_MAXNUMTRACKS; ii++)
		n_colTrack::clear(m_ts[ii]);
}
unsigned char s_TracksRun::init(
	Img* dim_img,
	int num_col_plates,
	int num_tracks,
	int num_luna_col_plates,
	int num_L1_plates_per_lunaPlate,
	int num_L2_plates_per_L1plate
) {
	if (dim_img == NULL)
		return ECODE_FAIL;
	if (num_col_plates < 1 || num_tracks < 1 || num_luna_col_plates < 1)
		return ECODE_FAIL;
	if (num_luna_col_plates > num_col_plates)
		return ECODE_ABORT;
	m_img = dim_img;
	m_hexedImg = new s_HexBasePlate;/*generator takes an object that is there but not initialized*/
	m_colPlates = new s_ColPlateLayer;
	m_num_col_plates = num_col_plates;
	m_num_luna_col_plates = num_luna_col_plates;
	m_num_tracks = num_tracks;
	m_pre_set_tracks = 0;
	m_num_L1_plates_per_lunaPlate = num_L1_plates_per_lunaPlate;
	m_num_L2_plates_per_L1Plate = num_L2_plates_per_L1plate;
	for (int ii = 0; ii < num_tracks; ii++) {
		n_colTrack::genNewDataPlateObjects(m_ts[ii]);
	}
	genDataNetObjects();
	return ECODE_OK;
}
unsigned char s_TracksRun::AddTrackIndexes(int track_indexes[], int num_indexes) {
	if (num_indexes != m_num_luna_col_plates)
		return ECODE_ABORT;
	if (m_pre_set_tracks >= m_num_tracks)
		return ECODE_FAIL;
	unsigned char errc = n_colTrack::AddTrackIndexes(track_indexes, num_indexes, m_ts[m_pre_set_tracks]);
	if (errc != ECODE_OK)
		return errc;
	if (!n_colTrack::checkMaxTrackIndex(m_num_col_plates - 1, m_ts[m_pre_set_tracks]))
		return ECODE_FAIL;
	m_pre_set_tracks++;
	return errc;
}
unsigned char s_TracksRun::spawn(Col* genCol) {
	if (m_hexedImg == NULL || m_colPlates == NULL)
		return ECODE_FAIL;
	if (m_pre_set_tracks != m_num_tracks)
		return ECODE_ABORT;
	/*m_hexedImg is initialized when m_genHexImg was initialized*/
	if (Err(genCol->spawn(m_hexedImg, m_colPlates)))
		return ECODE_FAIL;
	for (int ii = 0; ii < m_num_tracks; ii++) {
		s_colTrack& t_track = m_ts[ii];
		if (t_track.num_sam_plates != m_num_luna_col_plates)
			return ECODE_FAIL;
		if (Err(n_colTrack::spawnTrack(m_ts[ii], m_colPlates, m_num_L1_plates_per_lunaPlate, m_num_L2_plates_per_L1Plate)))
			return ECODE_FAIL;
	}
	return ECODE_OK;
}
void s_TracksRun::release() {
	delDataNetObjects();
	for (int ii = 0; ii < m_num_tracks; ii++) {
		n_colTrack::delDataPlateObjects(m_ts[ii]);
	}
	if (m_colPlates != NULL)
		delete m_colPlates;
	m_colPlates = NULL;
	if (m_hexedImg != NULL)
		delete m_hexedImg;
	m_hexedImg = NULL;
	m_img = NULL;
	m_num_tracks = 0;
	m_num_L2_plates_per_L1Plate = 0;
	m_num_L1_plates_per_lunaPlate = 0;
	m_num_luna_col_plates = 0;
	m_num_col_plates = 0;
}
unsigned char s_TracksRun::genDataNetObjects() {
	m_lunaNets = new s_Luna;
	m_L1Nets = new s_CNnets;
	m_L2Nets = new s_CNnets;
	return ECODE_OK;
}
void s_TracksRun::delDataNetObjects() {
	if (m_lunaNets != NULL)
		delete m_lunaNets;
	m_lunaNets = NULL;
	if (m_L1Nets != NULL)
		delete m_L1Nets;
	m_L1Nets = NULL;
	if (m_L2Nets != NULL)
		delete m_L2Nets;
	m_L2Nets = NULL;
}
unsigned char s_TracksRun::update(Img* iimg) {
	if (iimg != NULL) {
		m_img = iimg;
	}
	else
		return ECODE_ABORT;
	return ECODE_OK;
}
unsigned char TracksRun::init(Img* dim_img,
	int num_L1_nets,
	string& inFile_L1NNet,
	int num_L2_nets,
	string& inFile_L2NNet,
	int num_col_plates,
	int num_tracks,
	int num_luna_col_plates
) 
{
	if (num_col_plates>TRACKSRUN_MAXNUMCOLPLATES || num_tracks > TRACKSRUN_MAXNUMTRACKS || num_luna_col_plates>TRACKSRUN_MAXNUMCOLSAMPLATES)
		return ECODE_ABORT;
	if(Err(m_dat.init(dim_img, num_col_plates, num_tracks, num_luna_col_plates, num_L1_nets, num_L2_nets)))
		return ECODE_FAIL;
	if (num_L1_nets < 1 || num_L2_nets < 1)
		return ECODE_ABORT;
	m_num_L1NNets = num_L1_nets;
	m_num_L2NNets = num_L2_nets;

	m_genHexImg = new HexImg;
	m_genCol = new Col;
	m_genLuna = new Luna;
	m_genL1NNet = new NNet;
	m_genL1NNet = new NNet;

	/*now start initializing the gen objects and using them to initialize the data objects*/
		/*initialize the dat object that can be setup before the colors and the tracks*/
	if (Err(m_genHexImg->init(m_dat.m_img, m_dat.m_hexedImg)))/*initializes the hexedImg*/
		return ECODE_FAIL;
	if (Err(m_genCol->init(m_genHexImg, m_dat.m_num_col_plates)))/*max number of color plates allowed in memory for the gen object*/
		return ECODE_FAIL;
	float lowest_Rhex = m_genHexImg->getRhex();
	if (Err(m_genLuna->init(lowest_Rhex, m_dat.m_num_luna_col_plates)))/**/
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
	return m_dat.AddTrackIndexes(track_indexes, num_indexes);
}

unsigned char TracksRun::spawn() {
	/*dat.m_hexedImg is initialized by init*/
	/*cols should already have been added*/
	if (m_genCol->getNCols() != m_dat.m_num_col_plates)
		return ECODE_ABORT;
	if (Err(m_dat.spawn(m_genCol)))/*this spawns the s_TracksRun object and all its tracks with all the data plates not filled*/
		return ECODE_FAIL;
	/*spawn the structures for the little nnets*/
	if (Err(m_genLuna->spawn(m_dat.m_lunaNets)))/*m_lunaNets is already created but not init*/
		return ECODE_FAIL;
	if (Err(m_genL1NNet->spawn(m_dat.m_L1Nets)))
		return ECODE_FAIL;
	if (Err(m_genL2NNet->spawn(m_dat.m_L2Nets)))
		return ECODE_FAIL;
	return ECODE_OK;
}

unsigned char TracksRun::run(Img* iimg) {
	m_dat.update(iimg);/*this just updates the image pointer in the s_TracksRun object*/
	m_genHexImg->update(iimg);
	/*genHexImg has uses the pointer to the m_hexedImg plate in the m_dat object therefor this object will update when genHexImg is updated*/
	for (long hex_i = 0; hex_i < m_dat.m_hexedImg->N; hex_i++) {
		
	}
}