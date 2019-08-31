#pragma once
/*	Copyright  (c)	Günter Woigk 2007 - 2019
					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	template class GifArray
	--------------------

	Completely reworked:	2007-06-09 kio !


	Manage a dynamically allocated array of type T

	-	delete[] data when GifArray is destroyed
	-	index validation in operator[]
	-	resizing


	Restrictions:

	-	max. number of elements in array = max(int)
		byte-size of array is not restricted!

	-	data type T must have 'flat' default creator, destructor, and copy creator:

		-	default creator:	data is cleared to 0
		-	destructor:			no action
		-	copy creator:		flat copy
*/

#include "../kio/kio.h"
#include "../unix/FD.h"

typedef class std::bad_alloc bad_alloc;


template<class T>
class GifArray
{
public:
/*	creator, destructor:
			GifArray()				default constructor: empty array
			GifArray(GifArray&)		copy constructor
			GifArray(T*,count)		copy caller-supplied memory
			GifArray(count)			array with malloced memory, cleared to 0
			~GifArray()				destructor
			operator=(GifArray&)	copy data
			operator=(T)			create array with one item. deprecated.
*/
			GifArray		( )			noexcept	{ init(); }

			GifArray		( GifArray const& q )	throws/*bad alloc*/	{ init(q); }
			GifArray		( GifArray const* q )	throws/*bad alloc*/	{ init(*q); }
			GifArray		( T const*q, int n )	throws/*bad alloc*/	{ init(q,n); }
			GifArray		( int n )				throws/*bad alloc*/	{ init(n); }

			~GifArray		( )			noexcept	{ kill();  }

	GifArray&	operator=	( const GifArray& q )	throws/*bad alloc*/	{ if( this!=&q ) { kill(); init(q); } return *this; }
	GifArray&	operator=	( T q )					throws/*bad alloc*/	{ kill(); init(&q,1); return *this;  }


/*	access data members:
			Size()		get item count
			Data()		get array base address
			operator[i]	access item i. checks index during development.
*/
	int		Count		( )			const noexcept	{ return count; }
	T*		Data		( )				  noexcept	{ return array; }
	T const*Data		( )			const noexcept	{ return array; }
	T&		operator[]	( int i )		  noexcept	{ assert(uint(i) < uint(count)); return array[i]; }
	T const&operator[]	( int i )	const noexcept	{ assert(uint(i) < uint(count)); return array[i]; }


/*	resizing and clearing:
			Shrink		shrink if new_count < count
			Grow		grow   if new_count > count
			Purge		purge data and resize to 0
			Clear		overwrite array with 0s.
			Clear(T)	overwrite array with Ts.
*/
	void	Shrink		( int new_count )	noexcept;
	void	Grow		( int new_count )	throws/*bad alloc*/;
	void	Purge		( )					noexcept	{ kill(); init(); }
	void	Clear		( )					noexcept	{ clear( array, count ); }
	void	Clear		( T q )				noexcept	{ T* p=array; T* e=p+count; while(p<e) *p++=q; }

/*	concatenation:
	Append		append user-supplied data to this array. if bad_alloc is thrown, then GifArray remained unmodified.
	operator+=	append other array to my array. if bad_alloc is thrown, then GifArray remained unmodified.
	operator=	return concatenation of this array and other array. throws bad_alloc if malloc fails.
*/
	GifArray&	Append		( T const* q, int n ) throws/*bad alloc*/;
	GifArray&	operator+=	( GifArray const& q ) throws/*bad alloc*/	{ return Append(q.array,q.count); }
	GifArray	operator+	( GifArray const& q ) const throws/*bad alloc*/ { return GifArray(this).Append(q.array,q.count); }


private:
/*	static methods:
*/
  static size_t	sz			( size_t n )				noexcept		{ return n*sizeof(T); }		// convert item count to memory size
  static void	copy		( T* z, T const* q, int n ) noexcept		{ memcpy(z,q,sz(n)); }		// flat copy n items
  static void	clear		( T* z, int n )				noexcept		{ memset(z,0,sz(n)); }		// clear n items with 0
  static T*		malloc		( int n )			  throws/*bad alloc*/	{ return n?new T[n]:NULL; }	// allocate memory for n items
  static T*		newcopy		( T const* q, int n ) throws/*bad alloc*/	{ if(!n) return 0; T* z = new T[n]; copy(z,q,n); return z; }
  static T*		newcleared	( int n )			  throws/*bad alloc*/	{ if(!n) return 0; T* z = new T[n]; clear(z,n); return z; }
  static void	release		( T* array )				noexcept		{ delete[] array; array=0; }

/*	init and kill:
*/
  void	init	()					  noexcept	{ array=0; count=0; }						// init empty array
  void	init	( int n )			  throws/*bad alloc*/		{ array=0; count=n; array=newcleared(n); }	// alloc and clear array
  void	init	( T const* q, int n ) throws/*bad alloc*/		{ array=0; count=n; array=newcopy(q,n);  }	// init with copy of caller-supplied data
  void	init	( GifArray const& q ) throws/*bad alloc*/		{ init(q.array,q.count); }					// init with copy
  void	kill	()					  noexcept	{ release(array); }							// deallocate allocateded data


/*	data members:
*/
protected:
	T*		array;		// -> array[0].	array *may* be NULL for empty array.
	int		count;		// item count.	count==0 for empty array.
};



template<class T>		/* verified 2006-06-09 kio */
GifArray<T>&
GifArray<T>::Append ( T const* q, int n ) throws/*bad alloc*/
{
/*	Append caller-supplied items to my array.
	does nothing if item count == 0.
	can be called to append subarray of own array!
	if bad_alloc is thrown, then this GifArray remained unmodified.
*/
	if(n>0)
	{
		T*  old_array = array;
		int old_count = count;
		array = malloc(old_count+n);
		count = old_count + n;

		copy( array,old_array,old_count );	// copy own data
		copy( array+old_count, q, n );		// copy other data. may be subarray of my own array!
		release(old_array);					// => release own data after copying other data!
	}
	return *this;
}



template<class T>		/* verified 2006-06-09 kio */
void
GifArray<T>::Shrink ( int new_count ) noexcept
{
/*	Shrink GifArray:
	does nothing if requested new_count >= current count
	purges ( deallocates ) data if new_count == 0.
	silently handles new_count < 0 as new_count == 0.
	silently keeps the old data if data reallocation fails.
*/
	if( new_count>=count ) return;			// won't shrink
	if( new_count<=0 ) { Purge(); return; }	// empty array

	count = new_count;
	try							// try to reallocate data
	{
		T* old_array = array;
		array = newcopy( old_array, new_count );
		release(old_array);
	}
	catch(bad_alloc&){}		// but just don't do it if reallocation fails
}


template<class T>		/* verified 2006-06-09 kio */
void
GifArray<T>::Grow ( int new_count ) throws/*bad alloc*/
{
/*	Grow GifArray:
	does nothing if requested new_count <= current count.
	if bad_alloc is thrown, then GifArray remained unmodified.
*/
	if( new_count<=count ) return;

	int old_count = count;
	T*  old_array = array;

	array = malloc( new_count );
	count = new_count;

	copy ( array, old_array, old_count );
	clear( array+old_count, new_count-old_count );

	release(old_array);
}














