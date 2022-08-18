#include "sNet.h"
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