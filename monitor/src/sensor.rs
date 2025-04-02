use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct ThermalSensor {
    pub r#type: String,
    pub temp: i64,
    pub max_temp: i64,
    pub crit_temp: i64,
    pub label: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct HistoryEntry {
    pub time: i64,
    #[serde(rename = "CPU")]
    pub cpu: i64,
    pub thermal_sensors: Vec<ThermalSensor>,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct ThermalHistory {
    pub  history: Vec<HistoryEntry>,
}

pub struct SensorData {
    pub label: String,
    pub temp: f64,
    pub max_temp: f64,
    pub crit_temp: f64,
    pub sensor_type: String,
}

