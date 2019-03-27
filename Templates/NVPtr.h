#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2015
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

#include "../kio/kio.h"
#include "../unix/FD.h"


/*	NVPtr<T>
	retains and locks a volatile object
	until the NVPtr is destroyed or assigned another object or NULL
	provides non-volatile pointer
	member functions are the same as for RCPtr

	NVPtrs are used as temporary/local variables only!

	class T must provide:
		void lock() and unlock()
	if RCPtr<vT> is used (recommended):
		void retain() and void release()


	NVPtr	Lockt das Objekt solange er existiert und rückt einen nicht-volatilen Pointer heraus.
			Das verwendete Objekt.lock sollte NICHT REKURSIV sein!
			Einem NVPtr kann mit operator=() ein neues Objekt oder auch NULL zugewiesen werden.

	NVPtr und RCPtr:
			NVPtr können direkt aus einem NVPtr erzeugt werden.
			Die Verwendung von beiden – RCPtr<volatile T> für refcnt und NVPtr<T> zum Blocken – ist angeraten!

			Solange ein NVPtr oder ein RCPtr auf ein Objekt zeigt, existiert es.
			Sobald kein NVPtr und kein RCPtr mehr auf das Objekt zeigt, wird es gelöscht.

			Sinkt der Referenzzähler auf 0, so wird das Objekt in Objekt::release() gelöscht.
			Das Löschen des Objekts blockiert aber, solange es geblockt ist.

		NOTE:

			Man kann einen RCPtr nicht löschen, solange ein eigener NVPtr auf das selbe Objekt zeigt:
			- Ist das Lock nicht-rekursiv, hat man ein Dead Lock.
			- Ist das Lock rekursiv, zeigt der NVPtr danach auf ein gelöschtes Objekt.

	RCObject:
			an example base class for Objects can be found in RCObject.h
*/


template <class T>
class NVPtr
{
	typedef volatile T	vT;

	T*		p;

	void	lock		()					{ if(p) p->lock(); }
	void	unlock		()					{ if(p) p->unlock(); }

	// semantically impossible:
			NVPtr		(NVPtr&)			= delete;
			NVPtr		(NVPtr const&)		= delete;
	NVPtr&	operator=	(NVPtr&)			= delete;
	NVPtr&	operator=	(NVPtr const&)		= delete;

public:
			NVPtr		()					:p(NULL) {}
			NVPtr		(NVPtr&& q)			:p(q.p) { q.p=NULL; }
//			NVPtr		(RCPtr<vT>& q)		:p(const_cast<T*>(q.p)) { lock(); }
			NVPtr		(vT* p)      		:p(const_cast<T*>(p))  { lock(); }
			NVPtr		(vT& q)      		:p(const_cast<T*>(&q)) { lock(); }
			~NVPtr		()					{ unlock(); }

	NVPtr&	operator=	(NVPtr&& q)			{ assert(this!=&q); unlock(); p=q.p; q.p=NULL; return *this; }
//	NVPtr&	operator=	(RCPtr<vT>& q)		{ if(p!=q.p) { unlock(); p=q.p; lock(); } return *this; }
	NVPtr&	operator=	(vT* q)				{ if(p!=q)   { unlock(); p=q;   lock(); } return *this; }
	NVPtr&	operator=	(ptr p)				{ assert(p==nullptr); unlock(); p=nullptr; return *this; }


	T*		operator->	()					{ return p; }
	T&		operator*	()					{ return *p; }
	T*		ptr			()					{ return p; }
	T&		ref			()					{ return *p; }

	T*		operator->	() const			{ return p; }
	T&		operator*	() const			{ return *p; }
	T*		ptr			() const			{ return p; }
	T&		ref			() const			{ return *p; }

			operator T&	()					{ return *p; }
			operator T*	()					{ return p; }

			operator T& () const			{ return *p; }
			operator T* () const			{ return p; }

	uint	refcnt		()					{ return p ? p->refcnt() : 0; }
	void	swap		(NVPtr& q)			{ T* z=p; p=q.p; q.p=z; }

	bool	isNotNull	() const			{ return p!=NULL; }
	bool	isNull		() const			{ return p==NULL; }
	operator bool		() const			{ return p; }
//	bool	operator==	(const T* p) const	{ return p == this->p; }		ohne geht's besser

	void	print		(FD&, cstr indent) const;
	void	writeToFile	(FD&, void* data=nullptr) const		THF;
// 	void	readFromFile(FD&, void* data=nullptr)			THF;
};


// helper:
// intended usage:  nvptr(my_vol_obj_ptr)->call_non_vol_member_function();
//
template<class T> NVPtr<T> nvptr(volatile T* o) { return NVPtr<T>(o); }



template<class T>
void NVPtr<T>::print( FD& fd, cstr indent ) const
{
	if(p) p->print(fd,indent);
	else fd.write_fmt("%sNULL\n",indent);
}

template<class T>
void NVPtr<T>::writeToFile(FD& fd, void* data) const THF
{
	fd.write_uint8(p!=NULL);
	if(p) p->writeToFile(fd,data);
}










