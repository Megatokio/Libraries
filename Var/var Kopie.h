/*	Copyright  (c)	Günter Woigk   2001-2005
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


    ----------------------------------------------------------------------------

			class Var

    ----------------------------------------------------------------------------


	note on variable locking:

	new variables are unlocked. they may be deleted with delete.

	once a variable is locked, it will be immediately destroyed when the unlock count equals the lock count.
	note that variables are locked and unlocked as they are linked to resp. unlinked from a list variable.

	you _should_ lock variables for each pointer which holds a reference to it,
	and unlock it after use (instead of deleting it).

	when you can't guarantee that a variable will not be locked/unlocked by s.o. else
	then you _must_ lock it (and unlock it when you are finished).

	Use the inline functions Init(), Set() and Kill() or class VarPtr for locking.

		Init( Var*& target, Var* )							lock and store new Var*
		Set ( Var*& target, Var* )		unlock current Var, lock and store new Var*
		Kill( Var*& target )			unlock current Var
*/


#ifndef var_h
#define var_h


#include	"ustring/sstring.h"
#include	"NameHandles.h"

#ifdef VAR_LONG_DOUBLE
typedef	long double		Double;
typedef	Double const	cDouble;
#else
typedef	double			Double;
typedef	double const	cDouble;
#endif

class   Var;	typedef Var const	cVar;
class	Vek;	typedef Vek const	cVek;
class	Proc;	typedef	Proc const	cProc;
class 	VarPtr;	typedef VarPtr const cVarPtr;

typedef String(*DisassProcPtr)(cVar&);
typedef void(*SortProcPtr)(Var**,Var**);

#include "Stream.h"
#include "Sema.h"
#include "Irpt.h"
#include "Thread.h"
#include "BPObj.h"



/* ----	undef defines from "Menues.h" ----
*/
#undef SetItem
#undef GetItem
#undef setitem


/* ----	enum for Var::type_and_lock ----------------------------
*/
enum vtype
{			// data types
					isNumber = 0,		// float number
					isText, 			// text
					isList, 			// array / list
					isStream,			// open i/o stream
					isVarRef,			// pointer -> other Var
					isSema,				// semaphor
					isIrpt,				// interrupt
					isThread,
					isProc,	 			// procedure
};
enum
{			// protection flags+counters
				// var_lock_cnt:	Variable existence protection, e.g. during assignment
				// data_lock_cnt:	Variable existence & Data protection, e.g. during procedure execution

				// type_lock_bit:	Application setting for fixed-type variables
				// data_lock_bit:	Application setting for const-data variables

					type_bits		= 4,
					type_lock_bit	= 4,
					data_lock_bit	= 5,
					var_lock_bit0 	= 6,
					var_lock_bits	= 26,

					var_lock_base	= 1<<var_lock_bit0,

					type_mask		= RMASK(type_bits),
					type_lock_mask	= 1<<type_lock_bit,
					data_lock_mask	= 1<<data_lock_bit,
					var_lock_mask	= LMASK(var_lock_bit0),
};




/* ----	Var pointer class ----------------------------------------
		with automatic locking/unlocking
*/
class VarPtr
{
	Var*	var;

private:
			VarPtr		( cVar& q );				// prohibit: typo
	VarPtr&	operator=	( cVar& q );				// prohibit: typo

public:
			~VarPtr		( );

			VarPtr		( Var* vp );
	Var*	operator=	( Var* vp );

			VarPtr		( cVarPtr& q );
	Var*	operator=	( cVarPtr& q );

	Var*	operator->	( )	const					{ return var;  }
	Var&	operator*	( )	const					{ return *var; }

	Var&	ref			( )							{ return *var; }
	cVar&	ref			( ) const					{ return *var; }

	Var*	ptr			( )							{ return var; }
	cVar*	ptr			( ) const					{ return var; }

	bool	operator!=	( cVar* q )					{ return var!=q; }
	bool	operator==	( cVar* q )					{ return var==q; }

	friend	class Var;
};





/* ----	Struct Vek -----------------------------------------------
		array==0 && size=used=0			=> leerer vektor
		array!=0 && used<=size			=> dynamisch angelegter vektor
		array!=0 && used!=0 && size==0	=> subslice aus anderem vektor
*/
struct Vek
{
	Var**			array;				// array of Vars
	uint			size;				// allocated size (items)
	uint			used;				// used items (VScript: Size)

					Vek();				// prohibit!
					~Vek();				// prohibit!
					Vek(cVek&);			// prohibit!
	Vek&			operator=(cVek&);	// prohibit!
};


/* ----	struct Proc -----------------------------------------------
*/
struct Proc
{
	Var*			bundle;				// list Var: contains compiled sourcefile data
//	Var**			constants;
//	uptr			bu_anf;				// bundle->list().array[0]->Text().UCS1Text()
//	uptr			bu_end;
	uptr			ip;					// proc entry

