


#define POLY 0x8408

/*	                                     16   12   5
	this is the CCITT CRC 16 polynomial X  + X  + X  + 1.
	This works out to be 0x1021, but the way the algorithm works
	lets us use 0x8408 (the reverse of the bit pattern).  The high
	bit is always assumed to be set, thus we only use 16 bits to
	represent the 17 bit value.
*/



unsigned short crc16( const unsigned char* q, unsigned int count )
{
	unsigned int data, crc = 0xffff;

	while(count--)
	{
		for( data = 0x0100 + *q++; data>1; data >>= 1 )
		{
			crc = (crc^data) & 1 ? (crc >> 1) ^ POLY : crc >> 1;
		}
	}
	
	crc = ~crc;
	return (crc << 8) | (crc >> 8 & 0xff);
}
