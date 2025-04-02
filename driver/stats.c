#include "montemp.h"
#include <linux/kstrtox.h>
#define MAX_MEASUREMENTS 100


static u64 prev_active_time = 0;
static u64 prev_total_time = 0;

unsigned long calculate_cpu_load(void)
{
    u64 active_time, total_time;
    u64 delta_active, delta_total;
    unsigned long load;

    active_time = kcpustat_cpu(0).cpustat[CPUTIME_USER] +
                  kcpustat_cpu(0).cpustat[CPUTIME_NICE] +
                  kcpustat_cpu(0).cpustat[CPUTIME_SYSTEM];

    total_time = active_time +
                 kcpustat_cpu(0).cpustat[CPUTIME_IDLE] +
                 kcpustat_cpu(0).cpustat[CPUTIME_IOWAIT] +
                 kcpustat_cpu(0).cpustat[CPUTIME_IRQ] +
                 kcpustat_cpu(0).cpustat[CPUTIME_SOFTIRQ];

    delta_active = active_time - prev_active_time;
    delta_total = total_time - prev_total_time;

    prev_active_time = active_time;
    prev_total_time = total_time;

    if (delta_total == 0)
    {
        load = 0;
    }
    else
    {
        load = (unsigned long)((delta_active * 100ULL) / delta_total);
    }
    // printk(KERN_INFO "active_time: %llu, total_time: %llu\n", active_time, total_time);
    return load;
}

size_t list_size(struct list_head *list)
{
    struct list_head *pos;
    size_t count = 0;

    list_for_each(pos, list)
    {
        count++;
    }

    return count;
}

void limit_measurement_list(void)
{
    struct temp_measurement *measurement;
    //size_t freed_memory = 0;

    spin_lock(&measurement_lock);

    while (list_size(&measurement_list) > MAX_MEASUREMENTS)
    {
        measurement = list_first_entry(&measurement_list, struct temp_measurement, list);
        list_del(&measurement->list);
        kfree(measurement);
        //freed_memory += sizeof(*measurement);
    }

    spin_unlock(&measurement_lock);

    //if (freed_memory > 0)
    //{
    //    pr_info("Removed old measurements. Freed memory: %zu bytes\n", freed_memory);
    //}
}

void add_measurement(const char *json_data)
{
    struct temp_measurement *measurement;
    size_t allocated_memory = 0;

    measurement = kmalloc(sizeof(*measurement), GFP_KERNEL);
    if (!measurement)
    {
        pr_err("Failed to allocate memory for measurement\n");
        return;
    }
    allocated_memory += sizeof(*measurement);

    measurement->time = ktime_get_real_seconds();
    strncpy(measurement->data, json_data, sizeof(measurement->data) - 1);
    measurement->data[sizeof(measurement->data) - 1] = '\0';

    spin_lock(&measurement_lock);
    list_add_tail(&measurement->list, &measurement_list);
    spin_unlock(&measurement_lock);

    // pr_info("Added measurement. Allocated memory: %zu bytes\n", allocated_memory);

    limit_measurement_list();
}

void update_stats(void)
{
    char temp_buf[16];
    int temp, max_temp = 0, crit_temp = 0;
    char label[128];
    size_t offset = 0;
    char *json_data;
    int MAX_SIZE_BUFFER = 5120;
    size_t allocated_memory = 0;

    json_data = kmalloc(MAX_SIZE_BUFFER, GFP_KERNEL);
    if (!json_data)
    {
        pr_err("Failed to allocate memory for JSON data\n");
        return;
    }
    allocated_memory += MAX_SIZE_BUFFER;

    offset += snprintf(json_data + offset, MAX_SIZE_BUFFER - offset, "{\n");
    offset += snprintf(json_data + offset, MAX_SIZE_BUFFER - offset,
                       "  \"time\": %lld,\n", (long long)ktime_get_real_seconds());
    offset += snprintf(json_data + offset, MAX_SIZE_BUFFER - offset,
                       "  \"CPU\": %lu,\n", calculate_cpu_load());
    offset += snprintf(json_data + offset, MAX_SIZE_BUFFER - offset, "  \"thermal_sensors\": [\n");

    for (int i = 0; i < hwmon_device_count; i++)
    {
        if (read_file(hwmon_devices[i].temp_path, temp_buf, sizeof(temp_buf),1000) == 0)
        {
            if (kstrtoint(temp_buf, 10, &temp) == 0)
            {
                if (file_exists(hwmon_devices[i].max_temp_path))
                {
                    if (read_file(hwmon_devices[i].max_temp_path, temp_buf, sizeof(temp_buf),1000) == 0)
                    {
                        kstrtoint(temp_buf, 10, &max_temp);
                    }
                    else
                    {
                        max_temp = 0;
                    }
                }
                else
                {
                    max_temp = 0;
                }

                if (file_exists(hwmon_devices[i].crit_temp_path))
                {
                    if (read_file(hwmon_devices[i].crit_temp_path, temp_buf, sizeof(temp_buf),1000) == 0)
                    {
                        kstrtoint(temp_buf, 10, &crit_temp);
                    }
                    else
                    {
                        crit_temp = 0;
                    }
                }
                else
                {
                    crit_temp = 0;
                }

                if (file_exists(hwmon_devices[i].label))
                {
                    if (read_file(hwmon_devices[i].label, temp_buf, sizeof(temp_buf),1000) != 0)
                    {
                        strncpy(label, "", sizeof(label));
                    }
                    temp_buf[strcspn(temp_buf, "\n")] = 0;
                    strncpy(label, temp_buf, sizeof(label) - 1);
                }
                else
                {
                    strncpy(label, "", sizeof(label));
                }

                offset += snprintf(
                    json_data + offset,
                    MAX_SIZE_BUFFER - offset,
                    "    { \"type\": \"%s\", \"temp\": %d, \"max_temp\": %d, \"crit_temp\": %d, \"label\": \"%s\" } %s\n",
                    hwmon_devices[i].name,
                    temp / 1000,
                    max_temp / 1000,
                    crit_temp / 1000,
                    label,
                    (i == hwmon_device_count - 1) ? "" : ",");
            }
        }
        else
        {
            pr_err("Failed to read temperature for hwmon device %s\n", hwmon_devices[i].name);
        }
    }

    if (offset >= MAX_SIZE_BUFFER)
    {
        pr_err("JSON buffer overflow\n");
        kfree(json_data);
        allocated_memory -= MAX_SIZE_BUFFER;
        return;
    }

    offset += snprintf(json_data + offset, MAX_SIZE_BUFFER - offset, "  ]\n}");
    add_measurement(json_data);
    kfree(json_data);

    allocated_memory -= MAX_SIZE_BUFFER;
    // pr_info("Updated stats. Allocated memory: %zu bytes\n", allocated_memory);

    if (allocated_memory != 0)
    {
        pr_err("Memory leak detected in update_stats: %zu bytes not freed\n", allocated_memory);
    }
}

int stats_proc_show(struct seq_file *m, void *v)
{
    struct temp_measurement *measurement;
    seq_printf(m, "{\n");
    seq_printf(m, "  \"history\": [\n");

    spin_lock(&measurement_lock);
    list_for_each_entry(measurement, &measurement_list, list)
    {
        seq_printf(
            m, "    %s%s\n",
            measurement->data,
            (measurement->list.next == &measurement_list) ? "" : ",");
    }
    spin_unlock(&measurement_lock);
    seq_printf(m, "  ]\n}");
    return 0;
}

int stats_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, stats_proc_show, NULL);
}

const struct proc_ops stats_proc_fops = {
    .proc_open = stats_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};