	void			init		( cProc& q );
	void			init		( Var* r, uptr ip );
	void			kill		( );
	Proc&			operator=	( cProc& q );
					Proc		( Var* v, uptr p );
					Proc		( cProc& q );
					~Proc		( );
};
enum { bundle_core=0,   bundle_constants=1,  bundle_xref=2,
	   bundle_source=3, bundle_sourcefile=4, bundle_cachefile=5 };

enum { at_end=~0u };	// for idx in Var creator

extern	Var*	zero;
extern	Var*	eins;


struct Handle
{
	BPObj*		data;
	Var*		next;
	Var*		prev;
};




/* ====	variables class ================================
*/
class Var
{
friend class VScript;
friend struct InitVar;
friend class BPObj;

// data members:
	ulong			type_and_lock;						// variable type & security
	Var*			parent;								// parent list or NULL
	uint			index;								// own index in parent's list
	NameHandle		name;								// variable's name
	long			data[sizeof(String)/sizeof(long)];	// variable data

static Var**		pool;
static Var*			grow_pool		( );
static Var*			new_var			( )					{ XXXXLog("+"); Var* v = *--pool; return v ? v : grow_pool(); }
static void			delete_var		( Var* v )			{ XXXXLog("-"); *pool++ = v; }


// ----	private functions ------------------------------

// access lock:
	void		init_type_and_lock	( )					{ type_and_lock  = isNumber; }
	void		init_type_and_lock	( vtype type )		{ type_and_lock  = type; }

	ulong		decr_lock			( )					{ return type_and_lock -= var_lock_base; }
	void		incr_lock			( )					{ 		 type_and_lock += var_lock_base; }

static bool		is_locked			( ulong lock )		{ return lock          >= var_lock_base; }
static bool		is_unlocked			( ulong lock )		{ return lock          <  var_lock_base; }
	int			lock_count			( ) const			{ return type_and_lock>>var_lock_bit0;   }
	bool		is_locked			( )	const			{ return type_and_lock >= var_lock_base*1; }
	bool		is_double_locked	( )	const			{ return type_and_lock >= var_lock_base*2; }
	bool		is_triple_locked	( )	const			{ return type_and_lock >= var_lock_base*3; }
	bool		is_unlocked			( )	const			{ return type_and_lock <  var_lock_base*1; }
	bool		is_not_double_locked( )	const			{ return type_and_lock <  var_lock_base*2; }
	bool		is_not_triple_locked( )	const			{ return type_and_lock <  var_lock_base*3; }

	void		lock				( )					{ 				  incr_lock(); }
	void		unlock				( )					{ if(is_unlocked( decr_lock() )) { kill_data(); kill_name(); kill_link_nop(); delete_var(this); } }

	void		lock_data			( )					{ type_and_lock |=  data_lock_mask; }
	void		unlock_data			( )					{ type_and_lock &= ~data_lock_mask; }
	void		lock_type			( )					{ type_and_lock |=  type_lock_mask; }
	void		unlock_type			( )					{ type_and_lock &= ~type_lock_mask; }
	void		setmask				( ulong mask );
	void		resmask				( ulong mask );

	bool		data_is_locked		( )	const			{ return   type_and_lock  &  data_lock_mask;				 }
	bool		data_is_unlocked	( )	const			{ return (~type_and_lock) &  data_lock_mask;				 }
	bool		type_is_locked		( ) const			{ return   type_and_lock  &  				type_lock_mask;  }
	bool		type_is_unlocked	( ) const			{ return (~type_and_lock) & 		  	    type_lock_mask;  }
	bool		type_or_data_locked	( ) const			{ return   type_and_lock  & (data_lock_mask|type_lock_mask); }

// access type:
	void		init_type		( vtype type=isNumber )	{ type_and_lock = (type_and_lock & ~type_mask) + type;     }	// also preserve type and data lock
	void		kill_type		( )						{  }
	void		set_type		( vtype type=isNumber )	{ kill_type(); init_type(type); }
	vtype		get_type		( )	const				{ return vtype( type_and_lock & type_mask ); }

// access name:
	void		init_name		( NameHandle nh=0 )		{ name = nh; if(nh) LockNameHandle(nh); }
	void		init_name		( cstr s )				{ name = NewNameHandle(s); }
	void		init_name		( cString& s)			{ name = NewNameHandle(s); }
	void		kill_name		( )						{ if(name) UnlockNameHandle(name); }
	void		kill_name_nop	( )						{ /* when you are sure that it's unnamed */ }
	void		set_name		( NameHandle nh=0 )		{ kill_name(); init_name(nh); }
	void		set_name		( cstr s )				{ kill_name(); init_name(s); }
	void		set_name		( cString& s )			{ kill_name(); init_name(s); }
	bool		is_named		( )						{ return name!=0; }

// access links:
	void		init_link 		( )						{ parent = NULL; }
	void		init_link 		( Var* par, uint idx );
	void		init_link_grow	( Var* par, uint idx );	// may grow parent list
	void		init_link_force	( Var* par, uint idx );	// may grow parent list or unlink old item
	void		kill_link 		( )						{ parent->list().array[index]=NULL; }
	void		kill_link_nop 	( )						{ /* parent==0 or parent->list().array is mangled manually */ }
	bool		is_linked		( )						{ return parent!=NULL; }
	bool		is_unlinked		( )						{ return parent==NULL; }
	void		link			( Var* par, uint idx )	{ XXXTRAP(is_linked()); kill_link_nop(); init_link(par,idx); lock(); }
	void		unlink			( )						{ XXXTRAP(is_unlinked()); kill_link(); init_link(); unlock(); }
	void		relink			( Var* par, uint idx )	{ XXXTRAP(is_unlinked()); kill_link(); init_link(par,idx); }

// access data:
	Double&		value			( )						{ return *(Double*)data; }
	String&		text			( )						{ return *(String*)data; }
	Proc&		proc			( )						{ return *(Proc*)  data; }
	Vek&		list			( )						{ return *(Vek*   )data; }
	Var*&		varptr			( )						{ return *(Var**)  data; }
	Handle&		handle			( )						{ return *(Handle*)data; }

