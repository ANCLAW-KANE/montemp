use crate::app::ThermalApp;
use crate::utils::read_process_status;
use crate::sensor::{ThermalHistory,SensorData};

impl ThermalApp {
    pub fn update_data(&mut self) {
        if let Ok(data) = read_process_status() {
            if let Ok(thermal_history) = serde_json::from_str::<ThermalHistory>(&data) {
                let new_data: Vec<(f64, f64, Vec<SensorData>)> = thermal_history
                    .history
                    .iter()
                    .map(|entry| {
                        let sensors = entry
                            .thermal_sensors
                            .iter()
                            .map(|sensor| SensorData {
                                label: sensor.label.clone(),
                                temp: sensor.temp as f64,
                                max_temp: sensor.max_temp as f64,
                                crit_temp: sensor.crit_temp as f64,
                                sensor_type: sensor.r#type.clone(),
                            })
                            .collect();
                        (entry.time as f64, entry.cpu as f64, sensors)
                    })
                    .collect();

                if !new_data.is_empty() && !new_data[0].2.is_empty() {
                    self.sensor_names = new_data[0].2.iter().map(|s| s.label.clone()).collect();
                }

                self.history = new_data;
            }
        }
    }


}
