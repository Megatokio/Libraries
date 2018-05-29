/* (N)compress42.c - File compression ala IEEE Computer, Mar 1992.
 *
 * Authors:
 *   Spencer W. Thomas   (decvax!harpo!utah-cs!utah-gr!thomas)
 *   Jim McKie           (decvax!mcvax!jim)
 *   Steve Davies        (decvax!vax135!petsd!peora!srd)
 *   Ken Turkowski       (decvax!decwrl!turtlevax!ken)
 *   James A. Woods      (decvax!ihnp4!ames!jaw)
 *   Joe Orost           (decvax!vax135!petsd!joe)
 *   Dave Mack           (csu@alembic.acs.com)
 *   Peter Jannesen, Network Communication Systems
 *                       (peter@ncs.nl)
 *
 * Revision 4.2.3  92/03/14 peter@ncs.nl
 * Revision 4.1  91/05/26 csu@alembic.acs.com
 * Revision 4.0  85/07/30  12:50:00  joe
 * Revision 3.6  85/07/04  01:22:21  joe
 * Revision 3.5  85/06/30  20:47:21  jaw
 * Revision 3.4  85/06/27  12:00:00  ken
 * Revision 3.3  85/06/24  21:53:24  joe
 * Revision 3.2  85/06/06  21:53:24  jaw
 * Revision 3.1  85/05/12  18:56:13  jaw
 * Revision 3.0   84/11/27  11:50:00  petsd!joe
 * Revision 2.7   84/11/16  19:35:39  ames!jaw
 * Revision 2.6   84/11/05  19:18:21  ames!jaw
 * Revision 2.5   84/10/30  20:15:14  ames!jaw
 * Revision 2.4   84/10/16  11:11:11  ames!jaw
 * Revision 2.3   84/09/22  22:00:00  petsd!joe
 * Revision 2.2   84/09/18  14:12:21  ames!jaw
 * Revision 2.1   84/09/10  12:34:56  ames!jaw
 * Revision 2.0   84/08/28  22:00:00  petsd!joe
 * Revision 1.10  84/08/24  18:28:00  turtlevax!ken
 * Revision 1.9  84/08/16  17:28:00  turtlevax!ken
 * Revision 1.8  84/08/09  23:15:00  joe
 * Revision 1.6  84/08/01  22:08:00  joe
 * Revision 1.5  84/07/13  13:11:00  srd
 * Revision 1.4  84/07/05  03:11:11  thomas
 * Revision 1.3  84/07/05  02:06:54  thomas
 * Revision 1.2  84/07/05  00:27:27  thomas
 */


#include "../kio/kio.h"
#include "FD.h"

#undef	min
#define	min(a,b)	((a>b) ? b : a)

#ifndef	IBUFSIZ
#define	IBUFSIZ	BUFSIZ				/* Defailt input buffer size					*/
#endif
#ifndef	OBUFSIZ
#define	OBUFSIZ	BUFSIZ				/* Default output buffer size					*/
#endif


									/* Defines for third byte of header 			*/
#define	MAGIC_1		uint8('\037')	/* First byte of compressed file				*/
#define	MAGIC_2		uint8('\235')	/* Second byte of compressed file				*/
#define BIT_MASK	0x1f			/* Mask for 'number of compresssion bits'		*/
									/* Masks 0x20 and 0x40 are free.  				*/
									/* I think 0x20 should mean that there is		*/
									/* a fourth header byte (for expansion).    	*/
#define BLOCK_MODE	0x80			/* Block compresssion if table is full and		*/
									/* compression rate is dropping flush tables	*/

#define FIRST	257					/* first free entry 							*/
#define	CLEAR	256					/* table clear output code 						*/
#define INIT_BITS 9					/* initial number of bits/code					*/


// the following is hopefully the right way:
#ifdef _BIG_ENDIAN
#define	BYTEORDER	4321
#endif
#ifdef _LITTLE_ENDIAN
#define BYTEORDER 1234
#endif
#if _ALIGNMENT_REQUIRED
#define	NOALLIGN	0
#else
#define	NOALLIGN	1
#endif


