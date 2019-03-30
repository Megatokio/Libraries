/*	Copyright  (c)	Günter Woigk   2001-2005
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
	2003-10-12 kio	fixed handling of NULL in lists for string concatenation
	2003-10-18 kio	fixed list-, list size-, and NULL in list-related issues
	2004-05-28 kio	Umstellung auf NameHandles
	2004-06-29 kio	added variable and data locking

	---------
	Class Var
	---------

	maintain named variables of different types
*/


#define 	SAFE	3
#define 	LOG		1

#include	"config.h"
#include	<math.h>
#include	"var.h"
#include	"Unicode/Unicode.h"
#include	"templates/Stack.h"


#ifndef NAN
const Double NAN = 0.0/0.0;
#endif

#ifdef DEBUG_INIT_MSG
DEBUG_INIT_MSG("Var <init>");
#endif



// 	0.0
//
//	benötigt Initialisierung!
//	verwendet in:
//		<list>.compare(<list>)
//		<list>.MaxVar()
//		<list>.MinVar()
//		<list>.list_op(<fu>,<list>)
//		<list>.append_list(<list>,<disassproc>)
Var* zero = NULL;
Var* eins = NULL;

struct InitVar
{	InitVar()
	{	zero = new Var(0.0); zero->lock_data(); zero->lock();
		eins = new Var(1.0); eins->lock_data(); eins->lock();
	}
} 	InitVarObj;


/* ----	variables pool -------------------------------------
		new() gets new slots from the pool
		pool empty is detected by first item==NULL
		delete() returns slot to pool
		pool size is always == amount of all variables ever allocated
		=> return of all variables ever released to pool possible w/o test
		if pool is empty, a new chunk of variables is allocated
		and the pool is increased to the new amount of variables allocated
		allocated chunks are never deallocated.
*/
static Var* no_var		= NULL;
static uint	pool_size	= 1;

Var**  Var::pool		= (&no_var)+1;

Var*   Var::grow_pool ( )
{
	XXLogIn("Var::grow_pool()");

	if(pool!=&no_var) delete[] pool;

	uint n = 500 + pool_size/2;
	pool = new Var*[pool_size+=n];
	Var* var = (Var*)new char[n*sizeof(Var)];

	*pool++ = NULL;
	while(n--) *pool++ = var++;

	XXLogOut();
	return *--pool;
}




/* ----	validate variables ----------------------------------------
*/
void Var::validate ( cstr file, long line ) const
{
	#define VTRAP(X) if (!(X))	{} else Abort ( file, line, Using("internal error@%d: %s",__LINE__,#X) )

// test global prerequiste: Double fits in data[]:
	TRAP(sizeof(Double)>sizeof(data));
	TRAP(sizeof(String)>sizeof(data));
	TRAP(sizeof(Vek)>sizeof(data));

	VTRAP(!this);

	//Log("(%lu)",name);
	GetNameForHandle(name)._check(file,line);

	if (parent)
	{
		VTRAP((int)index<0);
		VTRAP(parent->IsNoList());
		VTRAP(parent->list().array==NULL);
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
		text()._check(file,line);
	}

	#undef VTRAP
}

#define XXXXCHECK(v)if(XXXXSAFE)(v)->validate(__FILE__,__LINE__)
#define XXXCHECK(v)	if(XXXSAFE) (v)->validate(__FILE__,__LINE__)
#define XXCHECK(v)	if(XXSAFE)  (v)->validate(__FILE__,__LINE__)
#define XCHECK(v)	if(XSAFE)   (v)->validate(__FILE__,__LINE__)
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
	XXTRAP(obj->varptr!=NULL);

	handle().next = handle().prev = this;
	handle().data = obj;

	obj->varptr = this;
	obj->type = type;
}


void Var::init_handle ( const Var& q )
{
	handle().next = q.handle().next;
	handle().prev = handle().next->handle().prev;
	XXXASSERT(handle().prev==&q);
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
	XXXTRAP ( par==NULL || par->IsNoList() );
	XXXTRAP ( par->list().used<=idx || par->list().array[idx]!=NULL );

	parent=par; index=idx; par->list().array[idx]=this;
}

