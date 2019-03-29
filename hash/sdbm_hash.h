#pragma once

typedef unsigned int uint;

inline uint sdbm_hash( const char* key )
{
	// Calculate hash for key
	//		note: this is the sdbm algorithm:
	//		hash(i) = hash(i-1) * 65599 + str[i];
	//	see: http://www.cse.yorku.ca/~oz/hash.html
	//	https://programmers.stackexchange.com/questions/49550

	uint hash = 0;
	int c;
	while((c=*key++))
	{
		hash = (hash << 6) + (hash << 16) - hash + uint(c);	// meistens eine Winzigkeit schneller als Multiplikation
	}

//	return hash;					// original algorithm
	return hash ^ (hash>>16);		// do some xor'ing to better support smaller required hash sizes
}


inline uint sdbm_hash( const char* bptr, uint size )
{
	// Calculate hash for buffer

	uint hash = 0;
	while(size--)
	{
		int c = *bptr++;
		hash = (hash << 6) + (hash << 16) - hash + uint(c);	// meistens eine Winzigkeit schneller als Multiplikation
	}

//	return hash;					// original algorithm
	return hash ^ (hash>>16);		// do some xor'ing to better support smaller required hash sizes
}











