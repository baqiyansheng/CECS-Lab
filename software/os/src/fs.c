#include <fs.h>
#include <debug.h>
#include <device.h>

// typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
// typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

// typedef struct {
//   char *  name;
//   size_t  size;
//   size_t  disk_offset;
//   bool    is_open;
//   ReadFn  read;
//   WriteFn write;
//   size_t  open_offset;
// } Finfo;

enum
{
    FD_STDIN,
    FD_STDOUT,
    FD_STDERR,
    FD_EVENTS,
    FD_FB,
    FD_DISPINFO
};

/* This is the information about all files in disk. */
Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, 0, invalid_read, serial_write},
    [FD_EVENTS] = {"/dev/events", 0, 0, 0, keyboard_read, invalid_write},
    [FD_FB] = {"/dev/fb", 0, 0, 0, invalid_read, fb_write},
    [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

/* Init the file system. */
void init_fs()
{
    Log("Initializing file system...");
    DEV_GPU_CONFIG_T gpu_config = io_read(DEV_GPU_CONFIG);
    file_table[FD_FB].size = gpu_config.height * gpu_config.width * 4;
}

/* Open a file and return the fd of file. */
int fs_open(const char *pathname, int flags, int mode)
{
    // Lab7 TODO: traverse the file_table and find the file
    int num_of_files = sizeof(file_table) / sizeof(file_table[0]);
    for (int i = 0; i < num_of_files; i++)
    {
        if (strcmp(pathname, file_table[i].name) == 0)
        {
            file_table[i].open_offset = 0;
            file_table[i].is_open = true;
            return i;
        }
    }
    return -1;
}

/* Read `len` bytes of data from the file with `fd` into `buf` */
size_t fs_read(int fd, void *buf, size_t len)
{
    // Lab7 TODO: read the file with fd from offset into buf
    if (fd <= 5)
    {
        int ret_len = file_table[fd].read(buf, file_table[fd].open_offset, len);
        // file_table[fd].open_offset += ret_len;
        return ret_len;
    }
    else if (file_table[fd].is_open)
    {
        len = len > file_table[fd].size - file_table[fd].open_offset ? file_table[fd].size - file_table[fd].open_offset : len;
        ramdisk_read(buf, file_table[fd].open_offset + file_table[fd].disk_offset, len);
        file_table[fd].open_offset += len;
        return len;
    }
    return -1;
}

/* Write `len` bytes of data from `buf` into the file with `fd` */
size_t fs_write(int fd, const void *buf, size_t len)
{
    // Lab7 TODO: write the file with fd from offset by buf
    if (fd <= 5)
    {
        int ret_len = file_table[fd].write(buf, file_table[fd].open_offset, len);
        // file_table[fd].open_offset += ret_len;
        return ret_len;
    }
    else if (file_table[fd].is_open)
    {
        len = len > file_table[fd].size - file_table[fd].open_offset ? file_table[fd].size - file_table[fd].open_offset : len;
        ramdisk_write(buf, file_table[fd].open_offset + file_table[fd].disk_offset, len);
        file_table[fd].open_offset += len;
        return len;
    }
    return -1;
}

/* Seek the file with `fd` from the `offset` based on `whence` */
size_t fs_lseek(int fd, size_t offset, int whence)
{
    // Lab7 TODO: seek the file with fd from the offset based on whence
    if (fd <= 5)
    {
        switch (whence)
        {
        case SEEK_SET:
        {
            file_table[fd].open_offset = offset;
            // Log("SEEK_SET: %d", file_table[fd].open_offset);
            return file_table[fd].open_offset;
            break;
        }
        case SEEK_CUR:
        {
            file_table[fd].open_offset += offset;
            // Log("SEEK_CUR: %d", file_table[fd].open_offset);
            return file_table[fd].open_offset;
            break;
        }
        case SEEK_END:
        {
            file_table[fd].open_offset = file_table[fd].size - offset;
            // Log("SEEK_END: %d", file_table[fd].open_offset);
            return file_table[fd].open_offset;
            break;
        }
        default:
            return -1;
        }
    }
    else if (file_table[fd].is_open)
    {
        switch (whence)
        {
        case SEEK_SET:
        {
            file_table[fd].open_offset = offset < file_table[fd].size? offset : file_table[fd].size;
            // Log("SEEK_SET: %d", file_table[fd].open_offset);
            return file_table[fd].open_offset;
            break;
        }
        case SEEK_CUR:
        {
            file_table[fd].open_offset = file_table[fd].open_offset + offset < file_table[fd].size? file_table[fd].open_offset + offset : file_table[fd].size;
            // Log("SEEK_CUR: %d", file_table[fd].open_offset);
            return file_table[fd].open_offset;
            break;
        }
        case SEEK_END:
        {
            file_table[fd].open_offset = file_table[fd].size + offset < file_table[fd].size? file_table[fd].size + offset : file_table[fd].size;
            // Log("SEEK_END: %d", file_table[fd].open_offset);
            return file_table[fd].open_offset;
            break;
        }
        default:
            return -1;
        }
    }
    return -1;
}

/* Close the file with `fd` */
int fs_close(int fd)
{
    // Lab7 TODO: close the file with fd
    if (file_table[fd].is_open)
    {
        file_table[fd].is_open = false;
        return 0;
    }
    return -1;
}