	cDouble&	value			( ) const				{ return *(Double*)data; }
	cString&	text			( )	const				{ return *(String*)data; }
	cProc&		proc			( )	const				{ return *(Proc*)  data; }
	cVek&		list			( )	const				{ return *(Vek*   )data; }
	cVar*const&	varptr			( )	const				{ return *(cVar**) data; }
	Handle const& handle		( )	const				{ return *(Handle*)data; }

	Stream* 	stream			( )	const				{ return (Stream*)(handle().data); }
	Sema*		sema			( ) const				{ return (Sema*)(handle().data);   }
	Irpt*		irpt			( ) const				{ return (Irpt*)(handle().data);   }
	Thread*		thread			( ) const				{ return (Thread*)(handle().data); }

	void		init_value		( )						{ value() = 0.0; }
	void		init_value		( Double n )			{ value() = n; }
	void		kill_value		( )						{ };

	void		init_text		( )						{ new(data) String; }
	void		init_text		( cString& s )			{ new(data) String(s); }
	void		kill_text		( )						{ text()._kill(); }

	void		init_list		( );
	void		init_list		( cVek& );
	void		kill_list		( );

	void		init_proc		( Var*r, uptr p )		{ proc().init(r,p);}
	void		init_proc		( cProc& q )			{ proc().init(q);  }
	void		kill_proc		( )						{ proc().kill();   }

	void		init_varptr		( cVar*v )				{ (varptr()=(Var*)v)->lock(); }
	void		kill_varptr		( )						{ Log("kill_varptr()"); varptr()->unlock(); }

	void		kill_handle		( );
	void		init_handle		( const Var& );
	void		init_handle		( BPObj*, int type );

	// note: init/kill data implies init/kill type
	void		init_data_and_lock	( )					{ init_type_and_lock();     	init_value();  }
	void		init_data_and_lock	( vtype t )			{ init_type_and_lock();			init_data(t);  }
	void		init_data_and_lock	( cVar& v )			{ init_type_and_lock();     	init_data(v);  }

	void		init_data_and_lock	( Double d )		{ init_type_and_lock();			init_value(d); }
	void		init_data_and_lock	( cString& s )		{ init_type_and_lock(isText);	init_text(s);  }
	void		init_data_and_lock	( cVek& l )			{ init_type_and_lock(isList);	init_list(l);  }
	void		init_data_and_lock	( cProc& p )		{ init_type_and_lock(isProc);	init_proc(p);  }
	void		init_data_and_lock	( Var* v )			{ init_type_and_lock(isVarRef);	init_varptr(v);}
	void		init_data_and_lock	( Stream* s )		{ init_type_and_lock(isStream);	init_handle(s,isStream);}
	void		init_data_and_lock	( Sema* s )			{ init_type_and_lock(isSema);	init_handle(s,isSema);}
	void		init_data_and_lock	( Irpt* i )			{ init_type_and_lock(isIrpt);	init_handle(i,isIrpt);}
	void		init_data_and_lock	( Thread* t )		{ init_type_and_lock(isThread);	init_handle(t,isThread);}

	void		init_data		( )						{ init_type();     		init_value();  }
	void		init_data		( Double d )			{ init_type(isNumber);	init_value(d); }
	void		init_data		( cString& s )			{ init_type(isText);  	init_text(s);  }
	void		init_data		( Stream* s )			{ init_type(isStream); 	init_handle(s,isStream);}
	void		init_data		( vtype v );
	void		init_data		( cVar& q );

