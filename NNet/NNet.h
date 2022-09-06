#pragma once
#ifndef NNET_H
#define NNET_H

#ifndef SNET_H
#include "../Nets/sNet.h"
#endif

using namespace std;
class NNet : public Base {
public:
	NNet();
	~NNet();

	unsigned char init();
	void          release();

	unsigned char spawn(s_CNnets* nets);
	void          despawn(s_CNnets* nets);

	unsigned char setFile(string& inf);
	unsigned char importFile(s_CNnets* nets);/* is run during spawn if file name already set, else can be run after spawn
											    import the weight and b values from the file in the nets of CNnets*/
protected:
	/*owned helper objects*/
	sNet* m_NetGen;
	HexEye* m_HexEyeGen;
	/*                   */
	string m_file;

	int    m_N_nets;
	int    m_N_lev;/*number of levels of the nets and the eyes*/
	int    m_N_lev_trigger;/*if this is greater than 0 then there is a trigger node to be set*/
};

namespace n_NNet {
	bool run(s_CNnets* nets, s_HexBasePlateLayer* platesIn, s_HexBasePlateLayer* platesOut, long plate_index);/* assumes all platesIn have the same geometery*/

	/** helpers to run **/
	bool rootNets(s_CNnets* nets, s_HexBasePlate* plateIn, long plate_index);
	bool runRootedNets(s_CNnets* nets);
}
#endif
