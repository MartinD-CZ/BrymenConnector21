#ifndef DECODER_H_
#define DECODER_H_

#include <stdint.h>



namespace decoder
{
	bool receiveMessage(void);
	void processMessage(void);
	int8_t decodeDigit(uint8_t source);
}


#endif /* DECODER_H_ */