#define	HBITS		17			/* 50% occupancy */
#define	HSIZE	   (1<<HBITS)
#define	HMASK	   (HSIZE-1)
#define	HPRIME		9941
#define	BITS		16				// max. allowed value: 16
#define CHECK_GAP	10000


#define MAXCODE(n)	(1L << (n))



union bytes
{
	int32	word;
	struct
	{
#if BYTEORDER == 4321
		uint8	b1;
		uint8	b2;
		uint8	b3;
		uint8	b4;
#elif BYTEORDER == 1234
		uint8	b4;
		uint8	b3;
		uint8	b2;
		uint8	b1;
#else
#error fixme
#endif
	} bytes;
};


#if BYTEORDER == 4321 && NOALLIGN == 1
#define	output(b,o,c,n)	{													\
							*(int32*)&((b)[(o)>>3]) |= (int32(c))<<((o)&0x7);\
							(o) += (n);										\
						}
#else
#define	output(b,o,c,n)	{	uint8	*p = &(b)[(o)>>3];						\
							union bytes i;									\
							i.word = (int32(c))<<((o)&0x7);					\
							p[0] |= i.bytes.b1;								\
							p[1] |= i.bytes.b2;								\
							p[2] |= i.bytes.b3;								\
							(o) += (n);										\
						}
#endif

#if BYTEORDER == 4321 && NOALLIGN == 1
#define	input(b,o,c,n,m){													\
							(c) = (*(int32 *)(&(b)[(o)>>3])>>((o)&0x7))&(m);\
							(o) += (n);										\
						}
#else
#define	input(b,o,c,n,m){	uint8* p = &(b)[(o)>>3];						\
							(c) = (((int32(p[0]))|(int32(p[1])<<8)|			\
								    (int32(p[2])<<16))>>((o)&0x7))&(m);		\
							(o) += (n);										\
						}
#endif


int			block_mode = BLOCK_MODE;/* Block compress mode -C compatible with 2.0*/
int			maxbits = BITS;		/* user settable max # bits/code 				*/
int 		zcat_flg = 0;		/* Write output on stdout, suppress messages 	*/
int			recursive = 0;  	/* compress directories 						*/
int			exit_code = -1;		/* Exitcode of compress (-1 no file compressed)	*/

uint8		inbuf[IBUFSIZ+64];	/* Input buffer									*/
uint8		outbuf[OBUFSIZ+2048];/* Output buffer								*/

char		*ifname;			/* Input filename								*/

int32 		bytes_in;			/* Total number of byte from input				*/
int32 		bytes_out;			/* Total number of byte to output				*/

int32		htab[HSIZE];
uint16		codetab[HSIZE];


#define	htabof(i)				htab[i]
#define	codetabof(i)			codetab[i]
#define	tab_prefixof(i)			codetabof(i)
#define	tab_suffixof(i)			((uint8 *)(htab))[i]
#define	de_stack				((uint8 *)&(htab[HSIZE-1]))
#define	clear_htab()			memset(htab, -1, sizeof(htab))
#define	clear_tab_prefixof()	memset(codetab, 0, 256);


