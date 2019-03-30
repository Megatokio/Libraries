/*	Copyright  (c)	Günter Woigk   2001 - 2019
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

	08.aug.2001	kio	started work on this file
	27.oct.2001 kio finished 2nd rework
	31.oct.2001 kio started test suite; fixed union bug
	04.nov.2001	kio finished test suite, documentation
	21.nov.2001 kio list().array now array of Var* ... again.
	2002-03-21 kio	fixed bug in InsertItems() if !XXXSAFE
	2002-04-01 kio	fixed bug in AppendList()
	2002-04-05 kio	enhanced operators on lists, lots of changes
	2002-04-07 kio	fixed bug in operator> et.al. for lists
	2002-05-20 kio	trap for sizeof(Double)>12
	2003-08-05 kio	ToList() new, ToString() new, changed ToNumber() for isVoid
	2003-08-21 kio	eliminated isVoid
	2003-08-23 kio	operator>>=() and operator<<=() now use ldexp() => now left operand may be non-int
	2003-09-08 kio	ToString() now includes item names and (optionally) skips procs in lists
	2003-10-12 kio	fixed handling of nullptr in lists for string concatenation
	2003-10-18 kio	fixed list-, list size-, and nullptr in list-related issues
	2004-05-28 kio	Umstellung auf NameHandles
	2004-06-29 kio	added variable and data locking

	---------
	Class Var
	---------

	maintain named variables of different types
*/


#define 	SAFE	2
#define 	LOG		1

#include	<math.h>
#include	"config.h"
#include	"Var.h"
#include	"Unicode/Unicode.h"
#include	"Templates/Stack.h"

// serrors.cpp:
inline	String	ErrorString	( int/*OSErr*/ e )				{ return errorstr(e); }
extern	String	ErrorString	( );
extern	void	ForceError	( int/*OSErr*/e, cString& msg );
inline	void	SetError	( int/*OSErr*/e, cString& msg )	{ if(errno==ok) ForceError(e,msg); }
inline	void	ForceError	( cString& msg )				{ ForceError(-1,msg); }
inline	void	SetError	( cString& msg )				{ if(errno==ok) ForceError(-1,msg); }
extern	void	AppendToError	( cString& msg );
extern	void	PrependToError	( cString& msg );

#ifndef NAN
const Double NAN = 0.0/0.0;
#endif

INIT_MSG

inline double random(double r)	{ return ldexp(random() * r, -31); }


// 	0.0
//
//	benötigt Initialisierung!
//	verwendet in:
//		<list>.compare(<list>)
//		<list>.MaxVar()
//		<list>.MinVar()
//		<list>.list_op(<fu>,<list>)
//		<list>.append_list(<list>,<disassproc>)
Var* zero = nullptr;
Var* eins = nullptr;

static struct InitVar
{	InitVar()
	{	zero = new Var(0.0); zero->lock_data(); zero->lock();
		eins = new Var(1.0); eins->lock_data(); eins->lock();
	}
} 	InitVarObj;


/* ----	variables pool -------------------------------------
		new() gets new slots from the pool
		pool empty is detected by first item==nullptr
		delete() returns slot to pool
		pool size is always == amount of all variables ever allocated
		=> return of all variables ever released to pool possible w/o test
		if pool is empty, a new chunk of variables is allocated
		and the pool is increased to the new amount of variables allocated
		allocated chunks are never deallocated.
*/
static Var* no_var		= nullptr;
static uint	pool_size	= 1;

Var**  Var::pool		= (&no_var)+1;

Var*   Var::grow_pool ( )
{
	xlogIn("Var::grow_pool()");

	if(pool!=&no_var) delete[] pool;

	uint n = 500 + pool_size/2;
	pool = new Var*[pool_size+=n];
	Var* var = reinterpret_cast<Var*>(new char[n*sizeof(Var)]);
	xxassert((size_t(var) % _MAX_ALIGNMENT) == 0);

	*pool++ = nullptr;
	while(n--) *pool++ = var++;

	return *--pool;
}




/* ----	validate variables ----------------------------------------
*/
void Var::validate ( cstr file, uint line ) const
{
	#define VTRAP(X) if(!(X)) {} else abort( "internal error in file %s line %u: %s", file, line, "VTRAP(" #X ")" )

	//VTRAP(!this);

	//Log("(%lu)",name);
	GetNameForHandle(name).Check( file, line );

	if (parent)
	{
		VTRAP(int(index)<0);
		VTRAP(parent->IsNoList());
		VTRAP(parent->list().array==nullptr);
		VTRAP(parent->list().used<=index);
		VTRAP(parent->list().array[index]!=this);
	}

	if (IsList())
	{
		Var**a = list().array;
		uint u = list().used;
		uint s = list().size;

		VTRAP(s!=0 && u>s);
		VTRAP((u||s)&&!a);

		for (uint i=0;i<u;i++)
		{
			Var* v = a[i];
			if (!v) continue;
			VTRAP(v->is_unlocked());
			VTRAP(v->parent==this&&v->index!=i);
		}
	}

	if (IsText())
	{
		text().Check( file, line );
	}

	#undef VTRAP
}

#define XXXXCHECK(v)if(SAFE>=4) (v)->validate(__FILE__,__LINE__)
#define XXXCHECK(v)	if(SAFE>=3) (v)->validate(__FILE__,__LINE__)
#define XXCHECK(v)	if(SAFE>=2) (v)->validate(__FILE__,__LINE__)
#define XCHECK(v)	if(SAFE>=1) (v)->validate(__FILE__,__LINE__)
#define CHECK(v)	            (v)->validate(__FILE__,__LINE__)


void Var::set_error_locked ( )
{
	if(data_is_locked())
	{
		SetError( vdataislocked, String("variable ")+GetNameForHandle(name).ToQuoted()+": data is locked" );
	}
	else
	{
		SetError( vtypeislocked, String("variable ")+GetNameForHandle(name).ToQuoted()+": type is locked" );
	}
}



void Var::init_handle ( BPObj* obj, int type )
{
	xassert(obj->varptr==nullptr);

	handle().next = handle().prev = this;
	handle().data = obj;

	obj->varptr = this;
	obj->type = type;
}


void Var::init_handle ( const Var& q )
{
	handle().next = q.handle().next;
	handle().prev = handle().next->handle().prev;
	xxassert(handle().prev==&q);
	handle().next->handle().prev = handle().prev->handle().next = this;
	handle().data = q.handle().data;
}


void Var::kill_handle ( )
{
	if(handle().next==this)
	{
	// last handle => kill object
		delete handle().data;
	}
	else
	{
	// more handles => unlink
		if(handle().data&&handle().data->varptr==this) handle().data->varptr=handle().next;
		handle().next->handle().prev=handle().prev;
		handle().prev->handle().next=handle().next;
	}
}






/* ----	link item into parent's list ---------------------------------
		slot must exist and must be free
*/
inline void Var::init_link ( Var* par, uint idx )
{
	xxassert( par!=nullptr && par->IsList() );
	xxassert( par->list().used>idx && par->list().array[idx]==nullptr );

	parent=par; index=idx; par->list().array[idx]=this;
}

/* ----	link item into parent's list ---------------------------------
		grow list if required
		slot must be free
		handles idx==at_end
*/
inline void Var::init_link_grow ( Var* par, uint idx )
{
	xxassert( par!=nullptr && par->IsList() );

	if (idx>=par->list().used)
	{
		if(idx==at_end) idx=par->list().used;
		par->grow_list(idx+1);
	}
	else
	{
		xxassert(par->list().array[idx]==nullptr);
	}

	parent=par; index=idx; par->list().array[idx]=this;
}