/* ----	link item into parent's list ---------------------------------
		grow list if required
		slot must be free
		handles idx==at_end
*/
inline void Var::init_link_grow ( Var* par, uint idx )
{
	XXXTRAP ( par==NULL || par->IsNoList() );

	if (idx>=par->list().used)
	{
		if(idx==at_end) idx=par->list().used;
		par->grow_list(idx+1);
	}
	else
	{
		XXXTRAP(par->list().array[idx]!=NULL);
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
	XXXTRAP ( par==NULL || par->IsNoList() );

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
	XXTRAP   ( par==NULL || par->IsNoList() );
	XXTRAP   ( (idx>>26) && idx!=at_end );

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

xl:	SetError(listsintersect); return;
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
	XXTRAP   ( parent==NULL );

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
		while( i && a[i-1]==NULL ) i--;

		unlock();
	}
}

/* ----	Unlink and close gap ---------------------------
*/
void Var::Vanish()
{
	XXXCHECK ( this );
	XXTRAP   ( parent==NULL );

	parent->DeleteItem(index);
}






/* ----	recursively lock variable type or data ---------------------------------
*/
void Var::setmask ( ulong mask )
{
	XXXTRAP(IsNoList());

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
void Var::resmask ( ulong mask )
{
	XXXTRAP(IsNoList());

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
	list().array = NULL;
	list().size  = 0;
	list().used  = 0;
}


/* ----	create copy of source vektor -----------------------
*/
void Var::init_list ( cVek& q )
{
	XXXLogIn("Var::init_list(Vek)");

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

	XXXLogOut();
}


/* ----	make list invalid -------------------------------------
		löscht alle variablen im array und delete[] array
		außer: size==0 => subslicer => don't destroy original!
		in:  data valid
		out: data void
*/
void Var::kill_list ( )
{
	XXXLogIn("Var::kill_list()");

	if (list().size!=0)
	{
		XXXTRAP(list().used>list().size);
		XXXTRAP(list().array==NULL);

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

	XXXLogOut();
}



/* ----	grow list ------------------------------------------
		grow list => loose fit
*/
void Var::grow_list ( uint newused )
{
	XXXLogIn("Var::grow_list()");

	XXXTRAP(newused>>26);			// oorange security

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
		list().array[list().used++] = NULL; 	// händisch 'createn'
	}

	XXXLogOut();
}


/* ----	shrink list ------------------------------------------
		shrink list => tight fit
*/
void Var::shrink_list ( uint newused )
{
	XXXLogIn("Var::shrink_list()");

	XXXTRAP(newused>list().used);

	if (list().size>newused)		// !subslicer && !already_tight
	{
		Var** oldarray = list().array;

		while (list().used>newused)			// händisch destroyen
		{
			Var* v = oldarray[--list().used];
			if (v) { if(v->parent==this) { v->kill_link_nop(); v->init_link(); } v->unlock(); }
		}
		list().size = newused;
		Var** newarray = NULL;

		if (newused>0)
		{
			newarray = new Var*[newused];
			memcpy ( newarray, oldarray, newused*sizeof(Var*) );			// move items flat
		}

		delete[] oldarray;
		list().array = newarray;
	}

	XXXLogOut();
}


/* ----	Insert empty items into list -------------------------------
		note: inserted items are set to NULL == Var("",0.0)
*/
void Var::insert_items ( uint idx, uint n )
{
	XXXLogIn("Var::insert_items(idx,n)");
	XXXTRAP( (n>>26) || (idx>>26) );

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
			array[m] = NULL;
		}
	}

	XXXLogOut();
}


/* ----	Remove items from list -------------------------------
		fast, no shrink to fit
*/
void Var::delete_items ( uint idx, uint n )
{
	XXXLogIn("Var::delete_items(idx,n)");
	XXXTRAP(n>>26);

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

	XXXLogOut();
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
	XXXLogIn("Var::kill_data()");

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

	XXXLogOut();
}


/* ----	init data -------------------------------
		in:	 data invalid
		out: data valid: default value for type
*/
void Var::init_data ( vtype typ )
{
	XXXLogIn("Var::init_data(vtype)");

	init_type(typ);

	switch(typ)
	{
	default: 		IERR();
	case isText: 	init_text();  break;
	case isList:	init_list();  break;
	case isNumber:	init_value(); break;
	}

	XXXLogOut();
}