static int primetab[256] =		/* Special secudary hash table.		*/
{
	 1013, -1061, 1109, -1181, 1231, -1291, 1361, -1429,
	 1481, -1531, 1583, -1627, 1699, -1759, 1831, -1889,
	 1973, -2017, 2083, -2137, 2213, -2273, 2339, -2383,
	 2441, -2531, 2593, -2663, 2707, -2753, 2819, -2887,
	 2957, -3023, 3089, -3181, 3251, -3313, 3361, -3449,
	 3511, -3557, 3617, -3677, 3739, -3821, 3881, -3931,
	 4013, -4079, 4139, -4219, 4271, -4349, 4423, -4493,
	 4561, -4639, 4691, -4783, 4831, -4931, 4973, -5023,
	 5101, -5179, 5261, -5333, 5413, -5471, 5521, -5591,
	 5659, -5737, 5807, -5857, 5923, -6029, 6089, -6151,
	 6221, -6287, 6343, -6397, 6491, -6571, 6659, -6709,
	 6791, -6857, 6917, -6983, 7043, -7129, 7213, -7297,
	 7369, -7477, 7529, -7577, 7643, -7703, 7789, -7873,
	 7933, -8017, 8093, -8171, 8237, -8297, 8387, -8461,
	 8543, -8627, 8689, -8741, 8819, -8867, 8963, -9029,
	 9109, -9181, 9241, -9323, 9397, -9439, 9511, -9613,
	 9677, -9743, 9811, -9871, 9941,-10061,10111,-10177,
	10259,-10321,10399,-10477,10567,-10639,10711,-10789,
	10867,-10949,11047,-11113,11173,-11261,11329,-11423,
	11491,-11587,11681,-11777,11827,-11903,11959,-12041,
	12109,-12197,12263,-12343,12413,-12487,12541,-12611,
	12671,-12757,12829,-12917,12979,-13043,13127,-13187,
	13291,-13367,13451,-13523,13619,-13691,13751,-13829,
	13901,-13967,14057,-14153,14249,-14341,14419,-14489,
	14557,-14633,14717,-14767,14831,-14897,14983,-15083,
	15149,-15233,15289,-15359,15427,-15497,15583,-15649,
	15733,-15791,15881,-15937,16057,-16097,16189,-16267,
	16363,-16447,16529,-16619,16691,-16763,16879,-16937,
	17021,-17093,17183,-17257,17341,-17401,17477,-17551,
	17623,-17713,17791,-17891,17957,-18041,18097,-18169,
	18233,-18307,18379,-18451,18523,-18637,18731,-18803,
	18919,-19031,19121,-19211,19273,-19381,19429,-19477
} ;



void abort_compress()
{
	exit(1);
}

void read_error()
{
	fprintf(stderr, "\nread error");
	exit(1);
}

void write_error()
{
	fprintf(stderr, "\nwrite error");
	exit(1);
}



/*
 * compress fdin to fdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */
void compress(int fdin, int fdout)
{
	int32	hp;
	int		rpos;
//	int32	fc;
	int		outbits;
	int		rlop;
	int		rsize;
	int		stcode;
	int32	free_ent;
	int		boff;
	int		n_bits;
	int		ratio;
	int32	checkpoint;
	int32	extcode;

	union
	{
		int32		code;
		struct
		{
			uint8	c;
			uint16	ent;
		} e;
	} fcode;

	ratio = 0;
	checkpoint = CHECK_GAP;
	extcode = MAXCODE(n_bits = INIT_BITS)+1;
	stcode = 1;
	free_ent = FIRST;

	memset(outbuf, 0, sizeof(outbuf));
	bytes_out = 0; bytes_in = 0;
	outbuf[0] = MAGIC_1;
	outbuf[1] = MAGIC_2;
	outbuf[2] = (char)(maxbits | block_mode);
	boff = outbits = (3<<3);
	fcode.code = 0;

	clear_htab();

	while ((rsize = read(fdin, inbuf, IBUFSIZ)) > 0)
	{
		if (bytes_in == 0)
		{
			fcode.e.ent = inbuf[0];
			rpos = 1;
		}
		else
			rpos = 0;

		rlop = 0;

		do
		{
			if (free_ent >= extcode && fcode.e.ent < FIRST)
			{
				if (n_bits < maxbits)
				{
					boff = outbits = (outbits-1)+((n_bits<<3)-
								((outbits-boff-1+(n_bits<<3))%(n_bits<<3)));
					if (++n_bits < maxbits)
						extcode = MAXCODE(n_bits)+1;
					else
						extcode = MAXCODE(n_bits);
				}
				else
				{
					extcode = MAXCODE(16)+OBUFSIZ;
					stcode = 0;
				}
			}

			if (!stcode && bytes_in >= checkpoint && fcode.e.ent < FIRST)
			{
				int32 rat;

				checkpoint = bytes_in + CHECK_GAP;

				if (bytes_in > 0x007fffff)
				{							/* shift will overflow */
					rat = (bytes_out+(outbits>>3)) >> 8;

					if (rat == 0)				/* Don't divide by zero */
						rat = 0x7fffffff;
					else
						rat = bytes_in / rat;
				}
				else
					rat = (bytes_in << 8) / (bytes_out+(outbits>>3));	/* 8 fractional bits */
				if (rat >= ratio)
					ratio = int(rat);
				else
				{
					ratio = 0;
					clear_htab();
					output(outbuf,outbits,CLEAR,n_bits);
					boff = outbits = (outbits-1)+((n_bits<<3)-
								((outbits-boff-1+(n_bits<<3))%(n_bits<<3)));
					extcode = MAXCODE(n_bits = INIT_BITS)+1;
					free_ent = FIRST;
					stcode = 1;
				}
			}

			if (outbits >= (OBUFSIZ<<3))
			{
				if (write(fdout, outbuf, OBUFSIZ) != OBUFSIZ)
					write_error();

				outbits -= (OBUFSIZ<<3);
				boff = -(((OBUFSIZ<<3)-boff)%(n_bits<<3));
				bytes_out += OBUFSIZ;

				memcpy(outbuf, outbuf+OBUFSIZ, (outbits>>3)+1);
				memset(outbuf+(outbits>>3)+1, '\0', OBUFSIZ);
			}

			{
				int i;

				i = rsize-rlop;

				if (int32(i) > extcode-free_ent)	i = int(extcode-free_ent);
				if (i > ((int(sizeof(outbuf)) - 32)*8 - outbits)/n_bits)
					i = ((int(sizeof(outbuf)) - 32)*8 - outbits)/n_bits;

				if (!stcode && int32(i) > checkpoint-bytes_in)
					i = int(checkpoint-bytes_in);

				rlop += i;
				bytes_in += i;
			}

			goto next;
hfound:			fcode.e.ent = codetabof(hp);
next:  			if (rpos >= rlop)
				goto endlop;
next2: 			fcode.e.c = inbuf[rpos++];
			{
				int32	i;
				int32	p;
				//fc = fcode.code;
				#define fc fcode.code
				hp = (((int32(fcode.e.c)) << (HBITS-8)) ^ int32(fcode.e.ent));

				if ((i = htabof(hp)) == fc)	goto hfound;
				if (i == -1)				goto out;

				p = primetab[fcode.e.c];
lookup:				hp = (hp+p)&HMASK;
				if ((i = htabof(hp)) == fc)	goto hfound;
				if (i == -1)				goto out;
				hp = (hp+p)&HMASK;
				if ((i = htabof(hp)) == fc)	goto hfound;
				if (i == -1)				goto out;
				hp = (hp+p)&HMASK;
				if ((i = htabof(hp)) == fc)	goto hfound;
				if (i == -1)				goto out;
				goto lookup;
			}
out:			;
			output(outbuf,outbits,fcode.e.ent,n_bits);

			{
#undef	fc
				int32	fc;
				fc = fcode.code;
				fcode.e.ent = fcode.e.c;


				if (stcode)
				{
					codetabof(hp) = (uint16)free_ent++;
					htabof(hp) = fc;
				}
			}

			goto next;

endlop:			if (fcode.e.ent >= FIRST && rpos < rsize)
				goto next2;

			if (rpos > rlop)
			{
				bytes_in += rpos-rlop;
				rlop = rpos;
			}
		}
		while (rlop < rsize);
	}

	if (rsize < 0)
		read_error();

	if (bytes_in > 0)
		output(outbuf,outbits,fcode.e.ent,n_bits);

	if (write(fdout, outbuf, (outbits+7)>>3) != (outbits+7)>>3)
		write_error();

	bytes_out += (outbits+7)>>3;

	return;
}


/*
 * Decompress stdin to stdout.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.  The tables used herein are shared
 * with those of the compress() routine.  See the definitions above.
 */
