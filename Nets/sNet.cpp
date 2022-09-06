#include "sNet.h"
unsigned char s_Net::init(int nLev) {
	if (nLev < 1)
		return ECODE_ABORT;
	lev = new s_nPlate * [nLev];
	if (lev == NULL)
		return ECODE_FAIL;
	N_mem = nLev;
	N = 0;
	eye = NULL;
	return ECODE_OK;
}
unsigned char s_Net::init(const s_Net& other) {
	unsigned char err = init(other.N_mem);
	if (err != ECODE_OK)
		return err;
	this->eye = other.eye;
	this->N = other.N;
	for (int ii = 0; ii < N; ii++) {
		s_nPlate* other_lev = other.lev[ii];
		if (other_lev != NULL) {
			this->lev[ii] = new s_nPlate;
			if ((this->lev[ii]) == NULL)
				return ECODE_FAIL;
			err = this->lev[ii]->init(other_lev);
			if (err != ECODE_OK)
				return err;
		}
	}
	/* fix intra-plate links */
	for (int ii = 0; ii < (N - 1); ii++) {
		s_Plate* topLev = this->lev[ii];
		s_Plate* lowLev = this->lev[ii + 1];
		n_Plate::fixStackedPlateLinks(topLev, lowLev);
	}
	return ECODE_OK;
}
void s_Net::release() {
	if (lev != NULL) {
		for (int ii = 0; ii < N_mem; ii++) {
			if (lev[ii] != NULL) {
				lev[ii]->release();
				delete lev[ii];
			}
			lev[ii] = NULL;
		}
		delete[] lev;
	}
	lev = NULL;
	N_mem = 0;
	N = 0;
}
void n_Net::rootNNet(s_Net* net, s_HexPlate* eye_base, s_HexBasePlateLayer* plates) {
	/*assumes that net_base has same number of nodes as eye_base and that both have more than one node/hex */
	s_nPlate* net_base = net->getBottom();
	int net_hanging = 0;
	s_nNode* net_node = net_base->get(0);
	for (long hex_i = 0; hex_i < eye_base->N; hex_i++) {
		long plate_index = eye_base->nodes[hex_i]->thislink;
		for (int plate_i = 0; plate_i < plates->N; plate_i++) {
			net_node->nodes[net_hanging] = plates->get(plate_i)->getNd(plate_index);
			net_hanging++;
		}
	}
	/*all nodes on the bottom of the net point to the same set of pointers*/
	for (long hex_i = 1; hex_i < net_base->N; hex_i++) {
		s_nNode* net_node_to_dup = net_base->get(hex_i);
		for (int hanging_i = 0; hanging_i < net_hanging; hanging_i++)
			net_node_to_dup->nodes[hanging_i] = net_node->nodes[hanging_i];
	}
}
void n_Net::runRootedNNet(s_Net* net) {
	/*assume that net has at least one level*/
	for (int lev_i = net->N - 1; lev_i >= 0; lev_i--) {
		s_nPlate* net_lev = net->lev[lev_i];
		for (long nd_i = 0; nd_i < net_lev->N; nd_i++) {
			s_nNode* net_node = net_lev->get(nd_i);
			float node_sum = 0.f;
			for (int w_i = 0; w_i < net_node->N; w_i++) {
				float X = net_node->nodes[w_i]->o;
				float w = net_node->w[w_i];
				node_sum += w * X;
			}
			float node_o = Math::StepFuncSym(node_sum);
			net_node->o = node_o;
		}
	}
	net->o = net->getTop()->get(0)->o;
}