/* ----	copy source data from target data -----------------------------
		in:   data invalid  q.data valid
		out:  data valid	q.data valid
*/
void Var::init_data ( cVar& q )
{
	XXXLogIn("Var::init_data(Var)");

	XXXTRAP  ( !this || !&q );
	XXXTRAP  ( &q==this );

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

	XXXLogOut();
}


/* ----	move source data from target data -----------------------------
		in:   data invalid  q.data valid
		out:  data valid	q.data invalid
*/
void Var::move_data ( Var& q )
{
	XXXLogIn("Var::move_data(Var)");

	XXXTRAP  ( !this || !&q );
	XXXTRAP  ( &q==this );

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

	XXXLogOut();
}


/* ----	swap source data with target data -----------------------------
		in:   data valid	q.data valid
		out:  data valid	q.data valid
		note: lists must not intersect or a circular list will result
*/
void Var::swap_data ( Var& q )
{
	XXXLogIn("Var::swap_data(Var)");

	Var z;
	z.kill_type();				// nop
	z.kill_value();				// nop

	z.move_data(q);
	q.move_data(*this);
	  move_data(z);

	z.init_type_and_lock();		// => can be safely destroyed

	XXXLogOut();
}


/* ----	creator, destructor ---------------------------------------------------
*/
Var::Var ( Var* par, uint idx )
{
	XXXLogIn("Var::Var(par,idx)");
	XXCHECK(par);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock();
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Var* par, uint idx, vtype typ )
{
	XXXLogIn("Var::Var(par,idx,type)");
	XXCHECK(par);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock(typ);
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Var* par, uint idx, Double val )
{
	XXXLogIn("Var::Var(par,idx,number)");
	XXCHECK(par);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock(val);
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Var* par, uint idx, cString& txt )
{
	XXXLogIn("Var::Var(par,idx,txt)");
	XXCHECK(par);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock(txt);
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Var* par, uint idx, cVar& q )
{
	XXXLogIn("Var::Var(par,idx,Var)");
	XXCHECK(par);
	XXCHECK(&q);

	init_name();
	init_link_force(par,idx);
	init_data_and_lock(q);
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}



Var::Var ( Var* par, uint idx, vtype typ, NameHandle nh )
{
	XXXLogIn("Var::Var(par,idx,type,name)");
	XXCHECK(par);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(typ);
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Var* par, uint idx, Double val, NameHandle nh )
{
	XXXLogIn("Var::Var(par,idx,number,name)");
	XXCHECK(par);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(val);
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Var* par, uint idx, cString& txt, NameHandle nh )
{
	XXXLogIn("Var::Var(par,idx,text,name)");
	XXCHECK(par);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(txt);
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Var* par, uint idx, Stream* Stream, NameHandle nh )
{
	XXXLogIn("Var::Var(par,idx,stream,name)");
	XXCHECK(par);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(Stream);
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Var* par, uint idx, cVar& q, NameHandle nh )
{
	XXXLogIn("Var::Var(par,idx,Var,name)");
	XXCHECK(par);
	XXCHECK(&q);

	init_name(nh);
	init_link_force(par,idx);
	init_data_and_lock(q);
	incr_lock();

	XXXXCHECK(this);
	XXXLogOut();
}



