
#include "montemp.h"



int file_exists(const char *path)
{
    struct path file_path;
    int ret;

    ret = kern_path(path, LOOKUP_FOLLOW, &file_path);
    if (ret == 0)
    {
        path_put(&file_path);
        return 1;
    }

    return 0;
}

int read_file(const char *path, char *buf, size_t buf_size, long timeout_ms)
{
    struct file *f;
    loff_t pos = 0;
    ssize_t bytes_read;
    unsigned long timeout = jiffies + msecs_to_jiffies(timeout_ms);

    f = filp_open(path, O_RDONLY, 0);
    if (IS_ERR(f)) {
        pr_err("Failed to open %s\n", path);
        return -1;
    }

    while (time_before(jiffies, timeout)) {
        bytes_read = kernel_read(f, buf, buf_size - 1, &pos);
        if (bytes_read > 0) {
            buf[bytes_read] = '\0';
            filp_close(f, NULL);
            return 0;
        }
        msleep(100);
    }

    filp_close(f, NULL);
    pr_err("Timeout while reading file %s\n", path);
    return -ETIMEDOUT;
}