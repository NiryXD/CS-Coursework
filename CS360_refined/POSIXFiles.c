#include <fcntl.h>
#include <unistd.h>

int open_file(const char *file_name)
{
    return open(file_name, O_RDONLY);
}

int read_bytes(int fd, char bytes[], int max_size)
{
    return (int)read(fd, bytes, max_size);
}

void seek(int fd, int offset, int whence)
{
    lseek(fd, offset, whence);
}

int tell(int fd)
{
    return (int)lseek(fd, 0, SEEK_CUR);
}

void close_file(int fd)
{
    close(fd);
}
