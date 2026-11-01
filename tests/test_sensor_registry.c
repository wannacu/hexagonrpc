/* SPDX-License-Identifier: GPL-3.0-or-later */
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

#include "../hexagonrpcd/hexagonfs.h"
#include "../hexagonrpcd/sensor_registry.h"

int main(void)
{
	char tmp[] = "/tmp/hexagonrpc-registry-XXXXXX";
	char directory[256], target[256], link[256];
	char bytes[16] = {0};
	const char *path = "/mnt/vendor/persist/sensors/registry/registry/DIR";
	struct hexagonfs_fd *fds[HEXAGONFS_MAX_FD] = {0};
	int hostfd, fd;
	assert(mkdtemp(tmp));
	assert(setenv("PAD3PRO_SENSOR_REGISTRY", tmp, 1) == 0);
	snprintf(directory, sizeof(directory), "%s/registry", tmp);
	assert(mkdir(directory, 0700) == 0);
	hostfd = sensor_registry_open(path, "w+");
	assert(hostfd >= 0);
	fd = hexagonfs_import_host_fd(fds, hostfd);
	assert(fd >= 0);
	assert(hexagonfs_write(fds, fd, 3, "DIR") == 3);
	assert(hexagonfs_lseek(fds, fd, 0, SEEK_SET) == 0);
	assert(hexagonfs_read(fds, fd, sizeof(bytes), bytes) == 3);
	assert(memcmp(bytes, "DIR", 3) == 0);
	assert(hexagonfs_close(fds, fd) == 0);
	assert(hexagonfs_write(fds, fd, 1, "x") == -EBADF);
	assert(sensor_registry_open("/etc/passwd", "w") == -EROFS);
	assert(sensor_registry_open("/mnt/vendor/persist/sensors/registry/registry/../escape", "w") < 0);
	assert(sensor_registry_open("/mnt/vendor/persist/sensors/registry/registry/..", "w") < 0);
	snprintf(target, sizeof(target), "%s/outside", tmp);
	hostfd = open(target, O_CREAT | O_WRONLY, 0600);
	assert(hostfd >= 0);
	assert(write(hostfd, "safe", 4) == 4);
	close(hostfd);
	snprintf(link, sizeof(link), "%s/registry/link", tmp);
	assert(symlink(target, link) == 0);
	assert(sensor_registry_open("/mnt/vendor/persist/sensors/registry/registry/link", "w") == -ELOOP);
	hostfd = open(target, O_RDONLY);
	assert(read(hostfd, bytes, sizeof(bytes)) == 4);
	assert(memcmp(bytes, "safe", 4) == 0);
	close(hostfd);
	assert(sensor_registry_remove(path) == 0);
	assert(sensor_registry_open(path, "r+") == -ENOENT);
	assert(sensor_registry_open(path, "invalid") == -EINVAL);
	assert(unsetenv("PAD3PRO_SENSOR_REGISTRY") == 0);
	assert(sensor_registry_open(path, "w") == -EROFS);
	unlink(link);
	unlink(target);
	rmdir(directory);
	rmdir(tmp);
	puts("Private registry I/O and path confinement passed.");
	return 0;
}
