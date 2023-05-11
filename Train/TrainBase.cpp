#include "TrainBase.h"

unsigned char TrainBase::init(string& L) {
	m_L = L;
	string first_img_file(m_L);
	string num = n_ParseTxt::intToFileNameExt(0);
	string fileName ="0_" + num + ".tga";
	first_img_file += fileName;
	m_tga = new CTargaImage;
	m_tga->Init();
	if (Err(m_tga->Open(first_img_file.c_str()))) {
		std::cout<<"Failed to open file: "<<first_img_file;
		m_tga->Release();
		delete m_tga;
		return ECODE_ABORT;
	}
	genNewDataPlateObjects();
	genDataNetObjects();
	genGenObjects();
	if (Err(InitImgFixDim(m_tga)))
		return ECODE_FAIL;
	/*image has been transfered so can now release*/
	m_tga->Close();
	if (Err(InitGenObjs()))
		return ECODE_FAIL;
	if (Err(SpawnDataObjs()))
		return ECODE_FAIL;
	if (Err(SpawnNetObjs()))
		return ECODE_FAIL;
	m_parse = new ParseTxt;

	return ECODE_OK;
}




unsigned char TrainBase::InitImgFixDim(CTargaImage* fileImg) {
	if (m_img == NULL || fileImg == NULL)
		return ECODE_FAIL;
	return m_img->init(fileImg->GetImage(), fileImg->GetWidth(), fileImg->GetHeight(), fileImg->GetColorMode());
}

unsigned char TrainBase::SpawnNetObjs() {
	if (Err(m_genLuna->spawn(m_lunaNets_t1)))
		return ECODE_FAIL;
}
unsigned char TrainBase::SpawnDataObjs() {
	/*genHexImg was initialized with the init for HexImg*/
	/*initialize the col that will be used for the single plate in this case*/
	s_ColWheel colW;
	n_ColWheel::clear(colW);
	colW.pixMax = 255.f;
	colW.Dhue = 0.3f;
	colW.DI = 0.7f;
	colW.DSat = 0.3f;
	colW.HueFadeV = 0.7f;
	colW.I_target = 0.9f;
	colW.Hue_target.x0 = 1.0f;
	colW.Hue_target.x1 = 0.0f;
	colW.Sat_target = 0.1f;
	colW.finalScaleFactor = 10.f;

	return ECODE_OK;
}
void TrainBase::delDataPlateObjects() {
	if (m_L1Plates_t1 != NULL)
		delete m_L1Plates_t1;
	if (m_lunPlates_t1 != NULL)
		delete m_lunPlates_t1;
	if (m_lunBasePlates_t1 != NULL)
		delete m_lunBasePlates_t1;
	if (m_ColPlates != NULL)
		delete m_ColPlates;
	if (m_hexedImg != NULL)
		delete m_hexedImg;
	if (m_img != NULL)
		delete m_img;
}
void TrainBase::delDataNetObjects() {
	if (m_L2Nets_t1 != NULL)
		delete m_L2Nets_t1;
	if (m_L1Nets_t1 != NULL)
		delete m_L1Nets_t1;
	if (m_lunaNets_t1 != NULL) {
		delete m_lunaNets_t1;
	}
}
void TrainBase::delGenObjects() {
	if (m_genL1NNet != NULL)
		delete m_genL1NNet;
	if (m_genLuna != NULL)
		delete m_genLuna;
	if (m_genCol != NULL)
		delete m_genCol;
	if (m_genHexImg != NULL)
		delte m_genHexImg;
}
void TrainBase::releaseImg() {
	if (m_img != NULL)
		m_img->release();
}
void TrainBase::releaseGenObjs() {
	if (m_genLuna != NULL)
		m_genLuna->release();
	if (m_genCol != NULL)
		m_genCol->release();
	if (m_genHexImg != NULL)
		m_genHexImg->release();
}
void TrainBase::deSpawnNetObjs() {
	if (m_genLuan != NULL)
		m_genLuna->despawn(m_lunaNets_t1);
}
void TrainBase::deSpawnDataObjs() {
	if (m_genCol != NULL) {
		m_genCol->desamLayer(m_lunBasePlates_t1);
	}
	if (m_genCol != NULL)
		m_genCol->despawn(m_colPlates);
}
unsigned char TrainBase::runToLuna_Img(int img_i) {
	if (m_img == NULL || m_lunBasePlates_t1==NULL)
		return ECODE_FAIL;
	/*need to replaced this with GenPreImgs the img is not actually saved*/
	string num = n_ParseTxt::intToFileNameExt(img_i);
	string fileName = m_L + "0_" + num + ".tga";
	if (Err(m_tga->Open(first_img_file.c_str()))) {
		std::cout << "Failed to open file: " << fileName;
		return ECODE_ABORT;
	}
	m_img->release();
	m_img->init(m_tga->GetImage(), m_tga->GetWidth(), m_tga->GetHeight(), m_tga->GetColorMode());
	m_genHexImg->update(m_img);
	m_genHexImg->run();
	s_HexBasePlate top_preLunaPlate = m_lunBasePlates_t1->get(0);
	bool run_ok = true;
	for(long ii=0; ii<top_preLunaPlate->N; ii++)
		run_ok = n_Luna(m_lunaNets_t1, *m_lunBasePlates_t1, *m_lunPlates_t1, ii);/*need to check if edge hexes are ok*/
	if (!run_ok)
		return ECODE_FAIL;
	return ECODE_OK;
}