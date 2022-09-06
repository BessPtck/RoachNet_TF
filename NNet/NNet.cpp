#include "NNet.h"

unsigned char NNet::init(
	float r,
	int N_low_plates,
	int N_nets,
	int N_lev,
	int N_lev_trigger
) {
	if (N_nets < 1)
		return ECODE_ABORT;
	m_NetGen = new sNet;
	m_HexEyeGen = new HexEye;
	m_r = r;
	m_N_nets = N_nets;
	m_N_lev = N_lev;
	unsigned char err = m_HexEyeGen->init(m_r, N_lev);
	if (IsErrFail(err))
		return err;
	m_N_plates = N_low_plates;
	err |= m_NetGen->init(m_HexEyeGen, m_N_plates);
	if (IsErrFail(err))
		return err;
	m_N_lev_trigger = N_lev_trigger;
	return err;
}
void NNet::release() {
	m_N_lev_trigger = 0;
	if (m_NetGen != NULL) {
		m_NetGen->release();
		delete m_NetGen;
	}
	m_NetGen = NULL;
	m_N_plates = 0;
	if (m_HexEyeGen != NULL) {
		m_HexEyeGen->release();
		delete m_HexEyeGen;
	}
	m_HexEyeGen = NULL;
	m_N_lev = 0;
	m_N_nets = 0;
	m_r = 0.f;
}

unsigned char NNet::spawn(s_CNnets* nets) {
	if (nets == NULL)
		return ECODE_ABORT;
	unsigned char err = nets->init(m_N_nets);
	if (Err(err))
		return err;
	nets->eye = new s_HexEye;
	err = m_HexEyeGen->spawn(nets->eye);
	if (Err(err))
		return err;
	for (int i_net = 0; i_net < m_N_nets; i_net++) {
		nets->net[i_net] = new s_Net;
		err = m_NetGen->spawn(nets->net[i_net], nets->eye);
		if (Err(err))
			return err;
	}
	err = importFile(nets);
	if (IsErrFail(err))
		return err;
	if(IsAbort(err))
		err=preSetWeights(nets);
	return err;
}



unsigned char NNet::preSetWeights(s_CNnets* nets) {
	if (nets == NULL)
		return ECODE_ABORT;
	for (int i = 0; i < nets->N; i++) {
		unsigned char err = preSetWeightNet(nets->net[i], nets->eye);
		if (Err(err))
			return err;
	}
	return ECODE_OK;
}
unsigned char NNet::preSetWeightNet(s_Net* net, s_HexEye* eye) {
	if (net == NULL)
		return ECODE_ABORT;
	if (net->N < 1)
		return ECODE_ABORT;
	/*set the weights for the hanging*/
	s_nPlate* netBot = net->getBottom();
	for (int i = 0; i < netBot->N; i++) {
		s_nNode* nd = netBot->get(i);
		/*in this kind of net the pointers should always be non-null*/
		for (int i_hang = 0; i_hang < nd->N; i_hang++)
			nd->w[i_hang] = 0.f;
		/*m_N_plates x netBot->N should be total number of hanging*/
		for (int i_plate = 0; i_plate < m_N_plates; i_plate++) {
			int i_assoc = i * m_N_plates + i_plate;
			nd->w[i_assoc] = 1.f;
		}
	}
	/*set the weights for the connected */
	if (net->N < 2)
		return ECODE_OK;
	if (eye == NULL)
		return ECODE_ABORT;
	if (net->N != eye->N)
		return ECODE_FAIL;
	for (int i_lev = 0; i_lev < (net->N-1); i_lev++) {
		s_nPlate* topP = net->lev[i_lev];
		s_nPlate* botP = net->lev[i_lev + 1];
		s_HexPlate* eyeTopP = eye->get(i_lev);
		s_HexPlate* eyeBotP = eye->get(i_lev + 1);
		if (topP->N != eyeTopP->N)
			return ECODE_ABORT;
		for (int i_top = 0; i_top < topP->N; i_top++) {
			s_nNode* topNd = topP->get(i_top);
			for (int i_hang = 0; i_hang < topNd->N; i_hang++)
				topNd->w[i_hang] = 0.f;
		}
		if (botP->N != eyeBotP->N)
			return ECODE_ABORT;
		for (int i_top = 0; i_top < eyeTopP->N; i_top++) {
			s_nNode* top_nd = topP->get(i_top);
			if (top_nd->N != eyeBotP->N)
				return ECODE_ABORT;
			/*lower nodes from the nNode should connect to all bottom nodes in order of their indexes*/
			s_Hex* hex_top_nd = eyeTopP->get(i_top);
			/*find indexes of lower nodes this top node is connected to*/
			for (int i_hex_hang = 0; i_hex_hang < hex_top_nd->N; i_hex_hang++) {
				s_Node* hex_bot_nd = hex_top_nd->nodes[i_hex_hang];
				long bot_plate_index = hex_bot_nd->thislink;
				/*the bot_plate_index should be the same in both the eye plate and the net plate*/
				/* the w's from the net plate cover and are ordered the same as the order of the nodes in the next net plate down*/
				top_nd->w[bot_plate_index] = 1.f;
			}
		}
	}
	return ECODE_OK;
}