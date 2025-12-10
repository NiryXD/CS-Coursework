/*
 * Program Description
 * -------------------
 * What this file does:
 *   Thin, low-level wrappers around POSIX file calls using file descriptors.
 *   - open_file : open a file for reading and return its descriptor
 *   - read_bytes: read up to N bytes from a file descriptor into a buffer
 *   - seek      : move the file's read/write cursor (like fseek for file* but for fd)
 *   - tell      : report the current cursor position (like ftell)
 *   - close_file: close the file descriptor
 *
 * Big picture in plain terms:
 *   - A file descriptor (fd) is just an integer handle the OS gives you when you open a file.
 *   - You can read() bytes from it, lseek() to jump around, and close() it when done.
 *
 * Notes & caveats:
 *   - On error, these system calls usually return -1. Your code should check for that.
 *   - read() can return fewer bytes than you asked for (short read). That is normal.
 *   - lseek() returns the new offset (type off_t). Here it's cast to int in tell().
 *     For very large files, consider using off_t/ssize_t instead of int to avoid truncation.
 */

#include <fcntl.h>   // open, O_* flags
#include <unistd.h>  // read, lseek, close

// Open a file read-only.
// Returns a non-negative file descriptor on success, or -1 on error.
int open_file(const char *file_name)
{
    return open(file_name, O_RDONLY);
}

// Read up to max_size bytes from fd into 'bytes'.
// Returns number of bytes actually read (0 means EOF), or -1 on error.
//
// Important: A return less than max_size is not necessarily an error;
// it can happen at EOF or for many other reasons (e.g., interrupted calls).
int read_bytes(int fd, char bytes[], int max_size)
{
    return (int)read(fd, bytes, max_size);
}

// Move the file cursor.
// 'offset' is how far to move; 'whence' is where to measure from:
//   SEEK_SET: from start of file
//   SEEK_CUR: from current position
//   SEEK_END: from end of file
// On error, lseek() returns (off_t)-1, but this wrapper ignores the return.
// In real code, you may want to check the result to detect errors.
void seek(int fd, int offset, int whence)
{
    lseek(fd, offset, whence);
}

// Report the current file cursor position.
// Internally calls lseek(fd, 0, SEEK_CUR).
// Returns the position as an int (or -1 on error).
// Note: For very large files, the true offset might not fit in an int.
// Consider returning off_t in more robust code.
int tell(int fd)
{
    return (int)lseek(fd, 0, SEEK_CUR);
}

// Close the file descriptor. Returns nothing here,
// but close() returns 0 on success, -1 on error.
// In real code, you may want to check that return value.
void close_file(int fd)
{
    close(fd);
}