/* ----	link item into parent's list ---------------------------------
		grow list if required
		unlink old item in target slot
		handles idx==at_end
*/
inline void Var::init_link_force ( Var* par, uint idx )
{
	xxassert( par!=nullptr && par->IsList() );

	if (idx>=par->list().used)
	{
		if(idx==at_end) idx=par->list().used;
		par->grow_list(idx+1);
	}
	else
	{
		Var*z = par->list().array[idx];
		if(z) { if(z->parent==par) { z->kill_link_nop(); z->init_link(); } z->unlock(); }
	}

	parent=par; index=idx; par->list().array[idx]=this;
}



/* ----	Link item to list -------------------------------------------
		unlink self
		unlink old item in target slot
		auto-grow parent list
*/
void Var::Link ( Var* par, uint idx )
{
	XXXCHECK ( this );
	XXXCHECK ( par );
	xassert ( par!=nullptr && par->IsList() );
	xassert ( (idx>>26)==0 || idx==at_end );

	if(IsList()&&Contains(par)) goto xl;
	if( par->data_is_locked() ) goto xp;

	if(parent)
	{
		if( parent->data_is_locked() ) goto xq;
		kill_link();
	}
	else
	{
		lock();
		kill_link_nop();
	}

	init_link_force(par,idx);
	return;

xl:	errno = listsintersect; return;
xq:	par = parent;
xp:	par->set_error_locked();  return;
}


/* ----	Unlink item from list -----------------
		slot in parent list becomes free
		auto-shrink
*/
void Var::Unlink ( )
{
	XXXCHECK ( this );
	xassert ( parent!=nullptr );

	Var* par=parent;
	if( par->data_is_locked() )
	{
		par->set_error_locked();
	}
	else
	{
		kill_link();
		init_link();

	// shrink parent list
		uint& i = par->list().used;
		Var** a = par->list().array;
		while( i && a[i-1]==nullptr ) i--;

		unlock();
	}
}

/* ----	Unlink and close gap ---------------------------
*/
void Var::Vanish()
{
	XXXCHECK ( this );
	xassert ( parent!=nullptr );

	parent->DeleteItem(index);
}






/* ----	recursively lock variable type or data ---------------------------------
*/
void Var::setmask ( uint32 mask )
{
	xxassert(IsList());

	Var** a = list().array;
	Var** e = a + list().used;
	while (e>a)
	{
		Var* v = *--e;
		if(v)
		{
			v->type_and_lock |= mask;
			if(v->IsList()) v->setmask(mask);
		}
	}
}

/* ----	recursively unlock variable type or data ---------------------------------
*/
void Var::resmask ( uint32 mask )
{
	xxassert(IsList());

	Var** a = list().array;
	Var** e = a + list().used;
	while (e>a)
	{
		Var* v = *--e;
		if(v)
		{
			v->type_and_lock &= mask;
			if(v->IsList()) v->resmask(mask);
		}
	}
}



/* ----	make list valid ---------------------------------------
		in:  data void
		out: data valid
*/
inline void Var::init_list ( )
{
	list().array = nullptr;
	list().size  = 0;
	list().used  = 0;
}


/* ----	create copy of source vektor -----------------------
*/
void Var::init_list ( cVek& q )
{
	xxlogIn("Var::init_list(Vek)");

	uint n = q.used;
	if (n==0)
	{
	 	init_list();
	}
	else
	{
		Var** qa = q.array;
		Var** za = list().array = new Var*[n];
		memset(za,0,n*sizeof(Var*));
		list().used = list().size = n;

		do
		{
			Var* v = qa[--n];
			if (v)
			{
				new Var( this,n, *v, v->name );
			}
		}
		while (n>0);
  	}
}


/* ----	make list invalid -------------------------------------
		löscht alle variablen im array und delete[] array
		außer: size==0 => subslicer => don't destroy original!
		in:  data valid
		out: data void
*/
void Var::kill_list ( )
{
	xxlogIn("Var::kill_list()");

	if (list().size!=0)
	{
		xxassert(list().used <= list().size);
		xxassert(list().array!=nullptr);

		Var** a = list().array;
		Var** e = a+list().used;
		while (a<e)
		{
			Var* v = *--e;
			if (v)
			{
				if(v->parent==this) { v->kill_link_nop(); v->init_link(); }
				v->unlock();
			}
		}
		delete[] a;
	}
}



/* ----	grow list ------------------------------------------
		grow list => loose fit
*/
void Var::grow_list ( uint newused )
{
	xxlogIn("Var::grow_list()");

	xxassert((newused>>26)==0);			// oorange security

	if (newused>list().size)		// grow array
	{
		list().size = newused + 10;
		Var** newarray = new Var*[list().size];
		memcpy ( newarray, list().array, list().used*sizeof(Var*) );		// move items flat
		delete[] list().array;
		list().array = newarray;
	}

	while (list().used<newused)
	{
		list().array[list().used++] = nullptr; 	// händisch 'createn'
	}
}


/* ----	shrink list ------------------------------------------
		shrink list => tight fit
*/
void Var::shrink_list ( uint newused )
{
	xxlogIn("Var::shrink_list()");

	xxassert(newused <= list().used);

	if (list().size>newused)		// !subslicer && !already_tight
	{
		Var** oldarray = list().array;

		while (list().used>newused)			// händisch destroyen
		{
			Var* v = oldarray[--list().used];
			if (v) { if(v->parent==this) { v->kill_link_nop(); v->init_link(); } v->unlock(); }
		}
		list().size = newused;
		Var** newarray = nullptr;

		if (newused>0)
		{
			newarray = new Var*[newused];
			memcpy ( newarray, oldarray, newused*sizeof(Var*) );			// move items flat
		}

		delete[] oldarray;
		list().array = newarray;
	}
}


/* ----	Insert empty items into list -------------------------------
		note: inserted items are set to nullptr == Var("",0.0)
*/
void Var::insert_items ( uint idx, uint n )
{
	xxlogIn("Var::insert_items(idx,n)");
	xxassert( (n>>26)==0 && (idx>>26)==0 );

	if (idx>list().used) { n += idx-list().used; idx=list().used; }

	if (n>0)
	{
		uint m = list().used+n;
		grow_list(m);						// make room
		Var** array = list().array;

		while (m>idx+n) 					// move items up
		{
			m--;
			if( (array[m] = array[m-n]) && array[m]->parent==this ) array[m]->index = m;
		}

		while (m>idx)						// init items in gap
		{
			m--;
			array[m] = nullptr;
		}
	}
}


/* ----	Remove items from list -------------------------------
		fast, no shrink to fit
*/
void Var::delete_items ( uint idx, uint n )
{
	xxlogIn("Var::delete_items(idx,n)");
	xxassert((n>>26)==0);

	if (n>0 && idx<list().used)				// idx>=used  =>  there's nothing to delete
	{
		if (n+idx>list().used) n = list().used-idx;

		Var** array = list().array;

		for (uint i=idx; i<idx+n; i++)
		{
			Var* v = array[i];
			if(v) { if(v->parent==this) { v->kill_link_nop(); v->init_link(); } v->unlock(); }
		}

		uint m = list().used-n;
		list().used = m;

		for (uint i=idx; i<m; i++)
		{
			if( (array[i] = array[i+n]) && array[i]->parent==this ) array[i]->index = i;
		}
	}
}





/* ----	purge data ------------------------------------------
		purge text of String or Proc
		purge List recursively
		in:  data valid
		out: data invalid
			 lock preserved
*/
void Var::kill_data ( )
{
	xxlogIn("Var::kill_data()");

	switch( get_type() )
	{
	default: 		IERR();

	case isSema:
	case isIrpt:
	case isThread:
	case isStream:	kill_handle();break;

	case isVarRef:	kill_varptr();break;
	case isProc:	kill_proc();  break;
	case isText:	kill_text();  break;
	case isList:	kill_list();  break;
	case isNumber:	kill_value(); break;
	}

	kill_type();
}


/* ----	init data -------------------------------
		in:	 data invalid
		out: data valid: default value for type
*/
void Var::init_data ( vtype typ )
{
	xxlogIn("Var::init_data(vtype)");

	init_type(typ);

	switch(typ)
	{
	default: 		IERR();
	case isText: 	init_text();  break;
	case isList:	init_list();  break;
	case isNumber:	init_value(); break;
	}
}


