#include "montemp.h"

void find_hwmon_devices(void)
{
    char path[128];
    char name_buf[64];
    int i, j;
    int thermal_sensors = 10;

    for (i = 0; i < MAX_THERMAL_ZONES; i++)
    {
        snprintf(path, sizeof(path), "%shwmon%d/name", HWMON_PATH, i);
        if (read_file(path, name_buf, sizeof(name_buf),1000) == 0)
        {
            name_buf[strcspn(name_buf, "\n")] = '\0';

            for (j = 1; j <= thermal_sensors; j++)
            {
                snprintf(path, sizeof(path), "%shwmon%d/temp%d_input", HWMON_PATH, i, j);

                if (file_exists(path))
                {
                    strncpy(
                        hwmon_devices[hwmon_device_count].name,
                        name_buf,
                        sizeof(hwmon_devices[hwmon_device_count].name) - 1);
                    snprintf(
                        hwmon_devices[hwmon_device_count].temp_path,
                        sizeof(hwmon_devices[hwmon_device_count].temp_path),
                        "%shwmon%d/temp%d_input",
                        HWMON_PATH, i, j);
                    snprintf(
                        hwmon_devices[hwmon_device_count].max_temp_path,
                        sizeof(hwmon_devices[hwmon_device_count].max_temp_path),
                        "%shwmon%d/temp%d_max",
                        HWMON_PATH, i, j);
                    snprintf(
                        hwmon_devices[hwmon_device_count].crit_temp_path,
                        sizeof(hwmon_devices[hwmon_device_count].crit_temp_path),
                        "%shwmon%d/temp%d_crit",
                        HWMON_PATH, i, j);
                    snprintf(
                        hwmon_devices[hwmon_device_count].label,
                        sizeof(hwmon_devices[hwmon_device_count].label),
                        "%shwmon%d/temp%d_label",
                        HWMON_PATH, i, j);

                    hwmon_device_count++;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            pr_info("No more hwmon devices found after %d\n", i);
            break;
        }
    }
}
