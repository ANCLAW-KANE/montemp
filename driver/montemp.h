#ifndef MONTEMP_H
#define MONTEMP_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/ktime.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/file.h>
#include <linux/kernel_stat.h>
#include <linux/tick.h>
#include <linux/jiffies.h>

#define HWMON_PATH "/sys/class/hwmon/"
#define THERMAL_PATH "/sys/class/thermal/"
#define POLL_INTERVAL 1000 
#define MAX_THERMAL_ZONES 32

struct temp_measurement
{
    struct list_head list;
    long long time;
    char data[4096];
};

struct hwmon_info
{
    char name[64];
    char temp_path[128];
    char max_temp_path[128];
    char crit_temp_path[128];
    char label[128];
};

extern struct list_head measurement_list;
extern spinlock_t measurement_lock;
extern struct hwmon_info hwmon_devices[MAX_THERMAL_ZONES];
extern int hwmon_device_count;
extern const struct proc_ops stats_proc_fops;


int file_exists(const char *path);
int read_file(const char *path, char *buf, size_t buf_size, long timeout_ms);
void find_hwmon_devices(void);
unsigned long calculate_cpu_load(void);
void add_measurement(const char *json_data);
void update_stats(void);
int stats_proc_show(struct seq_file *m, void *v);
int stats_proc_open(struct inode *inode, struct file *file);
void limit_measurement_list(void);
size_t list_size(struct list_head *list);
int montemp(void *data);
int __init temp_init(void);
void __exit temp_exit(void);

#endif 