/* ----	copy source data from target data -----------------------------
		in:   data invalid  q.data valid
		out:  data valid	q.data valid
*/
void Var::init_data ( cVar& q )
{
	xxlogIn("Var::init_data(Var)");

	//xxassert( this && &q );
	xxassert( &q!=this );

	vtype typ = q.get_type();
	init_type(typ);

	switch( typ )
	{
	default: 		IERR();

	case isThread:
	case isIrpt:
	case isSema:
	case isStream:	init_handle(q);break;

	case isVarRef:	init_varptr(q.varptr());break;
	case isProc:	init_proc(q.proc());    break;
	case isText:	init_text(q.text());    break;
	case isList:	init_list(q.list());    break;
	case isNumber:	init_value(q.value());  break;
	}
}


/* ----	move source data from target data -----------------------------
		in:   data invalid  q.data valid
		out:  data valid	q.data invalid
*/
void Var::move_data ( Var& q )
{
	xxlogIn("Var::move_data(Var)");

	//xxassert( this && &q );
	xxassert( &q!=this );

	vtype typ = q.get_type();
	init_type(typ);
	q.kill_type();

	switch( typ )
	{
	default:
		IERR();

	case isThread:
	case isSema:
	case isIrpt:
	case isStream:
		handle().next = q.handle().next;
		handle().prev = q.handle().prev;
		handle().data = q.handle().data;
		if(handle().data&&handle().data->varptr==&q) handle().data->varptr=this;
		handle().next->handle().prev = this;
		handle().prev->handle().next = this;
		break;

	case isVarRef:
		varptr() = q.varptr(); break;		// move pointer

	case isList:
	// move list
		list().array = q.list().array;
		list().size  = q.list().size;
		list().used  = q.list().used;
	// fix_parents
	{	Var** a = list().array;
		Var** e = a + list().used;
		while (a<e) { Var* v = *--e; if (v&&v->parent==&q) v->parent = this; }
	}	break;

	case isProc:
		proc().bundle	= q.proc().bundle;
		proc().ip		= q.proc().ip;
		break;

	case isText:
		text()._move(q.text()); break;

	case isNumber:
		value() = q.value();	break;
	}
}


/* ----	swap source data with target data -----------------------------
		in:   data valid	q.data valid
		out:  data valid	q.data valid
		note: lists must not intersect or a circular list will result
*/
void Var::swap_data ( Var& q )
{
	xxlogIn("Var::swap_data(Var)");

	Var z;
	z.kill_type();				// nop
	z.kill_value();				// nop

	z.move_data(q);
	q.move_data(*this);
	  move_data(z);

	z.init_type_and_lock();		// => can be safely destroyed
}


/* ----	creator, destructor ---------------------------------------------------
*/
Var::Var ( Var* par, uint idx )
{
	xxlogIn("Var::Var(par,idx)");
	XXCHECK(par);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock();
	incr_lock();

	XXXXCHECK(this);
}

Var::Var ( Var* par, uint idx, vtype typ )
{
	xxlogIn("Var::Var(par,idx,type)");
	XXCHECK(par);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock(typ);
	incr_lock();

	XXXXCHECK(this);
}

Var::Var ( Var* par, uint idx, Double value )
{
	xxlogIn("Var::Var(par,idx,number)");
	XXCHECK(par);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock(value);
	incr_lock();

	XXXXCHECK(this);
}

Var::Var ( Var* par, uint idx, cString& text )
{
	xxlogIn("Var::Var(par,idx,text)");
	XXCHECK(par);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock(text);
	incr_lock();

	XXXXCHECK(this);
}

Var::Var ( Var* par, uint idx, cVar& q )
{
	xxlogIn("Var::Var(par,idx,Var)");
	XXCHECK(par);
	XXCHECK(&q);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock(q);
	incr_lock();

	XXXXCHECK(this);
}



Var::Var ( Var* par, uint idx, vtype typ, NameHandle nh )
{
	xxlogIn("Var::Var(par,idx,type,name)");
	XXCHECK(par);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(typ);
	incr_lock();

	XXXXCHECK(this);
}

Var::Var ( Var* par, uint idx, Double value, NameHandle nh )
{
	xxlogIn("Var::Var(par,idx,number,name)");
	XXCHECK(par);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(value);
	incr_lock();

	XXXXCHECK(this);
}

Var::Var ( Var* par, uint idx, cString& text, NameHandle nh )
{
	xxlogIn("Var::Var(par,idx,text,name)");
	XXCHECK(par);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(text);
	incr_lock();

	XXXXCHECK(this);
}

Var::Var ( Var* par, uint idx, Stream* stream, NameHandle nh )
{
	xxlogIn("Var::Var(par,idx,stream,name)");
	XXCHECK(par);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(stream);
	incr_lock();

	XXXXCHECK(this);
}

Var::Var ( Var* par, uint idx, cVar& q, NameHandle nh )
{
	xxlogIn("Var::Var(par,idx,Var,name)");
	XXCHECK(par);
	XXCHECK(&q);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(q);
	incr_lock();

	XXXXCHECK(this);
}



Var::Var ( vtype typ )
{
	xxlogIn("Var::Var(type)");

	init_name();
	init_link();
	init_data_and_lock(typ);

	XXXXCHECK(this);
}

Var::Var ( Double value )
{
	xxlogIn("Var::Var(number)");

	init_name();
	init_link();
	init_data_and_lock(value);

	XXXXCHECK(this);
}

Var::Var ( cString& text )
{
	xxlogIn("Var::Var(text)");

	init_name();
	init_link();
	init_data_and_lock(text);

	XXXXCHECK(this);
}

/*
Var::Var ( cVar& q )
{
	xxlogIn("Var::Var(Var)");

	init_name();
	init_link();
	init_data_and_lock(q);

	XXXXCHECK(this);
}
*/


Var::Var ( vtype typ, NameHandle name )
{
	xxlogIn("Var::Var(type,name)");

	init_name(name);
	init_link();
	init_data_and_lock(typ);

	XXXXCHECK(this);
}

Var::Var ( Double value, NameHandle name )
{
	xxlogIn("Var::Var(number,name)");

	init_name(name);
	init_link();
	init_data_and_lock(value);

	XXXXCHECK(this);
}

Var::Var ( cString& text, NameHandle name )
{
	xxlogIn("Var::Var(text,name)");

	init_name(name);
	init_link();
	init_data_and_lock(text);

	XXXCHECK(this);
}

Var::Var ( cVar& q, NameHandle name )
{
	xxlogIn("Var::Var(Var,name)");

	init_name(name);
	init_link();
	init_data_and_lock(q);

	XXXXCHECK(this);
}



/* ----	List Var from cstr array ----------------------------
		list terminates after argc elements or at nullptr
*/
Var::Var ( char const*const* argv, uint n )
{
	xlogIn("Var::Var(argv[],argc)");

	init_name();
	init_link();
	init_data_and_lock(isList);

	cstr s;
	for ( uint i=0; i<n && (s=argv[i]); i++ )
	{
		new Var( this,i, s );
	}

	XXXXCHECK(this);
}