	void		kill_data		( );
	void		move_data		( Var& q );
	void		swap_data		( Var& q );

	void		set_data		( )						{ if(IsNoNumber()) { kill_data(); init_type(); } init_value(); }
	void		set_data		( Double d )			{ if(IsNoNumber()) { kill_data(); init_type(); } init_value(d);}
	void		set_data		( vtype v )				{ kill_data(); init_data(v); }
	void		set_data		( cString& s )			{ kill_data(); init_data(s); }
	void		set_data		( cVar& q )				{ kill_data(); init_data(q); }
	void		set_data		( Stream* s )			{ kill_data(); init_data(s); }


// misc.:
	void		grow_list		( uint n );
	void		shrink_list		( uint n );
	void		insert_items	( uint idx, uint n );
	void		delete_items	( uint idx, uint n );
	Double		compare			( cVar& q ) const;
static void		Sort			( Var**, Var** );
static void		RSort			( Var**, Var** );
	void		sort			( SortProcPtr );
	Var&		list_op			( Var&(Var::*fp)(Double), Double q );
	Var&		list_op			( Var&(Var::*fp)(Double), cVar& q  );
//	void		list_op			( void(Var::*fp)(cVar&,Double), cVar& q1, Double q2 );
//	void		list_op			( void(Var::*fp)(cVar&,Double), cVar& q1, cVar& q2 );
	Var&		append_list		( cVar& q, DisassProcPtr=NULL );
	void		set_error_locked( );

	void		operator delete[]	( void*, size_t );
	void*		operator new[]		( size_t sz );


// ----	public functions -----------------------------------

public:
	void		validate		( cstr, long ) const;
	#define		Validate()		validate(__FILE__,__LINE__)

	void*		operator new	( size_t )					{ return new_var(); }
	void		operator delete	( void* p, size_t )			{ XXTRAP(((Var*)p)->is_locked()); delete_var((Var*)p); }
	void*		operator new	( size_t, void* p )			{ return p; }

				~Var			( )							{ XXXLogIn("~Var()"); XXTRAP(is_locked()); kill_data(); kill_link_nop(); kill_name(); XXXLogOut(); }
				Var				( )							{ init_name(); init_link(); init_data_and_lock(); }

				Var				( vtype );
				Var				( Double value );
				Var				( cString& text );
				Var				( cVar& q )					{ XXXLogIn("Var(Var)"); init_name(); init_link(); init_data_and_lock(q); XXXLogOut(); }

				Var				( vtype v,  NameHandle );
				Var				( Double n, NameHandle );
				Var				( cString&s,NameHandle );
				Var				( cVar& q,  NameHandle );

				Var				( vtype v,  cstr var_name )		{ new(this) Var(v,NewNameHandle(var_name)); }
				Var				( Double n, cstr var_name )		{ new(this) Var(n,NewNameHandle(var_name)); }
				Var				( cString&s,cstr var_name )		{ new(this) Var(s,NewNameHandle(var_name)); }
				Var				( cVar& q, 	cstr var_name )		{ init_name(var_name); init_link(); init_data_and_lock(q); }

				Var				( vtype v,  cString& var_name )	{ new(this) Var(v,NewNameHandle(var_name)); }
				Var				( Double n, cString& var_name )	{ new(this) Var(n,NewNameHandle(var_name)); }
				Var				( cString&s,cString& var_name )	{ new(this) Var(s,NewNameHandle(var_name)); }
				Var				( cVar& q, 	cString& var_name )	{ init_name(var_name); init_link(); init_data_and_lock(q); }

				Var				( Var* par, uint idx );
				Var				( Var* par, uint idx, vtype v );
				Var				( Var* par, uint idx, Double d );
				Var				( Var* par, uint idx, cString& text );
				Var				( Var* par, uint idx, cVar& q );

				Var				( Var* par, uint idx, vtype v,  NameHandle );
				Var				( Var* par, uint idx, Double n, NameHandle );
				Var				( Var* par, uint idx, cString&, NameHandle );
				Var				( Var* par, uint idx, Stream*,  NameHandle );
				Var				( Var* par, uint idx, cVar& q,  NameHandle );

				Var				( Var* par, uint idx, vtype v,  cstr var_name )		{ new(this) Var(par,idx, v,NewNameHandle(var_name)); }
				Var				( Var* par, uint idx, Double n, cstr var_name )		{ new(this) Var(par,idx, n,NewNameHandle(var_name)); }
				Var				( Var* par, uint idx, cString&s,cstr var_name )		{ new(this) Var(par,idx, s,NewNameHandle(var_name)); }
				Var				( Var* par, uint idx, cVar& q,  cstr var_name )		{ new(this) Var(par,idx, q,NewNameHandle(var_name)); }
				Var				( Var* par, uint idx, Stream*s, cstr var_name )		{ new(this) Var(par,idx, s,NewNameHandle(var_name)); }

