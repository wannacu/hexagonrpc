/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>

#include "../hexagonrpcd/listener.c"

int main(void)
{
	struct fastrpc_function_impl methods[1] = {{0}};
	struct fastrpc_interface iface = {.n_procs = 1, .procs = methods};
	struct fastrpc_interface *ifaces[] = {&iface};
	struct fastrpc_io_buffer *out = NULL;
	uint32_t method = 36, result = 0;
	struct fastrpc_io_buffer in = {.s = sizeof(method), .p = &method};
	/* Unsupported optional RPCs must return an error without terminating the listener. */
	assert(invoke_requested_procedure(1, ifaces, 0, 0x1f050100, &result, &in, &out) == 0);
	assert(result == AEE_EUNSUPPORTED && out == NULL);
	assert(invoke_requested_procedure(1, ifaces, 8, 0, &result, &in, &out) == 0);
	assert(result == AEE_EUNSUPPORTED && out == NULL);
	assert(invoke_requested_procedure(1, ifaces, 0, 0, &result, &in, &out) == 0);
	assert(result == AEE_EUNSUPPORTED && out == NULL);
	assert(invoke_requested_procedure(1, ifaces, 0, 0x1f000000, &result, &in, &out) == 0);
	assert(result == AEE_EBADPARM && out == NULL);
	puts("Unsupported RPCs preserve the listener and return truthful errors.");
	return 0;
}
