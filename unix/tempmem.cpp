/*	Copyright  (c)	Günter Woigk 2008 - 2019
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

#include "config.h"
#include <pthread.h>
#include "tempmem.h"

static pthread_key_t tempmem_key;			// key for per-thread TempMemPool
#define ALIGNMENT_MASK	(_MAX_ALIGNMENT-1)
#define MAX_REQ_SIZE	500
#define BUFFER_SIZE		4000


/* ----	Deallocate pool ----------------------
        called at thread termination
        while pointer in tempmem_key is != NULL.
        This should happen only once, for
        the automatically created pool.
*/
static void deallocate_pool (void* pool)
{
    delete reinterpret_cast<TempMemPool*>(pool);
}


/* ----	Initialization ---------------------------------
*/
static pthread_once_t once_control = PTHREAD_ONCE_INIT;
static volatile bool  virgin	   = 1;				// for faster init test

static void create_key (void)
{
    int err = pthread_key_create( &tempmem_key, deallocate_pool );
    if(err) abort( "init TempMemPool: %s", strerror(err) );
}

static void init () noexcept
{
    xlogIn("init TempMemPool");

// initialize: get a pthread_key:
    IFDEBUG( int err = ) pthread_once( &once_control, create_key );
    assert(err==0);

    virgin = false;
}


/* ---- TempMemPool member functions --------------------
*/
TempMemPool::TempMemPool() noexcept
:	size(0),
    data(nullptr)
{
    xlogIn("new TempMemPool");

    if(virgin) init();

    prev = reinterpret_cast<TempMemPool*>(pthread_getspecific( tempmem_key ));
    xxlogline("  prev pool = %lx",(ulong)prev);
    xxlogline("  this pool = %lx",(ulong)this);
    //int err =
    pthread_setspecific( tempmem_key, this );				// may fail with ENOMEM (utmost unlikely)
    //if(err) { Abort("new TempMemPool: ",strerror(err)); }	// in which case this pool is not registered
}															// and GetPool() keeps using the outer pool


TempMemPool::~TempMemPool() noexcept
{
    xlogIn("delete TempMemPool");
    xxlogline("  this pool = %lx",(ulong)this);
    xxlogline("  prev pool = %lx",(ulong)prev);

    purge();
    int err = pthread_setspecific( tempmem_key, prev );		// may fail with ENOMEM (utmost unlikely)
    if(err) { abort("delete TempMemPool: %s",strerror(err)); }
}


void TempMemPool::purge () noexcept		// Purge() == destroy + create pool
{
    xlogIn("TempMemPool::Purge");
    xxlog("  this pool = %lx ",(ulong)this);

    while (data != nullptr)
    {
        xxlog(".");
        TempMemData* prev = data->prev;
        delete[] reinterpret_cast<char*>(data); data = prev;
    }
    size = 0;
    xxlogline(" ok");
}


char* TempMemPool::alloc (uint bytes) noexcept
{
    if (bytes <= size)					// fits in current buffer?
    {
        size -= bytes;
        return data->data + size;
    }
    else if (bytes <= MAX_REQ_SIZE)		// small request?
    {
        TempMemData* newdata = reinterpret_cast<TempMemData*>(new char[ sizeof(TempMemData) + 4000 ]);
        xxlogline("tempmem new data = $%lx",(ulong)newdata);
        assert( (uintptr_t(newdata) & ALIGNMENT_MASK) == 0 );
        newdata->prev = data;
        data = newdata;
        size = 4000-bytes;
        return newdata->data + size;
    }
    else								// large request
    {
        TempMemData* newdata = reinterpret_cast<TempMemData*>(new char[ sizeof(TempMemData) + bytes ]);
        xxlogline("tempmem new data = $%lx", ulong(newdata));
        assert( (uintptr_t(newdata) & ALIGNMENT_MASK) == 0 );
        if(data)
        {
            newdata->prev = data->prev;		// neuen Block 'unterheben'
            data->prev = newdata;
        }
        else
        {
            newdata->prev = nullptr;
            data = newdata;
            size = 0;
        }
        return newdata->data;
    }
}


char* TempMemPool::allocMem (uint bytes) noexcept
{
    char* p = alloc(bytes);
    if( data->prev && p == data->prev->data )	// wurde "large request" 'untergehoben' ?
    {
        return p;
    }
    else
    {
        uint n = size & ALIGNMENT_MASK;
        size -= n;
        return p-n;
    }
}


/* ---- Get the current temp mem pool -------------------------
        if there is no pool, then it is created.
*/
TempMemPool* TempMemPool::getPool() noexcept
{
    if(virgin) init();
    TempMemPool* pool = reinterpret_cast<TempMemPool*>(pthread_getspecific( tempmem_key ));
    return pool ? pool : new TempMemPool();
}


/* ---- Get the surrounding temp mem pool -------------------------
        if there is no surrounding pool, then it is created.
        a 'current pool' should be in place, else 2 pools are created.
*/
TempMemPool* TempMemPool::getXPool() noexcept
{
    TempMemPool* pool = getPool();
    TempMemPool* prev = pool->prev;
    if( !prev )
    {
        prev = new TempMemPool();					// automatically create 'outer' pool
        prev->prev = nullptr;						// 'outer' pool 'unterheben'.
        pool->prev = prev;
        pthread_setspecific( tempmem_key, pool );	// aktuellen Pool erneut als 'aktuell' markieren
    }												// note: *might* fail with ENOMEM (utmost unlikely)
    return prev;									// in which case we keep on using the outer pool
}


/* ---- Get a temp cstring -------------------------
*/
char* tempstr (uint len) noexcept
{
    return TempMemPool::getPool()->allocStr(len);
}


/* ---- Get a temp cstring -------------------------
        from the surrounding pool
*/
char* xtempstr (uint len) noexcept
{
    return TempMemPool::getXPool()->allocStr(len);
}


/* ---- Get memory for temp. usage -------------------------
*/
char* tempmem (uint size) noexcept
{
    return TempMemPool::getPool()->allocMem(size);
}


/* ---- Get memory for temp. usage -------------------------
        from the surrounding pool
*/
char* xtempmem (uint size) noexcept
{
    return TempMemPool::getXPool()->allocMem(size);
}


/* ---- Purge current pool -------------------------
*/
void purgeTempMem() noexcept
{
    TempMemPool::getPool()->purge();
}




#ifndef NDEBUG
namespace TempMemTest
{
    static struct T
    {
        T()
        {
            // check assumptions:
            assert( (sizeof(TempMemData)&(ALIGNMENT_MASK)) == 0 );
            ptr p1 = new char[17], p2 = new char[15];
            assert( (uintptr_t(p1)&ALIGNMENT_MASK)==0 );
            assert( (uintptr_t(p2)&ALIGNMENT_MASK)==0 );
            delete[] p1;
            delete[] p2;
        }
    } dummy;
}
#endif









