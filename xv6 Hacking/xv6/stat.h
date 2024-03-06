#define T_DIR  1     // Directory
#define T_FILE 2     // File
#define T_DEV  3     // Device

// Store File IO stats, stop if exec syscall
struct iostats {
  int read_bytes;   // Number of bytes read
  int write_bytes;  // Number of bytes written
};

int getiostats(int fd, struct iostats* stats);

struct stat {
  short type;      // Type
  int dev;         // Disk device
  uint ino;        // Inode #
  short nlink;     // # links
  uint size;       // Size (bytes)
};
