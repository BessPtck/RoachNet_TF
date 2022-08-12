#pragma once
#ifndef HEXPLATES_H
#define HEXPLATES_H
#ifndef STRUCTS_H
#include "../Base/Structs.h"
#endif

/*class owns the hex plates that are directly connected to the image geometry
  class also containes the untilites used to do most plate-to-plate manipluations
  original generation of square plates is done by HexImg*/

class HexPlates : public Base{
public:
protected:
	/*owned*/
	/* plates below are all at lowest resolution */
	s_HexBasePlate*   m_pImg;/*lowest plate the one that HexImg generates */
	s_HexBasePlate**  m_pCol;/*col plates with cols of interest selected number of plates corresponding to num of colors */
	s_HexBasePlate**  m_pLuna;/* max lunas for all col plates should have 8 plates corresponding to number of lunas*/
	/*                                            */
	/* plates are at L1 resolution about half of L0 resolution*/
	s_HexBasePlate** m_pL1;/* results of L0 network stamps run on the highest luna plates, with the highest value stamp
							      selected from the 7 lower stamps (pooling) reducing the resolution by about a factor of 2
								  there will be 74? or so stamps corresponding to different arc sizes and angles */
	/* plates are at L2 resolution about half of L1 resolution */
	s_HexBasePlate** m_pL2;
	/* plates are at L3 resolution about half of L2 resolution */
	s_HexBasePlate** m_pL3; 
	/* plates resolution is less than L3 resolution by generally MORE than half */
	s_HexBasePlate** m_pObj;/*plate looks for overall structure with more than 7 L3 nodes at its base*/
};

#endif