/* ----	Assignment ----------------------------------
		normal:     purge own data  &  copy source data
					wenn this eine Liste ist, wird der Inhalt rekursiv deleted!
					(bei Überschneidung von q mit z nur der nicht wiederverwertbare Teil)

		zusätzlich: prüfung auf überschneidung von quelle und ziel:

		wenn ziel==list && quelle element darin:
			normales vorgehen würde quelldaten vernichten
			=> erst quelldaten aus list extrahieren

			=>  a = { b,c }  =>  a = a[0]  =>  a == b

			••• Wichtig ••• Es ist vielleicht nicht offensichtlich,
			aber in diesem Fall wird auch die Quellvariable q selbst deleted, wenn sie nicht gelockt ist:
			q ist item in list z und alles was in z nicht re-used werden kann, wird deleted.
			würde q nicht deleted, wäre dies ein memory leak.

		wenn quelle==list && ziel element darin:
			normales vorgehen würde teil der quelldaten vernichten
			=> erst quelldaten duplizieren
			Anm.: es wäre möglich, unnötiges duplizieren der eigenen instanz (evtl. selbst ein fetter list!) zu vermeiden:
				  eigenen pfad bis zur quellinstanz merken => index-liste
				  eigene daten in scratch moven => eigene stelle jetzt leer
				  quelle nach ziel kopieren => es fehlt noch der eigene subtree
				  kopie der eigenen position anhand der index-liste suchen
				  daten aus scratch in die kopie der eigenen position moven

			=> a = { b,c }  =>  a[0] = a  =>  a == { {b,c} c }
*/



Var& Var::operator= ( Var& q )
{
	xxlogIn("Var::operator=(Var)");

	if (this!=&q)
	{
		XXCHECK( this );
		XXCHECK( &q );

		if( type_or_data_locked() && ( data_is_locked() || get_type()!=q.get_type() ) )
		{
			set_error_locked();
		}
		else
		{
	/*	wenn quelle und ziel sich überschneiden könnten die daten schnell verschoben werden,
		wenn man ausschließen kann, dass die datenquelle irgendwo in ihren tiefen items hat,
		die data- oder type-gelockt sind und auf die noch jmd. anderes einen pointer hält.
		bis das intelligent nachgeprüft ist, haben wir auch dumm kopiert.
	*/
			q.lock();			// in case we intersect
			kill_data();
			init_data(q);
			XXXCHECK(this);
			XXXCHECK(&q);
			q.unlock();			// may vanish
		}
	}

	return *this;
}


/* ----	quick copy data from source ----------------------------------
		Daten dürfen sich nicht überschneiden!
*/
void Var::CopyData ( Var& q )
{
	xxlogIn("Var::CopyData(Var)");

	XXCHECK ( this );
	XXCHECK ( &q );
	assert (   IsNoList() ||   !Contains(q)    );
	assert ( q.IsNoList() || !q.Contains(this) );

	if( type_or_data_locked() && (data_is_locked() || q.get_type()!=get_type()) )
	{
		set_error_locked();
	}
	else
	{
		kill_data();
		init_data(q);
		XXXCHECK ( this );
		XXXCHECK ( &q );
	}
}


/* ----	quick move data from source ----------------------------------------
		Daten dürfen sich nicht überschneiden!
*/
void Var::MoveData ( Var& q )
{
	xxlogIn("Var::MoveData(Var)");

	XXCHECK ( this );
	XXCHECK ( &q );
	assert (   IsNoList() ||   !Contains(q)    );
	assert ( q.IsNoList() || !q.Contains(this) );
	assert ( !q.type_or_data_locked() || (!q.data_is_locked() && q.get_type()==isNumber  ) );

	if( type_or_data_locked() && (data_is_locked() || q.get_type()!=get_type()) )
	{
		set_error_locked();
	}
	else
	{
		kill_data();
		move_data(q);
		q.init_data();

		XXXCHECK ( this );
		XXXCHECK ( &q );
	}
}


/* ----	quick swap data with source ----------------------------------
		Daten dürfen sich nicht überschneiden!
*/
void Var::SwapData ( Var& q )
{
	xxlogIn("Var::SwapData(Var)");

	XXCHECK ( this );
	XXCHECK ( &q );

	if( Parent()!=q.Parent() && ((IsList()&&Contains(q))||(q.IsList()&&q.Contains(this))) )
	{
		errno = listsintersect;
	}
	if( type_or_data_locked() && (data_is_locked() || q.get_type()!=get_type()) )
	{
		set_error_locked();
	}
	else if( q.type_or_data_locked() && (q.data_is_locked() || q.get_type()!=get_type()) )
	{
		q.set_error_locked();
	}
	else
	{
		swap_data(q);
		XXXCHECK ( this );
		XXXCHECK ( &q );
	}
}


/* ----	resize list ------------------------------------------
		grow list   => loose fit
		shrink list => tight fit
*/
void Var::ResizeList ( uint newused )
{
	xxlogIn("Var::ResizeList()");

	XXCHECK(this);
	assert(IsList());

	if(data_is_locked())
	{
		set_error_locked();
	}
	else
	{
		if (newused>list().used) grow_list(newused);
		else					 shrink_list(newused);
		XXXCHECK(this);
	}
}


/* ----	operator[] -------------------------------------------------
		grows list and creates item if required
		detects idx==at_end
*/
Var& Var::operator[] ( uint idx )
{
	xxlogIn("Var::operator[]");

	XXXCHECK(this);
	assert(IsList());

	if (idx>=list().used)
	{
		if(data_is_locked())
		{
			set_error_locked();
			return *this;
		}
		if(idx==at_end) idx=list().used;
		xassert((idx>>26)==0);
		grow_list(idx+1);
	}

	Var* v = list().array[idx];
	if(!v) v = new Var(this,idx);

	return *v;
}


/* ----	find item by name --------------------------------------------------
		find named item in list
		returns found Var or nullptr
		named item is searched starting at the end of the list
*/
Var* Var::FindItem ( NameHandle namehandle ) const
{
	xxlogIn("Var::FindItem(name)");
	XXXCHECK(this);

	if (IsList())
	{
		Var** a = list().array;
		Var** e = a+list().used;

		while ( e > a )
		{
			Var* v = *--e;
			if (v) { if (namehandle!=v->name) continue; }
			else   { if (namehandle!=0) continue; else v = new Var(const_cast<Var*>(this),uint(e-a)); }
			return v;
		}
	}

	return nullptr;
}


Var* Var::FindItem ( cString& name ) const
{
	NameHandle n = FindNameHandle(name);
	return  n!=0 || name.Len()==0  ? FindItem( n ) : nullptr;
}


Var* Var::FindItem ( cstr name ) const
{
	NameHandle n = FindNameHandle(name);
	return  n!=0 || name[0]==0  ? FindItem( n ) : nullptr;
}


/* ----	Search for Item ------------------------------------ [2004-06-10 kio]
		search for Item in List
		return index of item found or -1 if not found
*/
int32 Var::Find ( cVar& v, int32 startidx ) const
{
	xxlogIn("Var::Find(Var)");

	XXCHECK(this);
	XXXCHECK(&v);

	if (IsList())
	{
		bool visnull = v.IsNumber()&&v.value()==0.0;
		if (startidx<0) startidx=0;
		int32 endidx  = int32(list().used);

		while(startidx<endidx)
		{
			Var* a = list().array[startidx++];
			if (a==nullptr ? visnull : v.get_type()==a->get_type() && v.compare(*a)==0)
			{
				return startidx-1;
			}
		}
	}
	else errno = listrequired;

	return -1;
}

int32 Var::RFind ( cVar& v, int32 startidx ) const
{
	xxlogIn("Var::RFind(Var)");

	XXCHECK(this);
	XXXCHECK(&v);

	if (IsList())
	{
		bool visnull = v.IsNumber()&&v.value()==0.0;
		if (startidx>=(int32)list().used) startidx=list().used-1;

		while(startidx>=0)
		{
			Var* a = list().array[startidx--];
			if (a==nullptr ? visnull : v.get_type()==a->get_type() && v.compare(*a)==0)
			{
				return startidx+1;
			}
		}
	}
	else errno = listrequired;

	return -1;
}


/* ----	Insert empty items into list -------------------------------
		if idx>used then nothing is done, because accessing
		items beyond used automatically creates them
		note: inserted items are set to nullptr == Var("",0.0)
		handles idx==at_end
*/
void Var::InsertItems ( uint idx, uint n )
{
	xxlogIn("Var::InsertItems(idx,n)");

	XXCHECK(this);
	assert(IsList());

	if(idx==at_end) idx=list().used;
	xassert((idx>>26)==0 && (n>>26)==0);

	if(data_is_locked())
	{
		set_error_locked();
	}
	else
	{
		insert_items(idx,n);
		XXXCHECK(this);
	}
}