				Var				( Var* par, uint idx, vtype v,  cString& var_name )	{ new(this) Var(par,idx, v,NewNameHandle(var_name)); }
				Var				( Var* par, uint idx, Double n, cString& var_name )	{ new(this) Var(par,idx, n,NewNameHandle(var_name)); }
				Var				( Var* par, uint idx, cString&s,cString& var_name )	{ new(this) Var(par,idx, s,NewNameHandle(var_name)); }
				Var				( Var* par, uint idx, cVar& q,  cString& var_name )	{ new(this) Var(par,idx, q,NewNameHandle(var_name)); }

				Var				( Var* par, uint idx, char const*const* argv, uint argc=~0 );
				Var				( char const*const* argv, uint argc=~0 );
				Var				( cProc& p )			{ init_name(); init_link(); init_data_and_lock(p); }
explicit		Var				( Thread* t )			{ init_name(); init_link(); init_data_and_lock(t); }
explicit		Var				( Stream* s )			{ init_name(); init_link(); init_data_and_lock(s); }

	Var&		operator=		( Var& q );
	Var&		operator=		( cVar& q )				{ return operator=( *(Var*)&q ); }
	Var&		operator=		( Double n )			{ set_data(n); return *this; }
	Var&		operator=		( cString& s )			{ set_data(s); return *this; }		// note: no test for intersection
	Var&		operator=		( Stream* s )			{ set_data(s); return *this; }

	void		CopyData		( Var& q );				// this := copy of q
	void		MoveData		( Var& q );				// this := data of q; q := 0.0
	void		SwapData		( Var& q );				// this := data of q; q := data of this

// link to parent list:
	void		Link			( Var* par, uint idx );	// unlink & unlock self, unlink & unlock old item, grow parent list, lock & link.
	void		Unlink			( );					// unlink & unlock self and shrink parent list
	void		Vanish			( );					// unlink & unlock self, delete item in parent list and shrink parent list
	bool		IsLinked		( )						{ return parent!=NULL; }
	bool		IsUnlinked		( )						{ return parent==NULL; }
	Var*		Parent			( )						{ return parent; }
	uint		Index			( )						{ return index; }

// variable existance protection:
// must be balanced before the variable is destroyed!
	void		Lock			( )						{ if(this) lock(); }
	void		Unlock			( )						{ if(this) unlock(); }			// may vanish
	bool		IsLocked		( )						{ return is_locked(); }
	bool		IsUnlocked		( )						{ return is_unlocked(); }

// existance and link position protection:
// must be balanced before the root variable is destroyed!
	void		Protect			( )						{ for(Var*p=this;p;p=p->parent) p->lock(); }
	void		Unprotect		( )						{ Var*p=this; while(p) { Var*z=p; p=p->parent; z->unlock(); } }	// top parent incl. entire tree may vanish
	bool		IsProtected		( )	const				{ cVar*p=this; while(p&&p->is_locked()) p=p->parent; return p==NULL; }
	bool		IsUnprotected	( )	const				{ cVar*p=this; while(p&&p->is_locked()) p=p->parent; return p!=NULL; }

// type lock flag:
	void		LockType		( )						{ lock_type(); }
	void		UnlockType		( )						{ unlock_type(); }
	void		DeepLockType	( )						{ lock_type();   setmask( type_lock_mask); }
	void		DeepUnlockType	( )						{ unlock_type(); resmask(~type_lock_mask); }
	bool		TypeIsLocked	( )	const				{ return type_is_locked(); }
	bool		TypeIsUnlocked	( )	const				{ return type_is_unlocked(); }

// const data flag:
	void		LockData		( )						{ lock_data(); }
	void		UnlockData		( )						{ unlock_data(); }
	void		DeepLockData	( )						{ lock_data();   setmask( data_lock_mask); }
	void		DeepUnlockData	( )						{ unlock_data(); resmask(~data_lock_mask); }
	bool		DataIsLocked	( )	const				{ return data_is_locked(); }
	bool		DataIsUnlocked	( )	const				{ return data_is_unlocked(); }

// data type:
	void		SetType			( vtype typ )			{ set_data(typ); }
	void		SetNumber		( )						{ set_data(); }
	void		SetString		( )						{ set_data(isText); }
	void		SetList			( )						{ set_data(isList); }
	void		SetList			( uint n )				{ set_data(isList); grow_list(n); }
	void		SetProc			( )						{ set_data(isProc); }

