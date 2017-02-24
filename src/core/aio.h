//
// Copyright 2017 Garrett D'Amore <garrett@damore.org>
//
// This software is supplied under the terms of the MIT License, a
// copy of which should be located in the distribution where this
// file was obtained (LICENSE.txt).  A copy of the license may also be
// found online at https://opensource.org/licenses/MIT.
//

#ifndef CORE_AIO_H
#define CORE_AIO_H

#include "core/defs.h"
#include "core/list.h"
#include "core/thread.h"

typedef struct nni_aio_ops	nni_aio_ops;
typedef struct nni_aio		nni_aio;


// An nni_aio is an async I/O handle.
struct nni_aio {
	int		a_result;       // Result code (nng_errno)
	size_t		a_count;        // Bytes transferred (I/O only)
	nni_cb		a_cb;           // User specified callback.
	void *		a_cbarg;        // Callback argument.

	// These fields are private to the io events framework.
	nni_mtx		a_lk;
	nni_cv		a_cv;
	unsigned	a_flags;

	// Read/write operations.
	nni_iov		a_iov[4];
	int		a_niov;

	// Sendmsg operation.
	nni_msg *	a_msg;

	// Recvmsg operation.
	nni_msg **	a_msgp;

	// TBD: Resolver operations.

	// Provider-use fields.
	void *		a_prov_data;
	nni_list_node	a_prov_node;
};

// nni_aio_init initializes an aio object.  The callback is called with
// the supplied argument when the operation is complete.  If NULL is
// supplied for the callback, then nni_aio_wake is used in its place,
// and the aio is used for the argument.
extern void nni_aio_init(nni_aio *, nni_cb, void *);

// nni_aio_fini finalizes the aio, releasing resources (locks)
// associated with it.  The caller is responsible for ensuring that any
// associated I/O is unscheduled or complete.
extern void nni_aio_fini(nni_aio *);

// nni_aio_result returns the result code (0 on success, or an NNG errno)
// for the operation.  It is only valid to call this when the operation is
// complete (such as when the callback is executed or after nni_aio_wait
// is performed).
extern int nni_aio_result(nni_aio *);

// nni_aio_count returns the number of bytes of data transferred, if any.
// As with nni_aio_result, it is only defined if the I/O operation has
// completed.
extern size_t nni_aio_count(nni_aio *);

// nni_aio_wake wakes any threads blocked in nni_aio_wait.  This is the
// default callback if no other is supplied.  If a user callback is supplied
// then that code must call this routine to wake any waiters (unless the
// user code is certain that there are no such waiters).
extern void nni_aio_wake(nni_aio *);

// nni_aio_wait blocks the caller until the operation is complete, as indicated
// by nni_aio_wake being called.  (Recall nni_aio_wake is the default
// callback if none is supplied.)  If a user supplied callback is provided,
// and that callback does not call nni_aio_wake, then this routine may
// block the caller indefinitely.
extern void nni_aio_wait(nni_aio *);

// nni_aio_finish is called by the provider when an operation is complete.
// The provider gives the result code (0 for success, an NNG errno otherwise),
// and the amount of data transferred (if any).
extern void nni_aio_finish(nni_aio *, int, size_t);

#endif // CORE_AIO_H
