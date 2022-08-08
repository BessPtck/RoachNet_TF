#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H

#ifndef MATH_H
#include "Math.h"
#endif

class s_Node {
public:
	float x;
	float y;

	long thislink;
	s_Node** nodes;/*lower nodes*/
	int N;

	float o;/*used as colset flag for hex nodes*/
};

class s_Hex : public s_Node {
public:
	long i;
	long j;

	s_Node** web;
	int      Nweb;

	float rgb[3];
};

class s_nNode : public s_Node {/*nnet node*/
public:
	s_Hex* hex;/*hex node this NNet node  is attached to if such is linked*/

	float* w;/*w will have the length of nodes, N */
	float b;
};

class s_Plate {
public:
	s_Node* nodes;
	long    N;
};
class s_HexPlate : public s_Plate{
public:
	long height;
	long width;
	float Rhex;
	float RShex;
	float Shex;
	s_2pt hexU[6];
};
class s_HexBasePlate : public s_HexPlate {
public:
	/*for fast san xy to plate loc for square plate configuration*/
	s_2pt* RowStart; /*xy position of the first hex in each row*/
	s_2pt_i* RowStart_is;/* index in the hex array of the row start, and number in row*/
	long   Row_N;
	float  Col_d;/*seperation between columns 2/3 * R */
	float  Row_d;/*seperation between rows 2*RS */
};

class s_PlateLayer {
public:
	s_Plate** p;
	int N;
};
#endif