/* ----	insert item into list ----------------------------
		handles idx==at_end
*/
void Var::InsertItem ( uint idx, Var* q )
{
	xxlogIn("Var::InsertItem(idx,Var)");

	XXCHECK ( this );
	XXCHECK ( q );
	xassert( q->IsNoList() || !q->Contains(this) );
	assert ( IsList() );

	if(idx==at_end) idx=list().used;
	xassert((idx>>26)==0);

	if(data_is_locked())
	{
		set_error_locked();
	}
	else if(q->parent&&q->parent->data_is_locked())
	{
		q->parent->set_error_locked();
	}
	else
	{
		insert_items(idx,1);
		if(q->parent) { q->kill_link(); } else { q->kill_link_nop(); q->lock(); }
		q->init_link(this,idx);
		XXXCHECK(this);
	}
}


void Var::AppendItems ( Var* a, Var* b, Var* c, Var* d )
{
	xlogIn("Var::AppendItems(4)");
	XXCHECK (this);
	assert(IsList());
	xxassert(
		!d || !d->IsStream() || (d->GetStream()->InputEncoding()>=ucs1 && d->GetStream()->InputEncoding()<=utf8 &&
							     d->GetStream()->OutputEncoding()>=ucs1 && d->GetStream()->OutputEncoding()<=utf8) );

	if(data_is_locked())
	{
		set_error_locked();
	}
	else
	{
#define APPEND(V)													\
	if(V) { xassert(!V->Contains(this)); xassert(!V->is_linked());		\
			V->kill_link_nop(); V->init_link(this,n++); V->lock(); }

		uint n = list().used;
		grow_list(n+4);
		APPEND(a);
		APPEND(b);
		APPEND(c);
		APPEND(d);
		list().used = n;
		XXXCHECK(this);
	}

	xxassert(
		!d || !d->IsStream() || (d->GetStream()->InputEncoding()>=ucs1 && d->GetStream()->InputEncoding()<= utf8 &&
							     d->GetStream()->OutputEncoding()>=ucs1 && d->GetStream()->OutputEncoding()<=utf8));
}

void Var::AppendItems ( Var* a, Var* b, Var* c, Var* d, Var* e, Var* f, Var* g, Var* h )
{
	AppendItems(a,b,c,d);
	AppendItems(e,f,g,h);
}


/* ----	Remove items from list -------------------------------
*/
void Var::DeleteItems ( uint idx, uint n )
{
	xxlogIn("Var::DeleteItems(idx,n)");

	XXCHECK (this);
	assert(IsList());

	if(data_is_locked())
	{
		set_error_locked();
	}
	else
	{
		delete_items(idx,n);
		XXXCHECK(this);
	}
}


/* ----	remove last item from list ---------------------------
		fast, no shrinking of array
*/
void Var::DeleteLastItem ( )
{
	xxlogIn("Var::DeleteLastItem()");

	XXCHECK (this);
	assert(IsList());


	if(list().used>0)
	{
		if(data_is_locked())
		{
			set_error_locked();
		}
		else
		{
			Var* v = list().array[--list().used];
			if (v)
			{
				if(v->parent==this) { v->kill_link_nop(); v->init_link(); }
				v->unlock();
			}
			XXXCHECK(this);
		}
	}
}


/* ----	append list q to this list --------------------------
		in:  this must be a lists; else it is casted to list
		out: the contents of q is appended to this list
			 if q is no list, then q itself is appended to this list
*/
void Var::AppendListItems ( Var& q )
{
	xxlogIn("Var::AppendList(Var)");

	XXXCHECK(&q);
	XXXCHECK(this);
	xassert( !q.Contains(this) );

	if(data_is_locked()) { set_error_locked(); goto x; }

	if (IsNoList())
	{
		if(type_is_locked()) { set_error_locked(); goto x; }
	// convert to list
		Var*z = new Var(isList);
		swap_data(*z);
		z->kill_link_nop();
		z->init_link_grow(this,0);
		z->lock();
	}

	if (q.IsNoList())
	{
		AppendItem(new Var(q));
		goto x;
	}
	else	// this is list & q is list
	{
		if(q.data_is_locked()) { q.set_error_locked(); goto x; }

		uint  n = list().used;
		uint  m = q.list().used;
		grow_list(n+m);
		Var** qa = q.list().array;

		while (m--)
		{
			if (qa[m])
			{
				qa[m]->kill_link_nop();
				qa[m]->init_link(this,n+m);
			}
		}

		q.list().used=0;
	}

	XXXCHECK(this);
	XXXCHECK(&q);
x:	;
}


/* ----	compare two Variables ---------------------------------------
		value: NANs according to IEEE
		text:  alphanumeric without any auto-intelligent pittraps
		proc:  as text
		list:  compare item by item
		mixed types: compare type (for Sort()/RSort()) and set error

		return values:
		>0	this >  q
		 0	this == q
		<0	this <  q
		NAN unsortable
*/
Double Var::compare ( cVar& q ) const
{
	xxlogIn("Var::compare(Var)");

	XXXCHECK(this);
	XXXCHECK(&q);

	if (get_type() == q.get_type())
	{
		switch(get_type())
		{
		case isNumber:	// special care for NAN!
		{
			Double a = value();
			Double b = q.value();
			return a>b ?  1 : a<b ? -1 : a==b ? 0 : NAN;
		}

		case isProc:	return ssize_t( proc().ip - q.proc().ip);
		case isText:	return text().compare(q.text());

		case isList:
		{	uint n = min(list().used,q.list().used);
			for (uint i=0; i<n; i++)
			{
				Var* a = list().array[i];	if (!a) a=zero;
				Var* b = q.list().array[i];	if (!b) b=zero;
				Double r = a->compare(*b);
				if (r==0) continue;
			//	if(errno&&errno!=typemismatch) ClearError();	// ***TODO*** setzen NANs errno ?
				return r;		// >, <, or unsortable
			}
		}	return int(list().used) - int(q.list().used);

		case isStream:	return ssize_t( stream() - q.stream() );
		case isIrpt:	return ssize_t( irpt() - q.irpt() );
		case isSema:	return ssize_t( sema() - q.sema() );
		case isThread:	return ssize_t( thread() - q.thread() );
		case isVarRef:	return varptr()->compare(*q.varptr());
		default: 		IERR();
		}
	}
	if(IsVarRef()) return varptr()->compare(q);
	if(q.IsVarRef()) return compare(*q.varptr());

	errno = typemismatch;
	return int(get_type()) - int(q.get_type());		// mixed type
}


/* ----	sort list -------------------------------------------------------
*/
#undef TYPE
#undef GT
#define TYPE		Var*
#define GT(A,B) 	(*(A)>*(B))
void Var::Sort(TYPE* a, TYPE* e)
#include "Templates/sorter.h"

#undef TYPE
#undef GT
#define TYPE		Var*
#define GT(A,B)	(*(A)<*(B))
void Var::RSort(TYPE* a, TYPE* e)
#include "Templates/sorter.h"


void Var::sort( SortProcPtr sorter )
{
	xxlogIn("Var::sort()");
	XXCHECK(this);

	if( IsList() && list().used>=2 )
	{
		if(data_is_locked())
		{
			set_error_locked();
		}
		else
		{
		// prepare for sorting:
			uint  n     = list().used;
			Var** qlist = list().array;
			for(uint i=n;i--;) { if(qlist[i]==nullptr) qlist[i] = zero; }

		// doit
			// use temp array, if compare() performs CHECK()
			// because sorter() only swaps pointers and not also item.index
			#if SAFE >= 3
				Var** zlist = new Var*[n];
				memcpy(zlist,qlist,n*sizeof(Var*));
				(*sorter)(zlist,zlist+n);
				memcpy(qlist,zlist,n*sizeof(Var*));
				delete[] zlist;
			#else
				(*sorter)(qlist,qlist+n);
			#endif

		// fix parents and NULLs
			for(uint i=n;i--;)
			{
				if(qlist[i]->parent==this) qlist[i]->index = i;
				else if(qlist[i]==zero) { qlist[i] = nullptr; }
			}

			//ClearError();	// clear type mismatchs

			XXXCHECK(this);
		}
	}
}


