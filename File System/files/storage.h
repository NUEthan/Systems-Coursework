// Disk storage abstracttion.
//
// Feel free to use as inspiration. Provided as-is.

// based on cs3650 starter code

#ifndef NUFS_STORAGE_H
#define NUFS_STORAGE_H

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "inode.h"
#include "slist.h"
#include "directory.h"


void storage_init(const char *path); // Init storage
int storage_stat(const char *path, int inum, struct stat *st);  // Get stats from file/directory
int storage_read(const char *path, int inum, char *buf, size_t size, off_t offset);  // Read data from file into buffer
int storage_write(const char *path, int inum, const char *buf, size_t size, off_t offset);  // Write data to file from buffer
int storage_truncate(const char *path, off_t size);  // Truncate file to size
int storage_mknod(const char *path, const char *name, int pinum, int mode);  // Create new file/directory
int storage_unlink(const char *path, int pinum, const char *name);  // Delete object at path
int storage_link(const char *from, int from_inum, const char *to_parent, int to_pinum, const char *to_child);  // Create hardlink between 2 paths
// Rename/move object from path1 to path2
int storage_rename(const char *from_parent, int from_pinum, const char *from_child, const char *to_parent, int to_pinum, const char *to_child);
dirent_node_t *storage_list(const char *path, int inum);  // List objects at path
void split_path(const char *path, char *directory, char *name);  // Split full path into directory & file parts

#endif