Var::Var ( vtype typ )
{
	XXXLogIn("Var::Var(type)");

	init_name();
	init_link();
	init_data_and_lock(typ);

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Double val )
{
	XXXLogIn("Var::Var(number)");

	init_name();
	init_link();
	init_data_and_lock(val);

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( cString& txt )
{
	XXXLogIn("Var::Var(text)");

	init_name();
	init_link();
	init_data_and_lock(txt);

	XXXXCHECK(this);
	XXXLogOut();
}

/*
Var::Var ( cVar& q )
{
	XXXLogIn("Var::Var(Var)");

	init_name();
	init_link();
	init_data_and_lock(q);

	XXXXCHECK(this);
	XXXLogOut();
}
*/


Var::Var ( vtype typ, NameHandle nam )
{
	XXXLogIn("Var::Var(type,name)");

	init_name(nam);
	init_link();
	init_data_and_lock(typ);

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( Double val, NameHandle nam )
{
	XXXLogIn("Var::Var(number,name)");

	init_name(nam);
	init_link();
	init_data_and_lock(val);

	XXXXCHECK(this);
	XXXLogOut();
}

Var::Var ( cString& txt, NameHandle nam )
{
	XXXLogIn("Var::Var(txt,name)");

	init_name(nam);
	init_link();
	init_data_and_lock(txt);

	XXXCHECK(this);
	XXXLogOut();
}

Var::Var ( cVar& q, NameHandle nam )
{
	XXXLogIn("Var::Var(Var,name)");

	init_name(nam);
	init_link();
	init_data_and_lock(q);

	XXXXCHECK(this);
	XXXLogOut();
}



/* ----	List Var from cstr array ----------------------------
		list terminates after argc elements or at NULL
*/
Var::Var ( char const*const* argv, uint n )
{
	XXLogIn("Var::Var(argv[],argc)");

	init_name();
	init_link();
	init_data_and_lock(isList);

	cstr s;
	for ( uint i=0; i<n && (s=argv[i]); i++ )
	{
		new Var( this,i, s );
	}

	XXXXCHECK(this);
	XXLogOut();
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
	XXXLogIn("Var::operator=(Var)");

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

	XXXLogOut();
	return *this;
}


/* ----	quick copy data from source ----------------------------------
		Daten dürfen sich nicht überschneiden!
*/
void Var::CopyData ( Var& q )
{
	XXXLogIn("Var::CopyData(Var)");

	XXCHECK ( this );
	XXCHECK ( &q );
	XTRAP   (   IsList() &&   Contains(q)    );
	XTRAP   ( q.IsList() && q.Contains(this) );

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

	XXXLogOut();
}


/* ----	quick move data from source ----------------------------------------
		Daten dürfen sich nicht überschneiden!
*/
void Var::MoveData ( Var& q )
{
	XXXLogIn("Var::MoveData(Var)");

	XXCHECK ( this );
	XXCHECK ( &q );
	XTRAP   (   IsList() &&   Contains(q)    );
	XTRAP   ( q.IsList() && q.Contains(this) );
	XTRAP   ( q.type_or_data_locked() && (q.data_is_locked() || q.get_type()!=isNumber  ) );

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

	XXXLogOut();
}


/* ----	quick swap data with source ----------------------------------
		Daten dürfen sich nicht überschneiden!
*/
void Var::SwapData ( Var& q )
{
	XXXLogIn("Var::SwapData(Var)");

	XXCHECK ( this );
	XXCHECK ( &q );

	if( Parent()!=q.Parent() && ((IsList()&&Contains(q))||(q.IsList()&&q.Contains(this))) )
	{
		SetError(listsintersect);
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

	XXXLogOut();
}


/* ----	resize list ------------------------------------------
		grow list   => loose fit
		shrink list => tight fit
*/
void Var::ResizeList ( uint newused )
{
	XXXLogIn("Var::ResizeList()");

	XXCHECK(this);
	XTRAP(IsNoList());

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

	XXXLogOut();
}


/* ----	operator[] -------------------------------------------------
		grows list and creates item if required
		detects idx==at_end
*/
Var& Var::operator[] ( uint idx )
{
	XXXLogIn("Var::operator[]");

	XXXCHECK(this);
	XTRAP(IsNoList());

	if (idx>=list().used)
	{
		if(data_is_locked())
		{
			set_error_locked();
			XXXLogOut();
			return *this;
		}
		if(idx==at_end) idx=list().used;
		XXTRAP(idx>>26);
		grow_list(idx+1);
	}

	Var* v = list().array[idx];
	if(!v) v = new Var(this,idx);

	XXXLogOut();
	return *v;
}


/* ----	find item by name --------------------------------------------------
		find named item in list
		returns found Var or NULL
		named item is searched starting at the end of the list
*/
Var* Var::FindItem ( NameHandle namehandle ) const
{
	XXXLogIn("Var::FindItem(name)");
	XXXCHECK(this);

	if (IsList())
	{
		Var** a = list().array;
		Var** e = a+list().used;

		while ( e > a )
		{
			Var* v = *--e;
			if (v) { if (namehandle!=v->name) continue; }
			else   { if (namehandle!=0) continue; else v = new Var((Var*)this,e-a); }
			XXXLogOut();
			return v;
		}
	}

	XXXLogOut();
	return NULL;
}


Var* Var::FindItem ( cString& nam ) const
{
	NameHandle n = FindNameHandle(nam);
	return  n!=0 || nam.Len()==0  ? FindItem( n ) : NULL;
}


Var* Var::FindItem ( cstr nam ) const
{
	NameHandle n = FindNameHandle(nam);
	return  n!=0 || nam[0]==0  ? FindItem( n ) : NULL;
}


/* ----	Search for Item ------------------------------------ [2004-06-10 kio]
		search for Item in List
		return index of item found or -1 if not found
*/
long Var::Find ( cVar& v, long startidx ) const
{
	XXXLogIn("Var::Find(Var)");

	XXCHECK(this);
	XXXCHECK(&v);

	if (IsList())
	{
		bool visnull = v.IsNumber()&&v.value()==0.0;
		if (startidx<0) startidx=0;
		long endidx  = list().used;

		while(startidx<endidx)
		{
			Var* a = list().array[startidx++];
			if (a==NULL ? visnull : v.get_type()==a->get_type() && v.compare(*a)==0)
			{
				XXXLogOut();
				return startidx-1;
			}
		}
	}
	else SetError(listrequired);

	XXXLogOut();
	return -1;
}

long Var::RFind ( cVar& v, long startidx ) const
{
	XXXLogIn("Var::RFind(Var)");

	XXCHECK(this);
	XXXCHECK(&v);

	if (IsList())
	{
		bool visnull = v.IsNumber()&&v.value()==0.0;
		if (startidx>=(long)list().used) startidx=list().used-1;

		while(startidx>=0)
		{
			Var* a = list().array[startidx--];
			if (a==NULL ? visnull : v.get_type()==a->get_type() && v.compare(*a)==0)
			{
				XXXLogOut();
				return startidx+1;
			}
		}
	}
	else SetError(listrequired);

	XXXLogOut();
	return -1;
}


/* ----	Insert empty items into list -------------------------------
		if idx>used then nothing is done, because accessing
		items beyond used automatically creates them
		note: inserted items are set to NULL == Var("",0.0)
		handles idx==at_end
*/
void Var::InsertItems ( uint idx, uint n )
{
	XXXLogIn("Var::InsertItems(idx,n)");

	XXCHECK(this);
	XTRAP(IsNoList());

	if(idx==at_end) idx=list().used;
	XXTRAP(idx>>26||n>>26);

	if(data_is_locked())
	{
		set_error_locked();
	}
	else
	{
		insert_items(idx,n);
		XXXCHECK(this);
	}

	XXXLogOut();
}


/* ----	insert item into list ----------------------------
		handles idx==at_end
*/
void Var::InsertItem ( uint idx, Var* q )
{
	XXXLogIn("Var::InsertItem(idx,Var)");

	XXCHECK ( this );
	XXCHECK ( q );
	XXTRAP  ( q->IsList() && q->Contains(this) );
	XTRAP   ( IsNoList() );

	if(idx==at_end) idx=list().used;
	XXTRAP(idx>>26);

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

	XXXLogOut();
}


void Var::AppendItems ( Var* a, Var* b, Var* c, Var* d )
{
	XXLogIn("Var::AppendItems(4)");
	XXCHECK (this);
	XTRAP(IsNoList());
	XXXTRAP( d && d->IsStream() && ( /*d->GetStream()->InputEncoding()<ucs1 ||*/ d->GetStream()->InputEncoding()>utf8||
							         /*d->GetStream()->OutputEncoding()<ucs1||*/ d->GetStream()->OutputEncoding()>utf8));

	if(data_is_locked())
	{
		set_error_locked();
	}
	else
	{
#define APPEND(V)													\
	if(V) { XXTRAP(V->Contains(this)); XXTRAP(V->is_linked());		\
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

	XXXTRAP( d && d->IsStream() && ( /*d->GetStream()->InputEncoding()<ucs1 ||*/ d->GetStream()->InputEncoding()>utf8||
							         /*d->GetStream()->OutputEncoding()<ucs1||*/ d->GetStream()->OutputEncoding()>utf8));
	XXLogOut();
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
	XXXLogIn("Var::DeleteItems(idx,n)");

	XXCHECK (this);
	XTRAP   (IsNoList());

	if(data_is_locked())
	{
		set_error_locked();
	}
	else
	{
		delete_items(idx,n);
		XXXCHECK(this);
	}

	XXXLogOut();
}


/* ----	remove last item from list ---------------------------
		fast, no shrinking of array
*/
void Var::DeleteLastItem ( )
{
	XXXLogIn("Var::DeleteLastItem()");

	XXCHECK (this);
	XTRAP   (IsNoList());


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

	XXXLogOut();
}


/* ----	append list q to this list --------------------------
		in:  this must be a lists; else it is casted to list
		out: the contents of q is appended to this list
			 if q is no list, then q itself is appended to this list
*/
void Var::AppendListItems ( Var& q )
{
	XXXLogIn("Var::AppendList(Var)");

	XXXCHECK(&q);
	XXXCHECK(this);
	XXTRAP  ( q.Contains(this) );

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
x:	XXXLogOut();
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
	XXXLogIn("Var::compare(Var)");

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
			XXXLogOut();
			return a>b ?  1 : a<b ? -1 : a==b ? 0 : NAN;
		}

		case isProc:	XXXLogOut(); return ssize_t( proc().ip - q.proc().ip);
		case isText:	XXXLogOut(); return text().compare(q.text());

		case isList:
		{	uint n = Min(list().used,q.list().used);
			for (uint i=0; i<n; i++)
			{
				Var* a = list().array[i];	if (!a) a=zero;
				Var* b = q.list().array[i];	if (!b) b=zero;
				Double r = a->compare(*b);
				if (r==0) continue;
			//	if(Error()&&Error()!=typemismatch) ClearError();	// ***TODO*** setzen NANs errno ?
				XXXLogOut();
				return r;		// >, <, or unsortable
			}
		}	XXXLogOut(); return int(list().used) - int(q.list().used);

		case isStream:	XXXLogOut(); return ssize_t( stream() - q.stream() );
		case isIrpt:	XXXLogOut(); return ssize_t( irpt() - q.irpt() );
		case isSema:	XXXLogOut(); return ssize_t( sema() - q.sema() );
		case isThread:	XXXLogOut(); return ssize_t( thread() - q.thread() );
		case isVarRef:	XXXLogOut(); return varptr()->compare(*q.varptr());
		default: 		IERR();
		}
	}
	if(IsVarRef()) return varptr()->compare(q);
	if(q.IsVarRef()) return compare(*q.varptr());

	SetError(typemismatch);
	XXXLogOut();
	return int(get_type()) - int(q.get_type());		// mixed type
}


/* ----	sort list -------------------------------------------------------
*/
#undef  SWAP

#define TYPE		Var*
#define BEHIND(A,B)	(*(A)>*(B))
#define SWAP(A,B)	Swap((A),(B))
#define SORTER		Var::Sort

#include "sort/threshold-sorter-2.h"

#define TYPE		Var*
#define BEHIND(A,B)	(*(A)<*(B))
#define SWAP(A,B)	Swap((A),(B))
#define SORTER		Var::RSort

#include "sort/threshold-sorter-2.h"


void Var::sort( SortProcPtr sorter )
{
	XXXLogIn("Var::sort()");
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
			for(uint i=n;i--;) { if(qlist[i]==NULL) qlist[i] = zero; }

		// doit
			// use temp array, if compare() performs CHECK()
			// because sorter() only swaps pointers and not also item.index
			#if XXXSAFE
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
				else if(qlist[i]==zero) { qlist[i] = NULL; }
			}

			//ClearError();	// clear type mismatchs

			XXXCHECK(this);
		}
	}

	XXXLogOut();
}


