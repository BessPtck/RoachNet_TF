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
bool n_Luna::run(s_HexBasePlateLayer* colPlates, s_HexBasePlateLayer* lunPlates, long plate_index) {
	for (int lun_i = 0; lun_i < lunPlates->N; lun_i++) {
		s_HexPlate* lun_plate = (s_HexPlate*)lunPlates->get(lun_i);
		s_lunHex* lun_hex = (s_lunHex*)lun_hex->nodes[plate_index];
		float highest_o = -1.f;
		int   highest_col_i = -1;
		for (int i_colplate = 0; i_colplate < colPlates->N; i_colplate++) {
			if (runImbeddedLuna(lun_hex, (s_HexPlate*)colPlates->p[i_colplate])) {
				float o = lun_hex->o;
				if (o > highest_o) {
					highest_o = o;
					highest_col_i = i_colplate;
				}
			}
			else
				return false; /*this luna is on the edge*/
		}
		lun_hex->o = highest_o;
		lun_hex->col_i = highest_col_i;
	}
	return true;
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
bool n_Luna::runImbeddedLuna(s_lunHex* lun, s_HexPlate* colPlate) {
	float wsum = 0.f;
	for (int i_down = 0; i_down < lun->N; i_down++) {
		s_Node* nd_down = lun->nodes[i_down];
		if (nd_down == NULL)
			return false;
		long plate_index = nd_down->thislink;
		s_Node* plate_nd = colPlate->get(plate_index);
		float w = lun->w[i_down];
		float x = plate_nd->o;
		wsum += w * x;
	}
	return Math::StepFuncSym(wsum);
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
	unsigned char err = m_EyeMaster->spawn(lun->eye);
	if (Err(err))
		return err;
	for (int i_luna = 0; i_luna < NUM_LUNA_PATTERNS; i_luna++) {
		err |= m_NetMaster->spawn(lun->net[i_luna], lun->eye);
		if (RetOk(err))
			err |= connLunaInterLinks(lun->net[i_luna], lun->eye);
	}
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
		}
	}
	if (lun->eye != NULL) {
		m_EyeMaster->despawn(lun->eye);
	}
	lun->release();
}
unsigned char Luna::spawn(s_Luna* lun, s_HexBasePlateLayer* lunPlates, s_HexBasePlate* base_plate) {
	if (lun == NULL || lunPlates == NULL || base_plate == NULL)
		return ECODE_ABORT;
	unsigned char err = spawn(lun);
	if (Err(err))
		return err;
	err = lunPlates->init(lun->N);
	if (Err(err))
		return err;
	for (int i = 0; i < lunPlates->getNmem(); i++) {
		lunPlates->p[i] = new s_HexBasePlate;
		if (lunPlates->p[i] == NULL)
			return ECODE_FAIL;
		err = lunPlates->p[i]->init(base_plate);/*this initializes the plate but does not connect the down links, or fill the w's for the down links*/
		if (Err(err))
			return err;
		err = replaceHexWithLuna_inPlate(lunPlates->p[i]);
		if (Err(err))
			return err;
		err = setDownLinks(lunPlates->p[i], base_plate);
		if (Err(err))
			return err;
		err = imbedLunaInPlate(lun, i, lunPlates->p[i]);
		if (Err(err))
			return err;
		lunPlates->N++;
	}
	return ECODE_OK;
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
unsigned char Luna::replaceHexWithLuna_inPlate(s_HexPlate* lunPlate) {
	for (long i_hex = 0; i_hex < lunPlate->N; i_hex++) {
		s_Hex* throw_node = lunPlate->get(i_hex);
		s_lunHex* lun_node = new s_lunHex;
		unsigned char err = lun_node->init(throw_node);
		if (Err(err))
			return err;
		lunPlate->nodes[i_hex] = (s_Node*)lun_node;
		throw_node->release();
		delete throw_node;
	}
	return ECODE_OK;
}
unsigned char Luna::setDownLinks(s_HexPlate* lunPlate, s_HexBasePlate* base_plate) {
	for (long i_hex = 0; i_hex < lunPlate->N; i_hex++) {
		s_Hex* topHex = lunPlate->get(i_hex);
		s_Hex* bot_center_node = base_plate->get(i_hex);
		topHex->nodes[0] = (s_Node*)bot_center_node;
		for (int i_web = 0; i_web < 6; i_web++) {
			s_Node* bot_node = bot_center_node->web[i_web];
			int i_top_web = i_web + 1;
			topHex->nodes[i_top_web] = bot_node;
		}
	}
	return ECODE_OK;
}
unsigned char Luna::imbedLunaInPlate(s_Luna* lun, int luna_i, s_HexPlate* lunPlate) {
	s_Net* net = lun->net[luna_i];
	s_nPlate* topLunPlate = net->getTop();
	s_nNode* lunNd = topLunPlate->get(0);
	for (long i_hex = 0; i_hex < lunPlate->N; i_hex++) {
		s_lunHex* topNd = (s_lunHex*)lunPlate->nodes[i_hex];
		for (int i_w = 0; i_w < lunNd->N; i_w++) {/*assumes that topNd, N is 7 and lunNd N is 7 */
			topNd->w[i_w] = lunNd->w[i_w];
		}
	}
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