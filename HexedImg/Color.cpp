#include "Color.h"

s_ColPlateLayer::s_ColPlateLayer() { ; }
s_ColPlateLayer::~s_ColPlateLayer() { ; }

unsigned char Col::init(HexImg* genHexImg, int nCols) {
	if (genHexImg == NULL)
		return ECODE_FAIL;
	m_genHexImg = genHexImg;
	m_genCol = new ColPlate;
	if (Err(m_genCol->init()))
		return ECODE_FAIL;
	if (nCols < 1)
		return ECODE_FAIL;
	m_mem_Cols = nCols;
	m_Cols = new s_ColWheel[m_mem_Cols];
	m_N_Cols = 0;
	return ECODE_OK;
}
void Col::release() {
	if (m_Cols != NULL) {
		delete[] m_Cols;
		m_Cols = NULL;
	}
	m_N_Cols = 0;
	m_mem_Cols = 0;
	if (m_genCol != NULL) {
		m_genCol->release();
		delete m_genCol;
		m_genCol = NULL;
	}
	m_genHexImg = NULL;
}
unsigned char Col::addCol(s_ColWheel* col) {
	if (col == NULL)
		return ECODE_ABORT;
	if (m_N_Cols >= m_mem_Cols)
		return ECODE_ABORT;
	n_ColWheel::copy(m_Cols[m_N_Cols], *col);
	m_N_Cols++;
	return ECODE_OK;
}
unsigned char Col::spawn(s_HexBasePlate* hexedImg, s_ColPlateLayer* colPlates) {
	unsigned char errc = ECODE_OK;
	errc = initPlateLayer(colPlates);
	if (Err(errc))
		return errc;
	for (int i = 0; i < m_N_Cols; i++) {
		errc |= m_genCol->spawn(hexedImg, colPlates->get(i), m_Cols[i]);
	}
	return errc;
}
void Col::despawn(s_ColPlateLayer* colPlates) {
	for (int i = 0; i < m_N_Cols; i++)
		m_genCol->despawn(colPlates->get(i));
	releasePlateLayer(colPlates);
}
unsigned char Col::samLayer(s_ColPlateLayer* colPlates, int plate_i[], int plate_N, s_HexBasePlateLayer* sLayer) {
	if (plate_i == NULL || plate_N<1)
		return ECODE_ABORT;
	if (sLayer == NULL)
		return ECODE_ABORT;
	if (Err(sLayer->init(plate_N)))
		return ECODE_FAIL;
	for (int i = 0; i < plate_N; i++) {
		int cur_plate_index = plate_i[i];
		if (cur_plate_index<0 || cur_plate_index>colPlates->N)
			return ECODE_ABORT;
		s_HexBasePlate* cur_col_plate = (s_HexBasePlate*)colPlates->get(cur_plate_index);
		int cur_sam_i = sLayer->N;
		if (cur_sam_i >= sLayer->getNmem())
			return ECODE_ABORT;
		sLayer->p[cur_sam_i] = (s_HexPlate*)cur_col_plate;
		sLayer->N += 1;
	}
	return ECODE_OK;
}
void Col::desamLayer(s_HexBasePlateLayer* sLayer) {
	if (sLayer == NULL)
		return;
	for (int ii = 0; ii < sLayer->N; ii++)
		sLayer->p[ii] = NULL;
	sLayer->N = 0;
	sLayer->release();
}
unsigned char Col::initPlateLayer(s_ColPlateLayer* colPlates) {
	if (colPlates == NULL)
		return ECODE_ABORT;
	if (Err(colPlates->init(m_N_Cols)))
		return ECODE_FAIL;
	for (int i = 0; i < m_N_Cols; i++) {
		colPlates->p[i] = (s_HexPlate*)(new s_ColPlate);
	}
	return ECODE_OK;
}
void Col::releasePlateLayer(s_ColPlateLayer* colPlates) {
	/*assumes that the individual plates in the layer have already been released*/
	if (colPlates != NULL) {
		for (int i = 0; i < colPlates->getNmem(); i++) {
			if (colPlates->p[i] != NULL)
				delete colPlates->p[i];
		}
		colPlates->release();
	}
}

bool n_Col::run(s_HexBasePlate* hexedImg, s_ColPlateLayer* colPlates, long plate_hex_index) {
	bool retVal = true;
	for (long ii = 0; ii < colPlates->N; ii++)
		retVal &= runPlate(hexedImg, colPlates, ii, plate_hex_index);
	return retVal;
}
bool n_Col::runPlate(s_HexBasePlate* hexedImg, s_ColPlateLayer* colPlates, long layer_index, long plate_hex_index) {
	s_ColPlate* colPlt = colPlates->get(layer_index);
	return n_ColPlate::run(hexedImg, colPlt, plate_hex_index);
}