/* ----	shuffle list ----------------------------------- [2004-06-21 kio]
*/
void Var::Shuffle()
{
	XXXLogIn("Var::Shuffle()");
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
				if (i!=j) Swap(array[i],array[j]);
				if(array[i]) array[i]->index = i;
			}
		}
	}

	XXXLogOut();
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
	case isText:	Log(Using("%s ",CString(text().ToQuoted())));	break;
	case isProc: 	Log("proc ");									break;

	case isList:
		Log("{ ");

		for ( uint i=0; i<list().used; i++ )
		{
			Var* v = list().array[i];
			if (v && v->IsNamed()) Log(Using("%s=",CString(v->Name())));
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
	SetError(notanumber); return NAN;
}

void Var::ConvertToNumber ( )
{
	if(type_or_data_locked())
	{
		if(data_is_locked()||IsNoNumber()) { set_error_locked(); return; }
	}
	switch(get_type())
	{
	default:		set_data(NAN); SetError(notanumber); break;
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
		XXLogIn("Var::ToString(List)");
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
					for (int j=1;f&&j<v_name.Len();j++) { c = v_name[j]; f = UCS4CharIsLetter(c) || UCS4CharIsDecimalDigit(c) || c=='_'; }
					s += f ? sep + v_name					// add well formed name
						   : sep + '@'+v_name.ToQuoted();	// add malformed name
					sep = '=';
				}
				s += sep + v->ToString(yes,disass);			// recursively add item
			}
			else											// NULL == number 0.0
			{
				s += sep + '0';
			}

			sep = sep2;
		}

		XXLogOut();
		return s + " }";
	}
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


