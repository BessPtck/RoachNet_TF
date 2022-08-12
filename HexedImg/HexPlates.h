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
	s_HexBasePlate* m_pImg;/*lowest plate the one that HexImg generates */
	
};

#endif