unsigned char s_CNnets::init(int nNets) {
	N = 0;
	eye = NULL;
	trigger_node = NULL;
	N_mem = 0;
	net = NULL;
	if (nNets < 1)
		return ECODE_OK;
	net = new s_Net * [nNets];
	if (net == NULL)
		return ECODE_FAIL;
	N_mem = nNets;
	for (int ii = 0; ii < N_mem; ii++)
		net[ii] = NULL;
	return ECODE_OK;
}
unsigned char s_CNnets::init(const s_CNnets& other) {
	unsigned char err = init(other.N_mem);
	if (err != ECODE_OK)
		return err;
	/*generally  assume that eye is NOT owned*/
	this->eye = other.eye;
	if (other.net != NULL) {
		for (int ii = 0; ii < N_mem; ii++) {
			if (other.net[ii] != NULL) {
				this->net[ii] = new s_Net;
				if (this->net[ii] == NULL)
					return ECODE_FAIL;
				err = this->net[ii]->init(*other.net[ii]);
				if (err == ECODE_FAIL)
					return err;
				this->N++;
			}
		}
	}
	if (other.trigger_node != NULL) {
		this->trigger_node = new s_nNode;
		if (this->trigger_node == NULL)
			return ECODE_FAIL;
		err=this->trigger_node->init(other.trigger_node);
		if (err != ECODE_OK)
			return err;
	}
	else
		this->trigger_node = NULL;
	return ECODE_OK;
}
void s_CNnets::release() {
	if (trigger_node != NULL) {
		trigger_node->release();
		delete trigger_node;
	}
	trigger_node = NULL;
	N = 0;
	if (net != NULL) {
		for (int ii = 0; ii < N_mem; ii++) {
			if (net[ii] != NULL) {
				net[ii]->release();
				delete net[ii];
			}
			net[ii] = NULL;
		}
		delete[] net;
	}
	net = NULL;
	N_mem = 0;
}
bool n_CNnets::rootEye(s_CNnets* nets, s_HexBasePlate& basePlate, long plate_index) {
	/*in interest of speed assumes the eye has been setup correctly as non-null*/
	unsigned char err = n_HexEye::imgRoot(nets->eye, &basePlate, plate_index);
	if (err != ECODE_OK)
		return false;
	return true;
}
void n_CNnets::rootOnPlates(s_CNnets* nets, s_HexBasePlateLayer& plates) {
	/*assumes eye has already been rooted*/
	s_HexPlate* eye_base = nets->eye->getBottom();
	for (long ii = 0; ii < eye_base->N; ii++) {
		long plate_index = eye_base->nodes[ii]->thislink;
		for (int i_net = 0; i_net < nets->N; i_net++) {
			s_nPlate* net_base = nets->net[i_net]->getBottom();
			s_nNode* net_node = net_base->get(ii);
			for (int i_hanging = 0; i_hanging < net_node->N; i_hanging++) {
				s_HexBasePlate* sel_plate = plates.get(i_hanging);
				net_node->nodes[i_hanging] = sel_plate->nodes[plate_index];
			}
		}
	}
}
void n_CNnets::rootNNet(s_CNnets* nets, s_HexBasePlateLayer* plates) {
	s_HexPlate* eye_base = nets->eye->getBottom();
	for (int i_net = 0; i_net < nets->N; i_net++) {
		n_Net::rootNNet(nets->net[i_net], eye_base, plates);
	}
}
bool n_CNnets::runNNet(s_CNnets* nets, s_HexBasePlateLayer* plates, long plate_index) {
	if (!rootEye(nets, *(plates->get(0)), plate_index))/*choose the first plate for the root all plates should have the same geometry*/
		return false;
	rootNNet(nets, plates);
	for (int i_net = 0; i_net < nets->N; i_net++) {
		n_Net::runRootedNNet(nets->net[i_net]);
	}
}
sNet::sNet() :m_nLev(0), m_numLevNodes(NULL), m_numHanging(0) { ; }
sNet::~sNet() { ; }
unsigned char sNet::init(int nLev, int numLevNodes[], int numHanging) {
	if (nLev < 1)
		return ECODE_ABORT;
	for (int i = 0; i < nLev; i++)
		if (numLevNodes[i] < 1)
			return ECODE_ABORT;
	m_nLev = nLev;
	m_numLevNodes = new int[m_nLev+1];
	for (int i = 0; i < nLev; i++)
		m_numLevNodes[i] = numLevNodes[i];
	m_numHanging = numHanging;
	m_numLevNodes[nLev] = numHanging;
	return ECODE_OK;
}
unsigned char sNet::init(HexEye* eye, int numPlates) {
	if (eye == NULL)
		return ECODE_ABORT;
	int m_nLev = eye->getNLevels();
	if (m_nLev < 1)
		return ECODE_ABORT;
	m_numLevNodes = new int[m_nLev + 1];
	for (int i = 0; i < m_nLev; i++) {
		m_numLevNodes[i] = eye->getNHexes(i);
	}
	m_numLevNodes[m_nLev] = numPlates;
	m_numHanging = numPlates;
}
void sNet::release() {
	m_numHanging = 0;
	if (m_numLevNodes != NULL)
		delete[] m_numLevNodes;
	m_numLevNodes = NULL;
	m_nLev = 0;
}
unsigned char sNet::spawn(s_Net* sn) {
	if (sn == NULL)
		return ECODE_ABORT;
	if (Err(sn->init(m_nLev)))
		return ECODE_FAIL;
	for (int ii = 0; ii < m_nLev; ii++) {
		unsigned char err = sn->lev[ii]->init((long)m_numLevNodes[ii], m_numLevNodes[ii + 1]);
		if (Err(err))
			return err;
		sn->N++;
	}
	return connDownNet(sn);
}
unsigned char sNet::spawn(s_Net* sn, s_HexEye* eye) {
	unsigned char err = spawn(sn);
	if (Err(err))
		return err;
	/*connect bottom plate to eye*/
	for (int ii = 0; ii < eye->getBottom()->N; ii++) {
		s_nNode* net_node = sn->getBottom()->get(ii);
		s_Hex* eye_node = eye->getBottom()->get(ii);
		net_node->hex = eye_node;
	}
	/*connect top plate/node to eye */
	s_nNode* net_top_node = sn->lev[0]->get(0);
	s_Hex* eye_top_node = eye->lev[0]->get(0);
	net_top_node->hex = eye_top_node;

	sn->eye = eye;
	return ECODE_OK;
}
void sNet::despawn(s_Net* sn) {
	if (sn == NULL)
		return;
	sn->eye = NULL;
	for (int ii = 0; ii < m_nLev; ii++) {
		if (sn->lev[ii] != NULL) {
			sn->lev[ii]->release();
		}
	}
	sn->release();
	sn->N = 0;
} 
int sNet::getTotalNumWeights(s_Net* sn) {
	if (sn == NULL)
		return -1;
	int num_weights = 0;
	if (sn->N < 2)
		return num_weights;
	for (int i = 1; i < sn->N; i++) {
		num_weights += sn->lev[i]->N;
	}
	s_nPlate* botLev = sn->getBottom();
	int num_bot_nodes = botLev->N;
	for (int i = 0; i < num_bot_nodes; i++) {
		s_Node* bot_node = botLev->getNd(i);
		if (bot_node != NULL)
			num_weights += bot_node->N;
	}
	return num_weights;
}
int sNet::getTotalNumNodes(s_Net* sn) {
	if (sn == NULL)
		return -1;
	int num_nodes = 0;
	if (sn->N < 1)
		return num_nodes;
	for (int i = 0; i < sn->N; i++) {
		num_nodes += sn->lev[i]->N;
	}
	return num_nodes;
}
unsigned char sNet::connDownNet(s_Net* sn) {
	int num_net_lev = sn->N;
	if (num_net_lev < 1)
		return ECODE_ABORT;
	for (int ii = 0; ii < (num_net_lev - 1); ii++) {
		int num_low_lev_nodes = (int)sn->lev[ii+1]->N;
		int num_lev_nodes = (int)sn->lev[ii]->N;
		for (int i_nd = 0; i_nd < num_lev_nodes; i_nd++) {
			s_nNode* nd = sn->lev[ii]->get(i_nd);
			int num_hanging = nd->getNmem();
			if (num_hanging != num_low_lev_nodes)
				return ECODE_FAIL;
			for (int i_hanging = 0; i_hanging < num_hanging; i_hanging++) {
				s_Node* hanging_nd = sn->lev[ii + 1]->getNd(i_hanging);
				nd->nodes[i_hanging] = hanging_nd;
			}
			nd->N = num_hanging;
		}
	}
	return ECODE_OK;
}
/*
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
*/