/* ----	shuffle list ----------------------------------- [2004-06-21 kio]
*/
void Var::Shuffle()
{
	xxlogIn("Var::Shuffle()");
	XXCHECK(this);

	if( IsList() && list().used>=2 )
	{
		if(data_is_locked())
		{
			set_error_locked();
		}
		else
		{
			Var** array = list().array;
			for (uint i=list().used;i;)
			{
				uint j = random(i--);
				if (i!=j) kio::swap(array[i],array[j]);
				if(array[i]) array[i]->index = i;
			}
		}
	}
}



/* ----	Greater of two variables -----------------------
		returns a reference to the larger one
		=> Max(a,b)=c possible
		any data type may be passed (as in Sort())
		returns this if both are same
*/
Var& Var::MaxVar ( Var& q )
{
	if (*this>=q) return *this; else return q;
}


/* ----	Smaller of two variables -----------------------
		returns a reference to the smaller one
		=> Max(a,b)=c possible
		any data type may be passed (as in Sort())
		returns this if both are same
*/
Var& Var::MinVar ( Var& q )
{
	if (*this<=q) return *this; else return q;
}


/* ----	largest item of list -------------------------
		returns a reference to the largest item in list
		=> Max(a)=b possible
		returns this if this is no list or list.used==0
		any data type may be passed (as in Cmp())
		returns lowest index item if multiple items are same
*/
Var& Var::MaxVar ( )
{
	XXXCHECK(this);

	if (IsList() && list().used>0)
	{
		Var** array = list().array;

		uint im = 0;
		Var* vm = array[0]; if (!vm) vm = zero;

		for (uint i=1; i<list().used; i++)
		{
			Var* v = array[i]; if(!v) v=zero;
			if (*v>*vm) { im=i; vm=v; }
		}

		if (vm!=zero) return *vm;
		return *new Var(this,im);			// ***TODO*** test it
	}
	return *this;
}


/* ----	smallest item of list -------------------------
		returns a reference to the smallest item in list
		=> Min(a)=b possible
		returns this if this is no list or list.used==0
		any data type may be passed (as in Cmp())
		returns lowest index item if multiple items are same
*/
Var& Var::MinVar ( )
{
	XXXCHECK(this);

	if (IsList() && list().used>0)
	{
		Var** array = list().array;

		uint im = 0;
		Var* vm = array[0]; if (!vm) vm = zero;

		for (uint i=1; i<list().used; i++)
		{
			Var* v = array[i]; if(!v) v=zero;
			if (*v<*vm) { im=i; vm=v; }
		}

		if (vm!=zero) return *vm;
		return *new Var(this,im);			// ***TODO*** test it
	}
	return *this;
}


/* ----	Convert Character Set -------------------------------
		convert text
		convert list items
		ignore number and proc
*/
void Var::ConvertTo ( CharEncoding ctype )
{
	XXXCHECK(this);

	if (IsText())
	{
		if(data_is_locked()) set_error_locked();
		else text() = text().ConvertedTo(ctype);
	}
	else if (IsList())
	{
		for (uint i=list().used;i;)
		{
			Var* v = list().array[--i];
			if (v) v->ConvertTo(ctype);
		}
	}
}

void Var::ConvertFrom ( CharEncoding ctype )
{
	XXXCHECK(this);

	if (IsText())
	{
		if(data_is_locked()) set_error_locked();
		else text() = text().ConvertedFrom(ctype);
	}
	else if (IsList())
	{
		for (uint i=list().used;i;)
		{
			Var* v = list().array[--i];
			if (v) v->ConvertFrom(ctype);
		}
	}
}



/* ----	Log to Log Window -------------------------------
		List variable contents to log window
		***UNUSED*** ?
*/
#if 0
void Var::LogVar ( )
{
	XXXCHECK(this);

	switch(get_type())
	{
	case isNumber:	Log("%s ", CString(NumString(value())));		break;
	case isText:	Log(usingstr("%s ",CString(text().ToQuoted())));	break;
	case isProc: 	Log("proc ");									break;

	case isList:
		Log("{ ");

		for ( uint i=0; i<list().used; i++ )
		{
			Var* v = list().array[i];
			if (v && v->IsNamed()) Log(usingstr("%s=",CString(v->Name())));
			if (v) v->LogVar(); else Log("0 ");
		}

		Log("} ");
		break;

	default: 		IERR();
	}
}
#endif



/* ----	convert variable to numeric value -------------------------

		as required for automatic type casting in vipsi

		number	->	number
		text	->	NumVal(text)
		proc	->	((can't eval to number))
		List	->	((recursive; not handled here))
*/
Double Var::ToNumber ( ) const
{
	if (IsNumber())	 	  return value();
	if (IsText())   	  return text().NumVal();
	errno = notanumber; return NAN;
}

void Var::ConvertToNumber ( )
{
	if(type_or_data_locked())
	{
		if(data_is_locked()||IsNoNumber()) { set_error_locked(); return; }
	}
	switch(get_type())
	{
	default:		set_data(NAN); errno = notanumber; break;
	case isText:	set_data(text().NumVal()); break;
	case isNumber: 	break;
	}
}


/* ----	convert Variable to String ----------------------------

		as required for automatic type casting in vipsi
		currently only for operator: text # text

		note:					vipsi function string(x) behaves differently

		number	-> "123.456"
		string	-> as is
		List	-> "{ 1, 2, 3, "text\n", ... }"
		Proc	-> disass

		quotestring:	quote and escape text or all texts contained in a list
		disass:			disassemble proc via disass(),
						else print only "proc()" / skip list items which are procs
*/
String Var::ToString ( bool quotestring, DisassProcPtr disass ) const
{
	switch(get_type())
	{
	case isNumber:	return NumString(value());			// isNumber
	case isText:	return quotestring ? text().ToQuoted() : text();
	case isVarRef:	return varptr()->ToString(quotestring,disass);
	case isStream:	if(!disass) return "stream<>";
	case isIrpt:	if(!disass) return "irpt<>";
	case isSema:	if(!disass) return "sema<>";
	case isProc:	if(!disass) return "proc()";
	default: 		if(!disass) return "void<>";
					return (*disass)(*this);
	case isList:
		xlogIn("Var::ToString(List)");
		String s = '{', sep = ' ', sep2 = ", ";

		for ( uint i=0; i<list().used; i++ )
		{
			Var* v = list().array[i];
			if (v)
			{
				if (v->IsProc()&&!disass) continue;			// don't include procs in listing

				if (v->IsNamed())							// include name ?
				{
					cString& v_name = v->Name();
					UCS4Char c = v_name[0];					// check for malformed name
					bool f = UCS4CharIsLetter(c) || c=='_';
					for (int i=1;f&&i<v_name.Len();i++) { c = v_name[i]; f = UCS4CharIsLetter(c) || UCS4CharIsDecimalDigit(c) || c=='_'; }
					s += f ? sep + v_name					// add well formed name
						   : sep + '@'+v_name.ToQuoted();	// add malformed name
					sep = '=';
				}
				s += sep + v->ToString(yes,disass);			// recursively add item
			}
			else											// nullptr == number 0.0
			{
				s += sep + '0';
			}

			sep = sep2;
		}

		return s + " }";
	}
}

void Var::ConvertToString ( bool quotestring, DisassProcPtr disass )
{
	XXCHECK(this);

	if (type_or_data_locked())
	{
		if( IsText()&&!quotestring ) return;
		if( data_is_locked() || IsNoText() ) { set_error_locked(); return; }
	}
	*this = ToString(quotestring,disass);

	XXXCHECK(this);
}