	vtype		GetType			( )	const				{ return get_type();		   }
	bool		IsNumber		( )	const				{ return get_type()==isNumber; }
	bool		IsText			( )	const				{ return get_type()==isText;   }
	bool		IsList			( )	const				{ return get_type()==isList;   }
	bool		IsProc			( )	const				{ return get_type()==isProc;   }
	bool		IsThread		( )	const				{ return get_type()==isThread; }
	bool		IsIrpt			( )	const				{ return get_type()==isIrpt;   }
	bool		IsSema			( )	const				{ return get_type()==isSema;   }
	bool		IsVarRef		( )	const				{ return get_type()==isVarRef; }
	bool		IsStream		( )	const				{ return get_type()==isStream; }
	bool		IsNoNumber		( )	const				{ return get_type()!=isNumber; }
	bool		IsNoText		( )	const				{ return get_type()!=isText;   }
	bool		IsNoList		( )	const				{ return get_type()!=isList;   }
	bool		IsNoProc		( )	const				{ return get_type()!=isProc;   }
	bool		IsNoThread		( )	const				{ return get_type()!=isThread; }
	bool		IsNoIrpt		( )	const				{ return get_type()!=isIrpt;   }
	bool		IsNoSema		( )	const				{ return get_type()!=isSema;   }
	bool		IsNoVarRef		( )	const				{ return get_type()!=isVarRef; }
	bool		IsNoStream		( )	const				{ return get_type()!=isStream; }

	String		Name			( )	const				{ return GetNameForHandle(name); }
	NameHandle&	GetNameHandle	( )						{ return name; }
	NameHandle	GetNameHandle	( ) const				{ return name; }
	bool		IsNamed			( )						{ return name!=0; }
	void		ClearName		( )						{ set_name(); }
	void		SetName			( cString& newname )	{ set_name(newname); }
	void		SetName			( cstr newname )		{ set_name(newname); }
	void		SetName			( NameHandle newname )	{ set_name(newname); }

	cDouble&	Value			( )	const				{ XXTRAP(IsNoNumber()); return value(); }
	Double&		Value			( )						{ XXTRAP(IsNoNumber()); return value(); }
	long		LongValue		( )	const				{ return (long)Value(); }
	llong		LLongValue		( )	const				{ return (llong)Value(); }

	String&		Text			( )						{ XXTRAP(IsNoText());   return text(); }
	cString&	Text			( ) const				{ XXTRAP(IsNoText());   return text(); }
	ptr			TextAddr		( )						{ XXTRAP(IsNoText());   return text().Text(); }
	UCS1Char*	Ucs1Addr		( )						{ XXTRAP(IsNoText());   return text().Ucs1(); }
	UCS2Char*	Ucs2Addr		( )						{ XXTRAP(IsNoText());   return text().Ucs2(); }
	UCS4Char*	Ucs4Addr		( )						{ XXTRAP(IsNoText());   return text().Ucs4(); }
	uint		TextLen			( )						{ XXTRAP(IsNoText());   return text().Len(); }

	Proc&		GetProc			( )						{ XXTRAP(IsNoProc());   return proc(); }
	cProc&		GetProc			( ) const				{ XXTRAP(IsNoProc());   return proc(); }
	uptr		ProcStart		( )						{ XXTRAP(IsNoProc());   return proc().ip; }
	Var*		ProcBundle		( )						{ XXTRAP(IsNoProc());   return proc().bundle; }
	Var&		ProcBundle		( uint i )				{ return (*ProcBundle())[i]; }
	Var**		ProcConstants	( )						{ return ProcBundle(bundle_constants).List().array; }
	cString&	ProcSourcefile	( )						{ return ProcBundle(bundle_sourcefile).Text(); }
	cString&	ProcCachefile	( )						{ return ProcBundle(bundle_cachefile).Text(); }
	cString&	ProcSource		( )						{ return ProcBundle(bundle_source).Text(); }
	cString&	ProcXref		( )						{ return ProcBundle(bundle_xref).Text(); }
	ulong*		ProcXrefBase	( )						{ return ProcBundle(bundle_xref).Text().UCS4Text(); }
	cString&	ProcCore		( )						{ return ProcBundle(bundle_core).Text(); }
	uptr		ProcCoreBase	( )						{ return ProcBundle(bundle_core).Text().UCS1Text(); }

	Var&		GetVarRef		( )						{ XXTRAP(IsNoVarRef());	return *varptr(); }

	Thread*		GetThread		( )						{ XXTRAP(IsNoThread()); return thread(); };
	Irpt*		GetIrpt			( )						{ XXTRAP(IsNoIrpt()); 	return irpt(); };
	Sema*		GetSema			( )						{ XXTRAP(IsNoSema()); 	return sema(); };
	Stream*		GetStream		( )						{ XXTRAP(IsNoStream());	return stream(); };

