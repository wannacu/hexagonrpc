/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Writes are limited to the private Linux registry copy selected by the service. */
#ifndef SENSOR_REGISTRY_H
#define SENSOR_REGISTRY_H

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int sensor_registry_parent(const char *path, const char **name)
{
	const char *prefixes[] = {
		"/mnt/vendor/persist/sensors/registry/",
		"/persist/sensors/registry/",
	};
	const char *root = getenv("PAD3PRO_SENSOR_REGISTRY");
	const char *relative = NULL;
	const char *p;
	int parent, child;
	size_t i;
	if (root == NULL || root[0] != '/')
		return -EROFS;
	for (i = 0; i < sizeof(prefixes) / sizeof(prefixes[0]); i++) {
		if (!strncmp(path, prefixes[i], strlen(prefixes[i]))) {
			relative = path + strlen(prefixes[i]);
			break;
		}
	}
	if (relative == NULL)
		return -EROFS;
	parent = open(root, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
	if (parent < 0)
		return -errno;
	if (!strncmp(relative, "registry/", 9)) {
		child = openat(parent, "registry", O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
		close(parent);
		if (child < 0)
			return -errno;
		parent = child;
		relative += 9;
	} else if (strcmp(relative, "sns_reg_version") &&
		   strcmp(relative, "parsed_file_list.csv") &&
		   strcmp(relative, "fstempfile")) {
		close(parent);
		return -EROFS;
	}
	if (!relative[0] || !strcmp(relative, ".") || !strcmp(relative, "..")) {
		close(parent);
		return -EINVAL;
	}
	for (p = relative; *p; p++) {
		if (!isalnum((unsigned char)*p) && *p != '.' && *p != '_' && *p != '-') {
			close(parent);
			return -EINVAL;
		}
	}
	*name = relative;
	return parent;
}

static int sensor_registry_open(const char *path, const char *mode)
{
	const char *name;
	int parent, fd, flags;
	if (!strcmp(mode, "w") || !strcmp(mode, "wb"))
		flags = O_WRONLY | O_CREAT | O_TRUNC;
	else if (!strcmp(mode, "a") || !strcmp(mode, "ab"))
		flags = O_WRONLY | O_CREAT | O_APPEND;
	else if (!strcmp(mode, "w+") || !strcmp(mode, "wb+"))
		flags = O_RDWR | O_CREAT | O_TRUNC;
	else if (!strcmp(mode, "r+") || !strcmp(mode, "rb+"))
		flags = O_RDWR;
	else
		return -EINVAL;
	parent = sensor_registry_parent(path, &name);
	if (parent < 0)
		return parent;
	fd = openat(parent, name, flags | O_CLOEXEC | O_NOFOLLOW, 0600);
	if (fd < 0)
		fd = -errno;
	close(parent);
	return fd;
}

static int sensor_registry_remove(const char *path)
{
	const char *name;
	int parent = sensor_registry_parent(path, &name);
	int ret;
	if (parent < 0)
		return parent;
	ret = unlinkat(parent, name, 0);
	if (ret < 0)
		ret = -errno;
	close(parent);
	return ret;
}

#endif
