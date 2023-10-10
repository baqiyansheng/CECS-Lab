#include <stdint.h>
#include <iostream>
#include <assert.h>
#include "include/debug.h"

extern uint8_t pmem[];

// load a binary file into your cpu
uint64_t load_img(char *img_file)
{

    // Lab2 TODO: load the 'img_file' to the start of pmem, and return its size
    FILE *file = fopen(img_file, "rb");
    if (file == NULL)
    {
        perror("Failed to open file");
        return 0;
    }
    fseek(file, 0, SEEK_END);         // 将文件指针挪到文件尾
    uint64_t file_size = ftell(file); // 计算文件大小
    fseek(file, 0, SEEK_SET);         // 将文件指针挪到文件头

    // Read the file into pmem
    size_t bytes_read = fread(pmem, sizeof(uint64_t), file_size, file);

    if (ferror(file))
    {
        perror("Error reading file");
        fclose(file);
        return 0;
    }
    fclose(file);
    return bytes_read;
}