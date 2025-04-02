use std::{fs, io};

pub fn read_process_status() -> io::Result<String> {
    fs::read_to_string("/proc/thermal_stats")
}

pub fn smooth_data(data: &[(f64, f64)]) -> Vec<(f64, f64)> {
    let mut smoothed = Vec::new();
    let window_size = 5;

    for i in 0..data.len() {
        let start = i.saturating_sub(window_size);
        let end = (i + window_size).min(data.len());
        let avg = data[start..end].iter().map(
            |(_, y)| *y
        ).sum::<f64>() / (end - start) as f64;
        smoothed.push((data[i].0, avg));
    }

    smoothed
}
