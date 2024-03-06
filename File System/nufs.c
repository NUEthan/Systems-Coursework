// based on cs3650 starter code

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "files/inode.h"
#include "files/storage.h"
#include "files/directory.h"

#define FUSE_USE_VERSION 30
#include <fuse.h>

//============================================================= nufs_access =//
// implementation for: man 2 access
// Checks if a file exists.
// Args:
// - path: Path to file
// - mask: Permissions mask
// Return 0 if successful, else -1
int nufs_access(const char *path, int mask) {
  int inum = tree_lookup(path);  // inode for given path

  if (inum == -1) {
    return -1;  // File not found
  }

  inode_t *node = get_inode(inum);  // Get inode for given inum

  printf("access(%s, %04o) -> \n", path, mask);
  return node ? 0 : -1;
}

//============================================================= nufs_getattr =//
// Gets an object's attributes (type, permissions, size, etc).
// Implementation for: man 2 stat
// This is a crucial function.
// Args:
// - path: Path to file
// - st: Pointer to struct stat (attributes storage)
// Returns 0 if successful, else -ENOENT
int nufs_getattr(const char *path, struct stat *st) {
  int rv = storage_stat(path, -1, st);  // Fill stat struct

  if (rv < 0) {
    return -ENOENT;  // File not found
  }

  printf("getting attr\n");
  return rv;
}

//============================================================= nufs_readdir =//
// implementation for: man 2 readdir
// lists the contents of a directory
// Args:
// - path: Path to directory
// - buf: Buffer (store entries)
// - filler: Add entries to buf.
// - offset: Offset (start reading entries)
// - fi: Fuse file info
// Returns 0 if successful
int nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                 off_t offset, struct fuse_file_info *fi) {
  int rv = 0;
  dirent_node_t *items = storage_list(path, -1);  // Get list of entries
  int flag = 0;

  for (dirent_node_t *xs = items; xs != 0;) {
    struct stat st;
    printf("current item: %s\n", xs->entry.name);

    rv = storage_stat(NULL, xs->entry.inum, &st);  // Get stat for every entry
    assert(rv == 0);
    filler(buf, xs->entry.name, &st, 0);  // Add entry to buffer

    // Memory management for list
    dirent_node_t *to_del = xs;
    xs = to_struct((list_next(&xs->dirent_list)), dirent_node_t, dirent_list);
    list_del(&to_del->dirent_list);
    free(to_del);

    if (to_del == xs) {
      break;
    }
  }
  printf("readdir(%s) -> %d\n", path, rv);
  return 0;
}