/* ---- convert Variable to List ------------------------------
		number	->	{ number }
		text	->	{ text }
		proc	->	{ proc }
		List	->	as is		if !putlistinlist
				->	{ list }	if putlistinlist
*/
void Var::ConvertToList ( bool putlistinlist )
{
	XXCHECK(this);

	if (type_or_data_locked())
	{
		if( IsList()&&!putlistinlist ) return;
		if( data_is_locked() || IsNoList() ) { set_error_locked(); return; }
	}

	Var*z = new Var(isList);
	swap_data(*z);
	z->kill_link_nop();
	z->init_link_grow(this,0);
	z->lock();

	XXXCHECK(this);
}


void Var::JoinLines( cString& sep )
{
	XXCHECK(this);

	if (type_or_data_locked())
	{
		if(data_is_locked()||IsNoText()) { set_error_locked(); return; }
	}

	if (IsNoList())
	{
		ConvertToString();
	}
	else if (list().used==0)
	{
		SetString();
	}
	else
	{
		int32 len = (list().used-1)*sep.Len();
		CharSize csz=sep.Csz();

		for(uint i=0;i<list().used;i++)
		{
			Var* v = list().array[i];
			if (v->IsNoText()) v->ConvertToString();
			len += v->text().Len();
			if (v->text().Csz()>csz) csz=v->text().Csz();
		}

		String t(len,csz);
		//t.MakeWritable();

		ptr z = t.Text();
		for(uint i=0;i<list().used;i++)
		{
			if(i) { UCSCopy( csz, z, sep.Csz(), sep.Text(), sep.Len() ); z += csz*sep.Len(); }
			cString& q = list().array[i]->text();
			UCSCopy( csz, z, q.Csz(), q.Text(), q.Len() ); z += csz*q.Len();
		}

		*this = t;

		XXXCHECK(this);
	}
}



/* ----	split text at separator ---------------------------
		list:	handled recursively
		text:	split
		other:	skipped
		""			-> { }					TODO: this should be { "" }
		"c"			-> { "" }				TODO: this should be { "","" }
		"---"		-> { "---" }
		"---c---"	-> { "---", "---" }
		"c---"		-> { "", "---" }
		"---c"		-> { "---", "" }
*/
void Var::SplitLines ( cString& sep )
{
	XXCHECK(this);

	switch(get_type())
	{
	default:
		break;

	case isList:
		for( uint i=list().used; i; )
		{
			Var* v = list().array[--i]; if(v) v->SplitLines(sep);
		};
		break;

	case isText:
		if (data_is_locked()) { set_error_locked(); break; }

		String s = text();

		int32 sep_len = sep.Len();
		int32 qi = 0;
		uint li = 0;

		SetList(s.Len()/(39+sep_len));

		switch(sep_len)
		{
		case 0:
			break;
		case 1:
			for( UCS4Char c=sep[0];; )
			{
				int32 ni = s.Find(c,qi); if(ni<0) break;
				new Var( this,li++, s.SubString(qi,ni) );
				qi = ni + 1;
			}
			break;
		default:
			for( ;; )
			{
				int32 ni = s.Find(sep,qi); if(ni<0) break;
				new Var( this,li++, s.SubString(qi,ni) );
				qi = ni + sep_len;
			}
			break;
		}
		new Var( this,li++, s.MidString(qi) );
		shrink_list(li);
		break;
	}
}


/* ----	split text at line breaks ---------------------------
		expands on lists
		ignores non-text
*/
void Var::SplitLines ( )
{
	XXCHECK(this);

	switch(get_type())
	{
	case isList:
		for( uint i=list().used; i; )
		{
			Var* v = list().array[--i]; if(v) v->SplitLines();
		};
		break;

	default:
		break;

	case isText:
		if (data_is_locked()) { set_error_locked(); break; }

		String s = text();
	//	int32 slen = s.Len();	Log(" s.len=%li ",slen);
		SetList(s.Len()/40);
		int32 qi=0;
		uint li=0;

		for ( int32 i=0; i<s.Len(); )
		{
	//		TRAP(s.Len()!=slen);
			UCS4Char c = s[i++];
	//		Log("%c",c>=' '&&c<127 ? (char)c : '.');
			if( c>13 || (c!=10&&c!=13&&c!=0) ) continue;
			new Var( this,li++, s.SubString(qi,i-1) );
			if( c!=0 && i<s.Len() && c+s[i]==23 ) i++;
			qi = i;
		}

		new Var( this,li++, s.MidString(qi) );
		shrink_list(li);
		break;
	}
}





/* ---- <list> <op>= <number> ----------------------------------------
*/
Var& Var::list_op ( Var&(Var::*op)(Double), Double q )
{
	XXXCHECK(this);
	xxassert(IsList());
	//if (data_is_locked()) { set_error_locked(); }		<-- nicht, weil die liste selbst nicht verändert wird.

	Var** a = list().array;
	uint  n = list().used;
	while (n--)
	{
		Var* p=a[n]; if (p==nullptr) p = new Var(this,n);
		(p->*op)(q);
	}
	return *this;
}

Var& Var::operator+= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value()+=q; return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator+=, q );
}

Var& Var::operator-= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value()-=q; return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator-=, q );
}

Var& Var::operator*= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value()*=q; return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator*=, q );
}

Var& Var::operator/= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value()/=q; return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator/=, q );
}

Var& Var::operator%= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value()=fmod(value(),q); return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator%=, q );
}

Var& Var::operator>>= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value() = ldexp(value(),-int(floor(0.5+q))); /* uint32(value())>>int(q); */ return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator>>=, q );
}

Var& Var::operator<<= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value() = ldexp(value(),int(floor(0.5+q))); /* uint32(value())<<int(q); */ return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator<<=, q );
}

//#define NEG(D) signbit(D)
//#define POS(D) !signbit(D)
#define NEG(D) (D<0)

Var& Var::operator&= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value() = llong(q) & llong(value()); return *this; }
//#if defined(_I386)
//		a:	value() = NEG(q) && NEG(value()) ? Double(  llong(q) &  llong(value()) )
//											 : Double( ullong(q) & ullong(value()) );		  return *this; }
//#else
//		a:	value() = NEG(q) ? NEG(value()) ? Double( llong(-ullong(-q) &-ullong(-value())))
//											: Double(ullong(-ullong(-q) & ullong( value())))
//							 : NEG(value()) ? Double(ullong( ullong( q) &-ullong(-value())))
//											: Double(ullong( ullong( q) & ullong( value()))); return *this; }
//#endif
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator&=, q );
}

Var& Var::operator|= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value() = llong(q) | llong(value()); return *this; }
//#ifdef _I386
//		a:	value() = NEG(q) || NEG(value()) ? Double(  llong(q) |  llong(value()) )
//											 : Double( ullong(q) | ullong(value()) );		  return *this; }
//#else
//		a:	value() = NEG(q) ? NEG(value()) ? Double( llong(-ullong(-q) |-ullong(-value())))
//											: Double( llong(-ullong(-q) | ullong( value())))
//							 : NEG(value()) ? Double( llong( ullong( q) |-ullong(-value())))
//											: Double(ullong( ullong( q) | ullong( value()))); return *this; }
//#endif
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator|=, q );
}

Var& Var::operator^= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value() = llong(q) ^ llong(value()); return *this; }
//#ifdef _I386
//		a:	value() = NEG(q) == NEG(value()) ? Double( ullong(q) ^ ullong(value()) )
//											 : Double(  llong(q) ^  llong(value()) );		  return *this; }
//#else
//		a:	value() = NEG(q) ? NEG(value()) ? Double(ullong( ullong(-q) ^ ullong(-value())))
//											: Double( llong(-ullong(-q) ^ ullong( value())))
//							 : NEG(value()) ? Double( llong( ullong( q) ^-ullong(-value())))
//											: Double(ullong( ullong( q) ^ ullong( value()))); return *this; }
//#endif
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator^=, q );
}