	Vek&		List			( )						{ XXTRAP(IsNoList());   return list(); }
	uint		ListSize		( )						{ XXTRAP(IsNoList());   return list().used; }
	Var&		operator[]		( uint i );
	cVar&		operator[]		( uint i )  const		{ return (*(Var*)this)[i]; }
	Var&		LastItem		( )						{ return (*this)[list().used-1]; }
	cVar&		LastItem		( )	const				{ return (*this)[list().used-1]; }
	Var&		FirstItem		( )						{ return (*this)[0]; }
	cVar&		FirstItem		( )	const				{ return (*this)[0]; }
	bool		Contains		( cVar* p )	const		{ while(p&&p!=this) p=p->parent; return p; }	// does this contain p ?
	bool		Contains		( cVar& p )	const		{ return Contains(&p); }						// does this contain p ?

	Var*		FindItem		( NameHandle name ) const;
	Var*		FindItem		( cString& name ) const;
	Var*		FindItem		( cstr name ) const;

	long		Find			( cVar& v, long startidx= 0 ) const;
	long		RFind			( cVar& v, long startidx= 0x7fffffff ) const;

	void 		ResizeList		( uint n );
	void		InsertItems		( uint idx, uint n );	// füge leere Variablen in List ein
	void		InsertItem		( uint idx )			{ InsertItems(idx,1); }
	void		InsertItem		( uint idx, Var* q );
	void		AppendItem		( Var* q )				{ InsertItem(at_end,q); }
	void		AppendItems		( Var* a,Var*b=0,Var*c=0,Var*d=0 );
	void		AppendItems		( Var* a,Var*b,Var*c,Var*d,Var*e,Var*f=0,Var*g=0,Var*h=0 );
	void		DeleteItems		( uint idx, uint n );	// lösche Variablen aus List
	void		DeleteItem		( uint idx )			{ DeleteItems(idx,1); }
	void		DeleteLastItem	( );
	void		AppendListItems	( Var& q );
	void		Sort			( )						{ sort( Sort ); }	// sortiere List
	void		RSort			( )						{ sort( RSort ); }	// sortiere List rückwärts
	void		Shuffle			( );					// shuffle list

	Var&		MaxVar			( Var& q );
	Var&		MinVar			( Var& q );
	Var&		MaxVar			( );					// of list
	Var&		MinVar			( );					// of list

	void		ConvertFrom		( CharEncoding );
	void		ConvertTo		( CharEncoding );
//	void		LogVar			( ) const;
	Double		ToNumber		( ) const;
	String		ToString		( bool quotestring=no, DisassProcPtr=NULL ) const;
	Var			ToList			( bool putlistinlist=no );
	void		ConvertToNumber	( );
	void		ConvertToString	( bool quotestring=no, DisassProcPtr=NULL );
	void		ConvertToList	( bool putlistinlist=no );

	void		SplitLines		( cString& sep );
	void		SplitLines		( );

	Var&		operator+=		( Double d );
	Var&		operator-=		( Double d );
	Var&		operator*=		( Double d );
	Var&		operator/=		( Double d );
	Var&		operator%=		( Double d );
	Var&		operator>>=		( Double d );
	Var&		operator<<=		( Double d );
	Var&		operator&=		( Double d );
	Var&		operator|=		( Double d );
	Var&		operator^=		( Double d );

	Var&		operator+=		( cVar& q );
	Var&		operator-=		( cVar& q );
	Var&		operator*=		( cVar& q );
	Var&		operator/=		( cVar& q );
	Var&		operator%=		( cVar& q );
	Var&		operator>>=		( cVar& d );
	Var&		operator<<=		( cVar& d );
	Var&		operator&=		( cVar& d );
	Var&		operator|=		( cVar& d );
	Var&		operator^=		( cVar& d );

	Var			operator+		( Double d ) const;
	Var			operator-		( Double d ) const;
	Var			operator*		( Double d ) const;
	Var			operator/		( Double d ) const;
	Var			operator%		( Double d ) const;
	Var			operator>>		( Double d ) const;
	Var			operator<<		( Double d ) const;
	Var			operator&		( Double d ) const;
	Var			operator|		( Double d ) const;
	Var			operator^		( Double d ) const;

	Var			operator+		( cVar& q ) const;
	Var			operator-		( cVar& q ) const;
	Var			operator*		( cVar& q ) const;
	Var			operator/		( cVar& q ) const;
	Var			operator%		( cVar& q ) const;
	Var			operator>>		( cVar& q ) const;
	Var			operator<<		( cVar& q ) const;
	Var			operator&		( cVar& q ) const;
	Var			operator|		( cVar& q ) const;
	Var			operator^		( cVar& q ) const;

	Var&		operator+=		( cString& s )				{ return AppendString(s); }
	Var&		AppendString	( cString& q, DisassProcPtr=NULL );
	Var&		AppendString	( cVar&    q, DisassProcPtr=NULL );

