// ======================================================================== //
// author:  ixty                                                       2018 //
// project: inline c runtime library                                        //
// licence: beerware                                                        //
// ======================================================================== //
// utilities (read_file, read /proc/pid/maps, ...)
#ifndef _ICRT_UTILS_H
#define _ICRT_UTILS_H

#include <stdbool.h>
#include <unistd.h>
#include "icrt_asm.h"

// A utility function to reverse a string
void reverse(char str[], int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        end--;
        start++;
    }
}

// Implementation of citoa()
char* citoa(int num, char* str, int base)
{
    int i = 0;
    bool isNegative = false;
 
    /* Handle 0 explicitly, otherwise empty string is
     * printed for 0 */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }
 
    // In standard itoa(), negative numbers are handled
    // only with base 10. Otherwise numbers are
    // considered unsigned.
    if (num < 0 && base == 10) {
        isNegative = true;
        num = -num;
    }
 
    // Process individual digits
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
 
    // If number is negative, append '-'
    if (isNegative)
        str[i++] = '-';
 
    str[i] = '\0'; // Append string terminator
 
    // Reverse the string
    reverse(str, i);
 
    return str;
}

// get file size, supports fds with no seek_end (such as /proc/pid/maps apparently...)
long _get_file_size(int fd)
{
    long fs, r;
    char tmp[0x1000];

    // can seek to end?
    fs = lseek(fd, 0, SEEK_END);
    if(fs > 0)
    {
        lseek(fd, 0, SEEK_SET);
        return fs;
    }

    fs = 0;
    do
    {
        r = read(fd, tmp, 0x1000);
        if(r < 0)
            return -1;
        else
            fs += r;
    } while(r > 0);

    lseek(fd, 0, SEEK_SET);
    return fs;
}

// allocate memory + read a file
int read_file(char * path, uint8_t ** out_buf, size_t * out_len)
{
    long fs, r, n=0;
    int fd;

    *out_buf = 0;
    *out_len = 0;

    // open file
    if((fd = open(path, O_RDONLY, 0)) < 0)
        return -1;

    // get file size
    if(!(fs = _get_file_size(fd)))
        return -1;
    *out_len = fs;

    // allocate mem
    if((*out_buf = realloc(0, fs)) == NULL)
        goto exit;

    // read file
    while(n < fs)
    {
        r = read(fd, *out_buf + n, fs);
        if(r < 0)
            goto exit;
        n += r;
        if(r == 0 && n != fs)
            goto exit;
    }
    close(fd);
    return 0;

exit:
    if(*out_buf)
    {
        free(*out_buf);
        *out_buf = 0;
    }
    close(fd);
    return -1;
}

int get_memmaps(int pid, uint8_t ** maps_buf, size_t * maps_len)
{
    char path[256];
    char pids[24];

    // convert pid to string
    if(citoa(pid, pids, 10) < 0)
        return -1;

    // build /proc/pid/maps string
    memset(path, 0, sizeof(path));
    strlcat(path, "/proc/", sizeof(path));
    strlcat(path, pids, sizeof(path));
    strlcat(path, "/maps", sizeof(path));

    // read file
    if(read_file(path, maps_buf, maps_len) < 0)
        return -1;

    return 0;
}

// returns max - low address (doesnt start with F or 7F) used by the process
unsigned long get_mapmax(int pid)
{
    char * maps_buf = NULL;
    size_t maps_len = 0;
    char * p;
    unsigned long max = 0;

    // read /proc/pid/maps
    if(get_memmaps(pid, (uint8_t**)&maps_buf, &maps_len) < 0)
        return -1;

    // parse all maps
    for(p = (char*)maps_buf; p >= maps_buf && p < maps_buf + maps_len; )
    {
        char *          endline = memmem(p, maps_len - (p - maps_buf), "\n", 1);
        char *          tmp     = NULL;
        unsigned long   e;
        unsigned long   t;

        strtoul(p, &tmp, 16);
        if(!tmp)
            continue;
        tmp ++;
        e = strtoul(tmp, NULL, 16);
        t = e;

        while(t > 0xff)
            t >>= 8;
        if(t != 0xff && t != 0x7f)
            max = e;

        p = endline + 1;
    }
    free(maps_buf);
    printf("> auto-detected manual mapping address 0x%lx\n", MAPS_ADDR_ALIGN(max));
    return MAPS_ADDR_ALIGN(max);
}
#endif