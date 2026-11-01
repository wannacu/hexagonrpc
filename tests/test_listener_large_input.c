/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <string.h>
#include <libhexagonrpc/error.h>

#include "../hexagonrpcd/interface/adsp_listener.h"
#include "../hexagonrpcd/iobuffer.h"

static char encoded[8192];
static uint32_t encoded_size;
static uint32_t advertised_size;
static int fetches;

static int mock_next(int fd, uint32_t previous_ctx, uint32_t previous_result,
                     uint32_t output_size, const char *output, uint32_t *ctx,
                     uint32_t *handle, uint32_t *sc, uint32_t *required,
                     uint32_t size, char *buffer)
{
	assert(previous_result == AEE_EUNSUPPORTED);
	assert(output_size == 0);
	*ctx = 42;
	*handle = 0;
	*sc = 0x00010000;
	*required = advertised_size;
	memcpy(buffer, encoded, size);
	return 0;
}

static int mock_get(int fd, uint32_t ctx, uint32_t offset, uint32_t size,
                    char *buffer, uint32_t *required)
{
	assert(ctx == 42 && offset == 4096);
	assert(offset + size == encoded_size);
	memcpy(buffer, encoded + offset, size);
	*required = encoded_size;
	fetches++;
	return 0;
}

#define adsp_listener_next2 mock_next
#define adsp_listener_get_in_bufs2 mock_get
#include "../hexagonrpcd/listener.c"

int main(void)
{
	char contents[6000];
	struct fastrpc_io_buffer in = {.s = sizeof(contents), .p = contents};
	struct fastrpc_io_buffer *decoded = NULL;
	uint32_t ctx = 0, handle = 0, sc = 0x1f050100;
	memset(contents, 'K', sizeof(contents));
	encoded_size = outbufs_calculate_size(1, &in);
	assert(encoded_size < sizeof(encoded));
	outbufs_encode(1, &in, encoded);
	advertised_size = encoded_size;
	assert(return_for_next_invoke(-1, AEE_EUNSUPPORTED, &ctx, &handle, &sc, NULL, &decoded) == 0);
	assert(fetches == 1);
	assert(decoded[0].s == sizeof(contents));
	assert(memcmp(decoded[0].p, contents, sizeof(contents)) == 0);
	iobuf_free(1, decoded);
	decoded = NULL;
	advertised_size = 5 * 1024 * 1024;
	assert(return_for_next_invoke(-1, AEE_EUNSUPPORTED, &ctx, &handle, &sc, NULL, &decoded) == -E2BIG);
	assert(fetches == 1 && decoded == NULL);
	puts("Large requests are fetched in full; oversized allocations are rejected.");
	return 0;
}