	Var			operator+		( cString& s ) const		{ return ConcatString(s); }
	Var 		ConcatString	( cString& q, DisassProcPtr=NULL ) const;
	Var			ConcatString	( cVar&    q, DisassProcPtr=NULL ) const;

#ifdef VAR_INCLUDE_BOOLEAN
// not really a good idea. this eliminates pruning.
	bool		operator&&		( bool q ) const;
	bool		operator||		( bool q ) const;
	bool		operator&&		( cVar& q ) const;
	bool		operator||		( cVar& q ) const;
#endif

	bool		operator==		( cVar& q ) const			{ return IsNumber()&&q.IsNumber() ? value()==q.value() : compare(q)==0; }
	bool		operator>		( cVar& q ) const			{ return IsNumber()&&q.IsNumber() ? value()> q.value() : compare(q)> 0; }
	bool		operator<		( cVar& q ) const			{ return IsNumber()&&q.IsNumber() ? value()< q.value() : compare(q)< 0; }
	bool		operator!=		( cVar& q ) const			{ return IsNumber()&&q.IsNumber() ? value()!=q.value() : compare(q)!=0; }
	bool		operator>=		( cVar& q ) const			{ return IsNumber()&&q.IsNumber() ? value()>=q.value() : compare(q)>=0; }
	bool		operator<=		( cVar& q ) const			{ return IsNumber()&&q.IsNumber() ? value()<=q.value() : compare(q)<=0; }

	bool		operator==		( cString& q ) const		{ return IsText()    && text()==q; }
	bool		operator>		( cString& q ) const		{ return IsText()    && text()> q; }
	bool		operator<		( cString& q ) const		{ return IsText()    && text()< q; }
	bool		operator!=		( cString& q ) const		{ return IsNoText()  || text()!=q; }
	bool		operator>=		( cString& q ) const		{ return IsText()    && text()>=q; }
	bool		operator<=		( cString& q ) const		{ return IsText()    && text()<=q; }

	// die folgenden hauptsächlich wg. operatorXY(0), was sonst als NULL interpretiert wird. GRRR!
	bool		operator==		( int q ) const				{ return IsNumber()  && value()==q; }
	bool		operator>		( int q ) const				{ return IsNumber()  && value()> q; }
	bool		operator<		( int q ) const				{ return IsNumber()  && value()< q; }
	bool		operator!=		( int q ) const				{ return IsNoNumber()|| value()!=q; }
	bool		operator>=		( int q ) const				{ return IsNumber()  && value()>=q; }
	bool		operator<=		( int q ) const				{ return IsNumber()  && value()<=q; }

	// die folgenden hauptsächlich damit operatorXY(Double) nicht zu operatorXY(int) autocasted
	bool		operator==		( Double q ) const			{ return IsNumber()  && value()==q; }
	bool		operator>		( Double q ) const			{ return IsNumber()  && value()> q; }
	bool		operator<		( Double q ) const			{ return IsNumber()  && value()< q; }
	bool		operator!=		( Double q ) const			{ return IsNoNumber()|| value()!=q; }
	bool		operator>=		( Double q ) const			{ return IsNumber()  && value()>=q; }
	bool		operator<=		( Double q ) const			{ return IsNumber()  && value()<=q; }

friend void		TestVarClass	( );
friend class VarPtr;
friend class Proc;
};


extern void		TestVarClass		( );
extern Var* 	NewEnvironmentVar	( bool lowercase=0 );



// class VarPtr:

inline			VarPtr::~VarPtr		( )				{ var->Unlock(); }

inline 			VarPtr::VarPtr 		( Var* vp )		{ (var=vp)->Lock(); }
inline Var*		VarPtr::operator=	( Var* q_var )	{ q_var->Lock(); var->Unlock(); var=q_var; return var; }

inline			VarPtr::VarPtr		( cVarPtr& q )	{ (var=q.var)->Lock(); }
inline Var*		VarPtr::operator=	( cVarPtr& q )	{ q.var->Lock(); var->Unlock(); var=q.var; return var; }

inline void		Init		( Var*& ptr, Var* var )	{ (ptr=var)->Lock(); }
inline void		Set			( Var*& ptr, Var* var )	{ var->Lock(); ptr->Unlock(); ptr=var; }
inline void		Kill		( Var*& ptr )			{ ptr->Unlock(); }


// struct Proc:

inline			Proc::Proc	( cProc& q )			{ (bundle=q.bundle)->lock(); ip=q.ip; }
inline			Proc::Proc	( Var* v, uptr p )		{ (bundle=v)->lock(); ip=p; }
inline			Proc::~Proc	( )						{  bundle->unlock(); }

inline 	void	Proc::init	( cProc& q )			{ (bundle=q.bundle)->lock(); ip=q.ip; }
inline 	void	Proc::init	( Var*r,uptr p )		{ (bundle=r)->lock();  ip=p;  }
inline	void	Proc::kill	( )						{  bundle->unlock(); }

#endif