/* ----	split text at separator ---------------------------
		list:	handled recursively
		text:	split
		other:	skipped
		""			-> { }
		"c"			-> { "" }
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

		long sep_len = sep.Len();
		long qi = 0;
		uint li = 0;

		SetList(s.Len()/(39+sep_len));

		switch(sep_len)
		{
		case 0:
			break;
		case 1:
			for( UCS4Char c=sep[0];; )
			{
				long ni = s.Find(c,qi); if(ni<0) break;
				new Var( this,li++, s.SubString(qi,ni) );
				qi = ni + 1;
			}
			break;
		default:
			for( ;; )
			{
				long ni = s.Find(sep,qi); if(ni<0) break;
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
	//	long slen = s.Len();	Log(" s.len=%li ",slen);
		SetList(s.Len()/40);
		long qi=0;
		uint li=0;

		for ( long i=0; i<s.Len(); )
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
	XXXTRAP (IsNoList());
	//if (data_is_locked()) { set_error_locked(); }		<-- nicht, weil die liste selbst nicht verändert wird.

	Var** a = list().array;
	uint  n = list().used;
	while (n--)
	{
		Var* p=a[n]; if (p==NULL) p = new Var(this,n);
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
	if (IsNumber()) { if (data_is_locked()) goto e; a: value() = ldexp(value(),-int(floor(0.5+q))); /* ulong(value())>>int(q); */ return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator>>=, q );
}