/* ---- <var> <op>= <list> ----------------------------------------
*/
Var& Var::list_op ( Var&(Var::*op)(Double), cVar& q)
{	Var **qa,**za,*qp,*zp;
	uint n,m;

	xxassert(q.IsList());

	qa = q.list().array;
	n  = q.list().used;

	if (IsList())
	{
	//	if (data_is_locked()) { set_error_locked(); goto x; }	<-- nein, liste selbst wird nicht verändert!

		if(q.Contains(this)||this->Contains(q)) if(this!=&q)return list_op(op,Var(q));

	// handling of remaining items if lists where not same size
		m = list().used;
		if(m<n)
		{	if (data_is_locked()) { set_error_locked(); goto x; }
			while(m<n) new Var(this,m++);
		}
		za = list().array;
		while(m>n) (za[--m]->*op)(0.0);

	//	handle common size
		while(n--)
		{
			if (!(zp=za[n])) zp = new Var(this,n);
			if (!(qp=qa[n])) qp = zero;
			if (qp->IsNumber()) { (zp->*op)(qp->value()); continue; }
			if (qp->IsNoList()) { (zp->*op)(qp->ToNumber()); continue; }
			zp->list_op(op,*qp);
		}
	}
	else
	{
		if (type_or_data_locked()) { set_error_locked(); goto x; }

		if(q.Contains(this)) return list_op(op,Var(q));

		float f = ToNumber();
		SetList(n);
		za = list().array;
		while(n--)
		{
			if (!(qp=qa[n])) qp=zero;
			zp = new Var(this,n, f, qp->name);

			if (qp->IsNumber()) { (zp->*op)(qp->value()); continue; }
			if (qp->IsNoList()) { (zp->*op)(qp->ToNumber()); continue; }
			zp->list_op(op,*qp);
		}
	}

x:	return *this;
}


/* ----	<var> <op>= <var> -----------------------------------
		<var> <op> <number>
		<var> <op> <var>
*/
#define OPERATOR(OP,OPG)											\
Var& Var::OPG ( cVar& q )											\
{																	\
	if (q.IsNumber()) return OPG(q.value());						\
	if (q.IsNoList()) return OPG(q.ToNumber());						\
	return list_op(&Var::OPG,q);									\
}																	\
Var Var::OP ( Double q ) const { Var z(*this); z.OPG(q); return z; }\
Var Var::OP ( cVar& q  ) const { Var z(*this); z.OPG(q); return z; }

OPERATOR(operator +, operator +=)
OPERATOR(operator -, operator -=)
OPERATOR(operator *, operator *=)
OPERATOR(operator /, operator /=)
OPERATOR(operator %, operator %=)
OPERATOR(operator >>,operator >>=)
OPERATOR(operator <<,operator <<=)
OPERATOR(operator &, operator &=)
OPERATOR(operator |, operator |=)
OPERATOR(operator ^, operator ^=)

#undef OPERATOR


// *********************************************************
//				TEXT CONCATENATION
// *********************************************************

/* ---- <var> #= <list> ------------------------------------
*/
Var& Var::append_list ( cVar& q, DisassProcPtr disass_proc )
{	Var **qa,**za,*qp,*zp;
	uint n,m;

	xxassert(q.IsList());

	qa = q.list().array;
	n  = q.list().used;

	if (IsList())	// list #= list
	{
		//if (data_is_locked()) { set_error_locked(); goto x; }	<-- nein, liste selbst wird nicht verändert.

		m = list().used;
		if(m<n)
		{	if (data_is_locked()) { set_error_locked(); goto x; }
			while(m<n) new Var( this,m++, emptyString );
		}
		za = list().array;
		while(m>n) za[--m]->AppendString(emptyString,disass_proc);

		while(n--)
		{
			if (!(zp=za[n])) zp = new Var(this,n);	// nullptr <=> 0.0
			if (!(qp=qa[n])) qp = zero;			// nullptr <=> 0.0

			if (qp->IsText())   { zp->AppendString(qp->text(),disass_proc); continue; }
			if (qp->IsNoList()) { zp->AppendString(qp->ToString(no,disass_proc),disass_proc); continue; }
			zp->append_list(*qp,disass_proc);
		}
	}
	else
	{
		if (type_or_data_locked()) { set_error_locked(); goto x; }

		String f = ToString(no,disass_proc);
		SetList(n);
		za = list().array;
		while(n--)
		{
			if (!(qp=qa[n])) qp = zero;
			zp = new Var(this,n, f,qp->name);

			if (qp->IsText())   { zp->text() += qp->text(); continue; }
			if (qp->IsNoList()) { zp->text() += qp->ToString(no,disass_proc); continue; }
			zp->append_list(*qp,disass_proc);
		}
	}

x:	return *this;
}


/* ----	<var> #= <text> --------------------------------------------
*/
Var& Var::AppendString ( cString& q, DisassProcPtr disass_proc )
{
	if (IsText()) { if(data_is_unlocked()) text()+=q; else set_error_locked(); }
	else if (IsNoList()) { *this = ToString(no,disass_proc) + q; }	// note: operator=() checks locks
	else
	{
		Var** a = list().array;
		uint  n = list().used;
		while (n--) ( a[n] ? a[n] : new Var(this,n) )->AppendString(q,disass_proc);
	}
	return *this;
}

/* ----	<var> #= <var> --------------------------------------------
*/
Var& Var::AppendString ( cVar& q, DisassProcPtr disass_proc )
{
	if (q.IsText())   return AppendString(q.text(),disass_proc);
	if (q.IsNoList()) return AppendString(q.ToString(no,disass_proc),disass_proc);
	return append_list(q,disass_proc);
}

/* ----	<var> # <text> ---------------------------------------------
*/
Var Var::ConcatString ( cString& q, DisassProcPtr disass_proc ) const
{
	Var z(*this); z.AppendString(q,disass_proc); return z;
}

/* ----	<var> # <var> -----------------------------------------------
*/
Var Var::ConcatString ( cVar& q, DisassProcPtr disass_proc ) const
{
	Var z(*this); z.AppendString(q,disass_proc); return z;
}





/* ====	Boolean =========================================
	note: reimplementing booleans eliminates pruning!
	note: if list-expansion is desired, then the return data type must be Var
*/
#ifdef VAR_INCLUDE_BOOLEAN

bool Var::operator&& ( bool q ) const
{
	if (!q)         return 0;
	if (IsNumber()) return value();
	if (IsNoList()) return ToNumber();
	SetError(notanumber); return 0;
}

bool Var::operator&& ( cVar& q ) const
{
	if (  IsNumber()) return   q     &&   value();
	if (q.IsNumber()) return (*this) && q.value();
	if (  IsNoList()) return   q     &&   ToNumber();
	if (q.IsNoList()) return (*this) && q.ToNumber();
	SetError(notanumber); return 0;
}

bool Var::operator|| ( bool q ) const
{
	if (q)          return 1;
	if (IsNumber()) return value();
	if (IsNoList()) return ToNumber();
	SetError(notanumber); return 0;
}

bool Var::operator|| ( cVar& q ) const
{
	if (  IsNumber()) return   q     ||   value();
	if (q.IsNumber()) return (*this) || q.value();
	if (  IsNoList()) return   q     ||   ToNumber();
	if (q.IsNoList()) return (*this) || q.ToNumber();
	SetError(notanumber); return 0;
}

#endif	// VAR_INCLUDE_BOOLEAN



extern cstr* environ;

Var* NewEnvironmentVar ( bool lowercase )
{
	Var* env = new Var(isList);

	for ( cstr* s=environ; *s; s++ )
	{
		cptr t  = strchr(*s,'='); if (!t) continue;
		str name = substr(*s,t); if(lowercase) ToLower(name);
		env->AppendItem( new Var(t+1, name ) );
	}
	return env;
}






