//============================================================= nufs_mknod =//
// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
// Note, for this assignment, you can alternatively implement the create
// function.
// Args:
// - path: Path to where object is created
// - mode: bits (of file)
// - rdev: Device ID (Didn't use)
// Returns 0 on success, -1 on failure.
int nufs_mknod(const char *path, mode_t mode, dev_t rdev) {
  char *directory = malloc(strlen(path) + 1);
  char *name = malloc(strlen(path) + 1);
  split_path(path, directory, name);  // Split path to directory & file name
  int rv = storage_mknod(directory, name, -1, mode);  // Create file object

  free(directory);
  free(name);

  printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

//============================================================= nufs_mkdir =//
// most of the following callbacks implement
// another system call; see section 2 of the manual
// Create directory at path
// Args:
// - path: Path to where directory is created
// - mode: Permission mode (directory)
// Returns 0 if successful, else negative
int nufs_mkdir(const char *path, mode_t mode) {
  int rv = nufs_mknod(path, mode | 040000, 0);
  int inum = tree_lookup(path);  // Find inum (directory)
  inode_t *node = get_inode(inum);  // Get inode (directory)

  char *from_parent = malloc(strlen(path) + 1);
  split_path(path, from_parent, NULL);  // Split path/ Get parent dir
  int parent_inum = tree_lookup(from_parent);  // Find inum (parent directory)

  free(from_parent);

  if (rv >= 0) {
    directory_put(node, ".", inum);  // Add entry (directory
    directory_put(node, "..", parent_inum); // Add entry (parent directory)
  }

  printf("mkdir(%s) -> %d\n", path, rv);
  return rv;
}

//============================================================= nufs_unlink =//
// unlinks file from this path
// Delete file at path
// Args:
// - path: Path to file
// Returns 0 if successful, else negative
int nufs_unlink(const char *path) {
  char *directory = malloc(strlen(path) + 1);
  char *child = malloc(strlen(path) + 1);
  split_path(path, directory, child);  // Split path/ Get dir & filename

  int rv = storage_unlink(directory, -1, child);  // Unlink file

  // free allocated memory
  free(directory);
  free(child);

  printf("unlink(%s) -> %d\n", path, rv);
  return rv;
}

//============================================================= nufs_link =//
// links the files from the to paths
// Create link to file
// Args:
// - from: Path to source file
// - to: Path to link location
// Returns 0 if successful, else negative
int nufs_link(const char *from, const char *to) {
  char *to_parent = malloc(strlen(to) + 1);
  char *to_child = malloc(strlen(to) + 1);

  split_path(to, to_parent, to_child);  // Split link location path

  int rv = storage_link(from, -1, to_parent, -1, to_child);  // Create link
  printf("link(%s => %s) -> %d\n", from, to, rv);

  free(to_parent);
  free(to_child);
  return rv;
}

//============================================================= nufs_rmdir =//
// removes the directory from that path
// Args:
// - path: Path to directory
// Returns 0 if successful, else negative
int nufs_rmdir(const char *path) {
  int inum = tree_lookup(path);  // Find inum (directory)
  inode_t *node = get_inode(inum);  // Get inode (diretory)

  int mode = node->mode;  // Get node of inode

  // Check mode --> directory
  if (mode != 040755) {
    printf("rmdir(%s) -> %d\n", path, -1);
    return -1;  // Not directory
  }
  return nufs_unlink(path);  // Unlink to remove directory
}

//============================================================= nufs_rename =//
// implements: man 2 rename
// called to move a file within the same filesystem
// Args:
// - from: OG path
// - to: New path
// Returns 0 if successful, else negative
int nufs_rename(const char *from, const char *to) {
  char *from_parent = malloc(strlen(from) + 1);
  char *from_child = malloc(strlen(from) + 1);
  char *to_parent = malloc(strlen(to) + 1);
  char *to_child = malloc(strlen(to) + 1);

  split_path(from, from_parent, from_child);  // Split source path
  split_path(to, to_parent, to_child);  // Split final path

  int rv = storage_rename(from_parent, -1, from_child, to_parent, -1, to_child);  // Rename
  printf("rename(%s => %s) -> %d\n", from, to, rv);

  free(from_parent);
  free(from_child);
  free(to_parent);
  free(to_child);
  return rv;
}

//============================================================= nufs_chmod =//
// changes permissions
// Args:
// - path: Path to file/directory.
// - mode: New permission mode
// Returns 0 if successful, else negative
int nufs_chmod(const char *path, mode_t mode) {
  int rv = -1;
  int inum = tree_lookup(path);  // Find inum (path)
  inode_t *node = get_inode(inum);  // Get inode (path)

  if (node->mode == mode) {
    return 0;  // no change
  } else {
    node->mode = mode;  // Update mode
    rv = 0;
  }

  printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
  return rv;
}

//============================================================= nufs_truncate =//
// truncates file/dir by the passed in size
// Args:
// - path: Path to file
// - size: New file size
// Returns 0 if successful, else negative
int nufs_truncate(const char *path, off_t size) {
  int rv = storage_truncate(path, size);
  printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
  return rv;
}

//============================================================= nufs_open =//
// This is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
// You can just check whether the file is accessible.
// Args:
// - path: Path to file
// - fi: Fuse file info (didn't use)
// Returns 0 if successful, else negative
int nufs_open(const char *path, struct fuse_file_info *fi) {
  int rv = nufs_access(path, 0);  // Check if can access file
  printf("open(%s) -> %d\n", path, rv);
  return rv;
}

//============================================================= nufs_read =//
// Actually read data
// Args:
// - path: Path to file
// - buf: Buffer (store read data)
// - size: # bytes to read
// - offset: Offset (start reading)
// - fi: Fuse file info,(didn't use)
// Returns # bytes read if successful, else negative
int nufs_read(const char *path, char *buf, size_t size, off_t offset,
              struct fuse_file_info *fi) {
  int rv = storage_read(path, -1, buf, size, offset);  // Read
  printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

//============================================================= nufs_write =//
// Actually write data
// Writes data to file
// Args:
// - path: Path to file
// - buf: Buffer (store data to write)
// - size: # bytes to write
// - offset: Offset (start writing in file)
// - fi: Fuse file info, (didn't use)
// Returns # bytes written if successful, else negative.
int nufs_write(const char *path, const char *buf, size_t size, off_t offset,
               struct fuse_file_info *fi) {
  int rv = storage_write(path, -1, buf, size, offset);  // Write
  printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
  return rv;
}

//============================================================= nufs_utimens =//
// Update the timestamps on a file or directory.
// Args:
// - path: Path to file/directory
// - ts: Array of timespec struct, ts[0] = time last accessed, ts[1] = time last mod
// Returns 0 if successful, else -1
int nufs_utimens(const char *path, const struct timespec ts[2]) {
  int inum = tree_lookup(path);  // Find inum (path)
  inode_t *node = get_inode(inum);  // Get inode (path)
  time_t time = ts->tv_sec;  // Update time modded
  node->mtime = time;

  if (node == NULL) {
    return -1;  // Inode not found
  }

  printf("utimens(%s, [%ld, %ld; %ld, %ld]) -> %d\n", path, ts[0].tv_sec,
         ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, 1);
  return 0;
}

//============================================================= nufs_ioctl =//
// Extended operations
// Args:
// - path: Path to file/directory
// - cmd: Cmd to perform
// - arg: Args for cmd
// - fi: Fuse file info (didn't use)
// - flags: Flags for ioctl
// - data: Data for cmd.
// Returns 0 if successful (always --> placeholder)
int nufs_ioctl(const char *path, int cmd, void *arg, struct fuse_file_info *fi,
               unsigned int flags, void *data) {
  int rv = 0;
  printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
  return rv;
}

//============================================================= struct =//
static const struct fuse_operations nufs_ops = {
  // Init Fuse filesystem ops
  // each field --> implementation of op
  .access = nufs_access,
  .getattr = nufs_getattr,
  .readdir = nufs_readdir,

  .mknod = nufs_mknod,
  .mkdir = nufs_mkdir,
  .link = nufs_link,

  .unlink = nufs_unlink,
  .rmdir = nufs_rmdir,
  .rename = nufs_rename,

  .chmod = nufs_chmod,
  .truncate = nufs_truncate,
  .open = nufs_open,

  .read = nufs_read,
  .write = nufs_write,
  .utimens = nufs_utimens,
  .ioctl = nufs_ioctl,
};

//============================================================= main =//
int main(int argc, char *argv[]) {
  assert(argc > 2 && argc < 6);

  // initalize blocks
  storage_init(argv[--argc]);
  return fuse_main(argc, argv, &nufs_ops, NULL);
}