Var& Var::operator<<= ( Double q )
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value() = ldexp(value(),int(floor(0.5+q))); /* ulong(value())<<int(q); */ return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator<<=, q );
}

Var& Var::operator&= ( Double q )		// TODO: work on full Double range
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value()=llong(value()) & llong(q); return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator&=, q );
}

Var& Var::operator|= ( Double q )		// TODO: full Double range
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value()=llong(value()) | llong(q); return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator|=, q );
}

Var& Var::operator^= ( Double q )		// TODO: full Double range
{
	if (IsNumber()) { if (data_is_locked()) goto e; a: value() = llong(value()) ^ llong(q); return *this; }
	if (IsNoList()) { if (type_or_data_locked()) e: set_error_locked(); else { ConvertToNumber(); goto a; } }
	return list_op( &Var::operator^=, q );
}


/* ---- <var> <op>= <list> ----------------------------------------
*/
Var& Var::list_op ( Var&(Var::*op)(Double), cVar& q)
{	Var **qa,**za,*qp,*zp;
	uint n,m;

	XXXTRAP(q.IsNoList());

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

	XXXTRAP(q.IsNoList());

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
			if (!(zp=za[n])) zp = new Var(this,n);	// NULL <=> 0.0
			if (!(qp=qa[n])) qp = zero;			// NULL <=> 0.0

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
		char* t  = strchr(*s,'='); if (!t) continue;
		str name = SubStr(*s,t); if(lowercase) ToLower(name);
		env->AppendItem( new Var(t+1, name ) );
	}
	return env;
}






























