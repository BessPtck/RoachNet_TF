#include "Base.h"
Base::Base() { ; }
Base::~Base() { ; }

void Base::Zero_Memory(void* mem_ptr, int byte_size) {
	uint8_t* byte_ptr = (uint8_t*)mem_ptr;
	uint8_t null_byte = 0x00;
	for (int mem_i = 0; mem_i < byte_size; mem_i++)
		byte_ptr[mem_i] = null_byte;
}
