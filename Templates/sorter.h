/*	Copyright  (c)	Günter Woigk 1998 - 2020
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.



	Threshold Sorter (alternating buffer variant)		adaptable via #defines
	---------------------------------------------		----------------------

<German>
	Der zu sortierende Bereich wird in drei Felder unterteilt:
		1. vorsortierte Elemente unterhalb (<=) einer unteren Schranke
		2. noch unsortierte Elemente
		3. vorsortierte Elemente oberhalb (>=) einer oberen Schranke

	Start:
		Falls 1. Element > letztes Element  =>  diese beiden vertauschen
		Feld 1 := 1. Element
		Feld 3 := letztes Element
		Untere Schranke := 1. Element
		Obere Schranke  := letztes Element

	Danach wird abwechselnd das erste und das letzte Element des mittleren, noch
	unsortierten Feldes betrachtet:
		Element[i] <= untere Grenze => unterem Bereich zuordnen
		Element[i] >= obere Grenze  => oberem Bereich zuordnen
		Element[i] dazwischen =>
			dem unteren bzw. oberen Bereich zuordnen und Grenzwert entsprechend neu setzen

		Dabei wird dem Bereich der Vorrang gegeben, an den Element[i] grenzt:
		Wird also gerade das erste Element des mittleren Bereiches betrachtet,
		wird es vorzugsweise dem unteren sortierten Feld zugeschlagen, sonst anders rum.

		Damit muss das Element gar nicht verschoben/vertauscht werden und durch das
		alternieren wird die Wahrscheinlichkeit für eine symmetrische Größe der vorsortierten
		Bereiche erhöht. (sie ist aber leider nicht besonders hoch.)

	Ist der gesamte Bereich vorsortiert, also das mittlere Feld leer, werden die beiden
	vorsortierten Felder rekursiv nach gleichem Schema sortiert.


	Vorteile:
		nicht quadratischer Zeitzuwachs
		benötigt keinen temp array
		nur das Vergleichs-Makro CMP(A,B) muss implementierbar sein
		(man kann damit also alles sortieren, was überhaupt sortierbar ist.)

	Nachteil:
		Die Reihgenfolge von Elementen mit gleichem Wert wird verwürfelt
		(also ungeeignet für's Sortieren von Datenbanken nach 2 oder mehr Spalten)
</German>
*/


#ifndef SWAP
 #define SWAP std::swap
#endif

#ifndef GT
 #define GT gt
#endif


// template<class TYPE> void sort(TYPE* a, TYPE* e)
{
	assert(a && e);							// both pointers not NULL
	assert(a + (e-a) == e);					// check alignment

	if(a>=e) return;

	// ToDo-Stack:
	TYPE* a_stack[sizeof(void*)<<3];
	TYPE* e_stack[sizeof(void*)<<3];
	int pushed = 0;

	e--;	// use [a .. e] instead of [a .. [e

	for(;;)
	{
		assert(a<=e);

		switch(e-a)
		{
		case 2: // 3 items
			if(GT(a[0],a[2])) SWAP(a[0],a[2]);
			if(GT(a[1],a[2])) SWAP(a[1],a[2]);
			/* fall through */

		case 1: // 2 items
			if(GT(a[0],a[1])) SWAP(a[0],a[1]);
			/* fall through */

		case 0: // 1 item
			if(pushed)
			{
				pushed--;
				a = a_stack[pushed];
				e = e_stack[pushed];
				continue;
			}
			return;
		}

	// 4 or more elements

		TYPE* a0 = a;
		TYPE* e0 = e;

		if(GT(*a,*e)) SWAP(*a,*e);

		TYPE* a_lim = a++;
		TYPE* e_lim = e--;

		for(;;)
		{
			if(a>=e) break;

			if(GT(*a,*a_lim))
				if(GT(*e_lim,*a))
					a_lim = a++;
				else
					SWAP(*a,*e), e--;
			else
				a++;

			if(a>=e) break;

			if(GT(*e_lim,*e))
				if (GT(*e,*a_lim))
					e_lim = e--;
				else
					SWAP(*a,*e), a++;
			else
				e--;
		}

	// letztes Element unten oder oben zuschlagen.
	// wäre unnötig, wenn in der schleife oben das abbruchkriterium (a>b) verwendet würde.
	// dann würde das letzte Element aber häufig mit sich selbst geswappt, was stören kann.
		if(GT(*a,*a_lim)) e--; else a++;

	// größeren der subbereiche [a0..e] und [a..e0] pushen, den anderen sofort sortieren
	// das ist nötig, um stacküberlauf sicher zu vermeiden
		xassert(pushed < int(NELEM(a_stack)));
		if(e-a0 > e0-a)
		{
			a_stack[pushed] = a0;
			e_stack[pushed] = e;
			pushed++;

			a = e;
			e = e0;
		}
		else
		{
			a_stack[pushed] = a;
			e_stack[pushed] = e0;
			pushed++;

			a = a0;
		  //e = e;
		}
	}
}

#undef SWAP
#undef GT

