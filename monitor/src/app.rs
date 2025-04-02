
use crate::sensor::SensorData;
use std::time::Duration;

pub struct ThermalApp {
    pub history: Vec<(f64, f64, Vec<SensorData>)>, // (time, cpu, sensors)
    pub update_interval: Duration,
    pub selected_sensor: usize,
    pub sensor_names: Vec<String>,
    pub sensor_offset: usize,
}

impl ThermalApp {
    pub fn new() -> Self {
        Self {
            history: Vec::new(),
            update_interval: Duration::from_millis(1000),
            selected_sensor: 0,
            sensor_names: Vec::new(),
            sensor_offset: 0,
        }
    }

}
