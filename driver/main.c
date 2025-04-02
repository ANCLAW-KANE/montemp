#include "montemp.h"

LIST_HEAD(measurement_list);
DEFINE_SPINLOCK(measurement_lock);

struct hwmon_info hwmon_devices[MAX_THERMAL_ZONES];
int hwmon_device_count = 0;

struct task_struct *temp_thread;

int montemp(void *data)
{
    while (!kthread_should_stop())
    {
        update_stats();
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(msecs_to_jiffies(POLL_INTERVAL));
    }

    return 0;
}

int __init temp_init(void)
{
    pr_alert("Initializing thermal and hwmon monitor\n");
    find_hwmon_devices();

    if (hwmon_device_count == 0)
    {
        pr_err("No thermal zones or hwmon devices found\n");
        return -ENODEV;
    }

    proc_create("thermal_stats", 0, NULL, &stats_proc_fops);

    temp_thread = kthread_run(montemp, NULL, "temp_monitor");
    if (IS_ERR(temp_thread))
    {
        pr_err("Failed to create temperature monitoring thread\n");
        remove_proc_entry("thermal_stats", NULL);
        return PTR_ERR(temp_thread);
    }

    pr_info("Temperature monitoring started\n");
    return 0;
}

void __exit temp_exit(void)
{
    if (temp_thread)
    {
        kthread_stop(temp_thread);
        pr_info("Temperature monitoring stopped\n");
    }
    remove_proc_entry("thermal_stats", NULL);
    pr_info("Thermal and hwmon monitor unloaded\n");
}

module_init(temp_init);
module_exit(temp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kane");
MODULE_DESCRIPTION("Мониторинг всех thermal zones и hwmon устройств в ядре Linux");