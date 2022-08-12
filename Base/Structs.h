#pragma once
#ifndef STRUCTS_H
#define STRUCTS_H

#ifndef BASE_H
#include "Base.h"
#endif

class s_Node {/*prob 44 bytes*/
public:
	s_Node();
	s_Node(const s_Node& other);/*this is only designed to work if the nodes are NOT owned*/
	~s_Node();

	virtual unsigned char init(int nNodes);
	virtual void release();
	unsigned char genSubNodes();
	void          releaseSubNodes();
	virtual void reset();

	virtual s_Node& operator=(const s_Node& other);/*the node pointers from nodes are copied but not owned by the copy
										   the size of nodes in memory must be equal for 
										   the node pointers to be copied */

	float x; /* prob 64 bit*/
	float y;

	long thislink; /* prob 64 bit*/
	s_Node** nodes;/*lower nodes*//* prob 64 bit*/
	int N; /* prob 32 bit */

	float o;/*used as colset flag for hex nodes*/

protected:
	void copy(const s_Node* other);
	int N_mem;/*lenght of nodes pointer array in memory*/
};

class s_Hex : public s_Node {
public:
	s_Hex();
	s_Hex(const s_Hex& other);
	~s_Hex();

	unsigned char init();
	void          release();
	s_Hex& operator=(const s_Hex& other);
	inline void setRGB(float col_rgb[]) { rgb[0] = col_rgb[0]; rgb[1] = col_rgb[1]; rgb[2] = col_rgb[2]; }
	inline void setRGB(float col_r, float col_g, float col_b) { rgb[0] = col_r; rgb[1] = col_g; rgb[2] = col_b; }
	inline void setColSet() { o = 1.f; }
	inline bool colSet() { return (o > 0.5f); }

	long i;
	long j;

	s_Node*  web[6];/*web will always be initialized to 6 pointers*/

	float rgb[3];
};

class s_nNode : public s_Node {/*nnet node*/
public:
	s_nNode();
	~s_nNode();

	unsigned char init(int nNodes);
	void          release();
	void          reset();
	s_nNode& operator=(const s_nNode& other);

	s_Hex* hex;/*hex node this NNet node  is attached to if such is linked*/

	float* w;/*w will have the length of nodes, N */
	float b;
};

class s_Plate {
public:
	s_Plate();
	~s_Plate();

	virtual unsigned char init(long nNodes);
	virtual void          release();/*assumes that the plate owns its subnodes if not NULL*/

	inline virtual void set(long indx, s_Node* nd) { this->nodes[indx] = nd; }
	inline virtual s_Node* get(long indx) { return this->nodes[indx]; }
	inline s_Node* getNd(long indx) { return this->nodes[indx]; }
	inline s_Node** getNodes() { return this->nodes; }

	s_Node** nodes;
	long    N;
protected:
	long N_mem;
	virtual void reset();
};
class s_HexPlate : public s_Plate{
public:
	s_HexPlate();
	~s_HexPlate();

	unsigned char         init(long nNodes);
	void                  initRs(float inRhex);
	void                  release();/*assumes that the plate owns its subnodes if not NULL*/

	inline void set(long indx, s_Hex* nd) { this->nodes[indx] = (s_Node*)nd; }
	inline void set(long indx, s_Hex& nd) { this->set(indx,&nd); }
	void setWeb(long index, int web_i, long target_i);
	inline s_Hex* get(long indx) { return (s_Hex*)this->nodes[indx]; }
	inline s_Hex  getCopy(long indx) { return *((s_Hex*)this->nodes[indx]); }
	bool inHex(const long hexNode_i, const s_2pt& pt, const float padding = 0.f) const;

	long height;
	long width;
	float Rhex;
	float RShex;
	float Shex;
	s_2pt hexU[6];
protected:
	virtual void reset();
	void genHexU_0();
};
namespace n_HexPlate {
	s_Hex* rotateCLK(const s_Hex* hexNode, const int start_web_i);
	s_Hex* rotateCCLK(const s_Hex* hexNode, const int start_web_i);
}
class s_HexBasePlate : public s_HexPlate {
public:
	s_HexBasePlate();
	~s_HexBasePlate();

	unsigned char initRowStart(long rowN);
	void          releaseRowStart();

	long  N_wHex;
	long  N_hHex;

	long xyToHexi(const s_2pt& xy);
	/*for fast scan xy to plate loc for square plate configuration*/
	s_2pt* RowStart; /*xy position of the first hex in each row*/
	s_2pt_i* RowStart_is;/* index in the hex array of the row start, and number in row*/
	long   Row_N;
	float  Col_d;/*seperation between columns 2/3 * R */
	float  Row_d;/*seperation between rows 2*RS */
protected:
	void reset();
};

class s_HexPlateLayer {
	s_HexPlate** p;
	int N;
protected:
	int N_mem;
};
#endif