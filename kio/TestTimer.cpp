/*	Copyright  (c)	Günter Woigk 2015 - 2019
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/


#include "kio/kio.h"
#include "TestTimer.h"
#include "cstrings/cstrings.h"


TestTimer::TestTimer()
{
	_start = now();
}


void TestTimer::start()
{
	_start = now();
}


void TestTimer::test(double maxdelay, cstr sourceposition)
{
	double end   = now();
	double delay = end-_start;
	_start = end;

	if(delay > maxdelay)
	{
		if (maxdelay >= 1.0) logline("TT: %s took %.3f sec", sourceposition, delay);
		else if (maxdelay >= 1e-3) logline("TT: %s took %.3f msec", sourceposition, delay*1e3);
		else logline("TT: %s took %.3f µsec", sourceposition, delay*1e6);
	}
}