void decompress(cstr qfilepath, cstr zfilepath) THF
{
	int32	code;
	int		finchar = 0;
	int32	oldcode = -1;
	int		inbits;
	int		posbits = 3<<3;
	int		outpos = 0;
	int		insize = 0;
	int		n_bits = INIT_BITS;
	int32	maxcode = MAXCODE(n_bits)-1;
	int		bitmask = (1<<n_bits)-1;
	int		rsize;

	FD q(qfilepath,'r');
	while(insize < 3 && (rsize = q.read_bytes(inbuf+insize,IBUFSIZ,1)) > 0) { insize += rsize; }
	if(insize < 3 || inbuf[0] != MAGIC_1 || inbuf[1] != MAGIC_2) throw file_error(q,customerror,"not in a compressed format");

	int maxbits      = inbuf[2] & BIT_MASK;
	if(maxbits > BITS) throw file_error(q,customerror,usingstr("corrupted data: claims %i bits",(int)maxbits));
	int block_mode   = inbuf[2] & BLOCK_MODE;
	int32 maxmaxcode = MAXCODE(maxbits);
	int32 free_ent   = ((block_mode) ? FIRST : 256);

	clear_tab_prefixof();
	for(code = 255; code >= 0; --code) { tab_suffixof(code) = uint8(code); }

	FD z(zfilepath,'w');

loop:
	{
		int o = posbits >> 3;
		int e = o <= insize ? insize - o : 0;
		for(int i = 0 ; i < e ; ++i) inbuf[i] = inbuf[i+o];
		insize = e;
		posbits = 0;
	}

	if (insize < (int)sizeof(inbuf)-IBUFSIZ)
	{
		rsize = q.read_bytes(inbuf+insize, IBUFSIZ,1);
		insize += rsize;
	}

	inbits = rsize > 0 ? (insize - insize%n_bits)<<3 : (insize<<3)-(n_bits-1);

	while(inbits > posbits)
	{
		if(free_ent > maxcode)
		{
			posbits = ((posbits-1) + ((n_bits<<3) - (posbits-1+(n_bits<<3))%(n_bits<<3)));

			++n_bits;
			maxcode = n_bits == maxbits ? maxmaxcode : MAXCODE(n_bits)-1;
			bitmask = (1<<n_bits)-1;
			goto loop;
		}

		input(inbuf,posbits,code,n_bits,bitmask);

		if(oldcode == -1)
		{
			if(code >= 256) throw file_error(q,customerror,usingstr("corrupted data: oldcode:-1 code:%i",int(code)));
			outbuf[outpos++] = uint8(finchar = (int)(oldcode = code));
			continue;
		}

		if(code == CLEAR && block_mode)
		{
			clear_tab_prefixof();
			free_ent = FIRST - 1;
			posbits = ((posbits-1) + ((n_bits<<3) - (posbits-1+(n_bits<<3))%(n_bits<<3)));
			maxcode = MAXCODE(n_bits = INIT_BITS)-1;
			bitmask = (1<<n_bits)-1;
			goto loop;
		}

		int32 incode = code;
		uint8* stackp = de_stack;

		if(code >= free_ent)				// Special case for KwKwK string
		{
			if(code > free_ent) throw file_error(q, customerror, "corrupted data: code > free_ent");
			*--stackp = uint8(finchar);
			code = oldcode;
		}

		while(code >= 256)					// Generate output characters in reverse order
		{
			*--stackp = tab_suffixof(code);
			code = tab_prefixof(code);
		}
		*--stackp =	uint8(finchar = tab_suffixof(code));

		// And put them out in forward order
		{
			int	i;

			if(outpos+(i = (de_stack-stackp)) >= OBUFSIZ)
			{
				do
				{
					if(i > OBUFSIZ-outpos) i = OBUFSIZ-outpos;
					if(i) { memcpy(outbuf+outpos, stackp, i); outpos += i; }
					if(outpos >= OBUFSIZ) { z.write_bytes(outbuf, outpos); outpos = 0; }
					stackp += i;
				}
				while((i = (de_stack-stackp)) > 0);
			}
			else
			{
				memcpy(outbuf+outpos, stackp, i);
				outpos += i;
			}
		}

		if((code = free_ent) < maxmaxcode)	// Generate new entry
		{
			tab_prefixof(code) = uint16(oldcode);
			tab_suffixof(code) = uint8(finchar);
			free_ent = code+1;
		}

		oldcode = incode;
	}

	if(rsize) goto loop;

	if(outpos) z.write_bytes(outbuf, outpos);
}















































