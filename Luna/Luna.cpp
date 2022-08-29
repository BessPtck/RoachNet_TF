#include "Luna.h"

unsigned char s_Luna::init() {
	unsigned char err=s_CNnets::init(NUM_LUNA_PATTERNS);
	if (err != ECODE_OK)
		return err;
	eye = new s_HexEye;
	if (eye == NULL)
		return ECODE_FAIL;
	for (int i = 0; i < N_mem; i++) {
		net[i] = new s_Net;
		if (net[i] == NULL)
			return ECODE_FAIL;
	}
	N = N_mem;
	return ECODE_OK;
}
unsigned char s_Luna::init(const s_Luna& other) {
	unsigned char err = s_CNnets::init(other);
	if (err != ECODE_OK)
		return err;
	this->eye = new s_HexEye;/*luna unlike some CNnets owns its own eye */
	if (this->eye == NULL)
		return ECODE_FAIL;
	err = this->eye->init(*other.eye);
	return err;
}
void s_Luna::release() {
	s_CNnets::release();
	if (eye != NULL) {
		eye->release();
		delete eye;
	}
	eye = NULL;
}
bool n_Luna::run(s_Luna* lun, s_HexBasePlateLayer& colPlates, s_HexBasePlateLayer& lunPlates, long plate_index) {
	s_HexBasePlate* basePlate = colPlates.get(0);/*all plates should have the exact same geometry*/
	bool locRootGood = rootEye(lun, *basePlate, plate_index);
	if (locRootGood) {
		rootOnPlates(lun, colPlates);
		runLunaPatterns(lun);
		for (int i_luna = 0; i_luna < lun->N; i_luna++) {
			s_Hex* lunPlate_hex = lunPlates.get(i_luna)->get(plate_index);
			lunPlate_hex->o = lun->net[i_luna]->o;
		}
	}
	return locRootGood;
}
void n_Luna::runLunaPatterns(s_Luna* lun) {
	int NumLunaPatterns = lun->N; /*this should be the same as NUM_LUNA_PATTERNS
								    it is also the same as the number of nets in the s_Luna */
	for (int i_luna = 0; i_luna < NumLunaPatterns; i_luna++) {
		s_Net* luna_net = lun->net[i_luna];
		int NumColPlates = luna_net->getBottom()->getNumHanging();
		luna_net->hanging_plate_i = -1;
		luna_net->o = -1.f;
		for (int i_plate = 0; i_plate < NumColPlates; i_plate++) {
			float o = runLunaPat_on_plate(luna_net, i_plate);
			if (o > luna_net->o) {
				luna_net->o = o;
				luna_net->hanging_plate_i = i_plate;
			}
		}
	}
}
float n_Luna::runLunaPat_on_plate(s_Net* lunaNet, int plate_i) {
	s_nNode* topNode = lunaNet->getTop()->get(0);
	s_nPlate* botLuna = lunaNet->getBottom();
	float o = 0.f;
	for (int i = 0; i < topNode->N; i++) {/*since this only a 2 level eye that the order of the hanging nodes(that corresponds to
										  the order of the weights and the order of the node indexes in the bottom plate is the same */
		float hex_o = botLuna->get(i)->nodes[plate_i]->o;
		float wt = topNode->w[i];
		o += wt * hex_o;
	}
	topNode->o = o;
	return o;
}
Luna::Luna():m_NetMaster(NULL), m_EyeMaster(NULL), m_num_color_plates(0) { ; }
Luna::~Luna() { ; }

unsigned char Luna::init(float r, int num_color_plates) {
	if (num_color_plates < 1)
		return ECODE_ABORT;
	m_num_color_plates = num_color_plates;
	m_NetMaster = new sNet;
	if (m_NetMaster == NULL)
		return ECODE_FAIL;
	m_EyeMaster = new HexEye;
	if (m_EyeMaster == NULL)
		return ECODE_FAIL;
	unsigned char err = ECODE_OK;
	err = m_EyeMaster->init(r, 2);
	if (Err(err))
		return err;
	err = m_NetMaster->init(m_EyeMaster, m_num_color_plates);
	return err;
}
void Luna::release() {
	if (m_NetMaster != NULL) {
		m_NetMaster->release();
		delete m_NetMaster;
	}
	m_NetMaster = NULL;
	if (m_EyeMaster != NULL) {
		m_EyeMaster->release();
		delete m_EyeMaster;
	}
	m_EyeMaster = NULL;
	m_num_color_plates = 0;
}
unsigned char Luna::spawn(s_Luna* lun) {
	if (lun == NULL)
		return ECODE_ABORT;
	lun->init();
	/*spawn eye*/
	lun->eye = new s_HexEye;
	unsigned char err = m_EyeMaster->spawn(lun->eye);
	if (Err(err))
		return err;
	for (int i_luna = 0; i_luna < NUM_LUNA_PATTERNS; i_luna++) {
		lun->net[i_luna] = new s_Net;
		err |= m_NetMaster->spawn(lun->net[i_luna], lun->eye);
		if (RetOk(err))
			err |= connLunaInterLinks(lun->net[i_luna], lun->eye);
	}
	lun->N = NUM_LUNA_PATTERNS;
	if (Err(err))
		return err;
	/*finished setting up the structure */
	/* now fill the w's so that the luna generate the correct patterns*/
	return genLunaPatterns(lun);
}
void Luna::despawn(s_Luna* lun) {
	if (lun == NULL)
		return;
	for (int i_luna = 0; i_luna < lun->N; i_luna++) {
		if (lun->net[i_luna] != NULL) {
			m_NetMaster->despawn(lun->net[i_luna]);
			delete lun->net[i_luna];
		}
		lun->N = 0;
		lun->net[i_luna] = NULL;
	}
	if (lun->eye != NULL) {
		m_EyeMaster->despawn(lun->eye);
		delete lun->eye;
	}
	lun->eye = NULL;
	lun->release();
}
unsigned char Luna::connLunaInterLinks(s_Net* sn, s_HexEye* eye) {
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