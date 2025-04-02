use crate::app::ThermalApp;
use crate::utils::smooth_data;

use chrono::DateTime;

use ratatui::layout::Rect;
use ratatui::{
    prelude::*,
    widgets::{Axis, Block, Borders, Chart, Dataset},
};

impl ThermalApp {
    
    pub fn draw_chart(&self, f: &mut Frame, area: Rect) {
        let block = Block::default()
            .title(" Temperature Chart ")
            .borders(Borders::ALL);
        let inner_area = block.inner(area);

        f.render_widget(&block, area);

        if self.history.is_empty() {
            return;
        }

        

        let mut smoothed_data = Vec::new();

        let mut sensor_label = None;

        if let Some((_, _, sensors)) = self.history.first() {
            if self.selected_sensor < sensors.len() {
                if sensors[self.selected_sensor].label.is_empty() {
                    sensor_label = Some(sensors[self.selected_sensor].sensor_type.clone());
                } else {
                    sensor_label = Some(sensors[self.selected_sensor].label.clone());
                }

                let data: Vec<(f64, f64)> = self
                    .history
                    .iter()
                    .filter_map(|(x, _, s)| {
                        s.iter().find(|sensor| {
                            if sensors[self.selected_sensor].label.is_empty() {
                                sensor.sensor_type == sensors[self.selected_sensor].sensor_type
                            } else {
                                sensor.label == sensors[self.selected_sensor].label
                            }
                        })
                            .map(|sensor| (*x, sensor.temp))
                    })
                    .collect();

                smoothed_data.push(smooth_data(&data));
            }
        }

        let mut datasets = Vec::new();

        if let Some(label) = sensor_label {
            let dataset = Dataset::default()
                .name(label)
                .marker(symbols::Marker::Braille)
                .style(Style::default().fg(Color::Green))
                .data(&smoothed_data[0]);

            datasets.push(dataset);
        }

        let min_y = smoothed_data
            .iter()
            .flat_map(
                |data| data.iter().map(
                    |&(_, y)| y
                )
            )
            .fold(f64::MAX, f64::min);

        let max_y = smoothed_data
            .iter()
            .flat_map(
                |data| data.iter().map(
                    |&(_, y)| y
                )
            )
            .fold(f64::MIN, f64::max);

        // padding
        let min_y = (min_y - 5.0).max(0.0);
        let max_y = max_y + 5.0;
        
        // time bounds
        let min_x = self.history.first().map(
            |(x, _, _)| *x
        ).unwrap_or(0.0);
        let max_x = self
            .history
            .last()
            .map(|(x, _, _)| *x)
            .unwrap_or(min_x + 100.0);
        
        // time labels
        let min_x_str = DateTime::from_timestamp(min_x as i64, 0)
            .map(|dt| dt.format("%H:%M:%S").to_string())
            .unwrap_or_else(|| "Invalid Time".to_string());
        let max_x_str = DateTime::from_timestamp(max_x as i64, 0)
            .map(|dt| dt.format("%H:%M:%S").to_string())
            .unwrap_or_else(|| "Invalid Time".to_string());

        let d_y = max_y - min_y;
        
        let unit_y = d_y / 5 as f64;

        let y_1 = min_y + unit_y;
        let y_2 = min_y + 2.0 * unit_y;
        let y_3 = min_y + 3.0 * unit_y;
        let y_4 = min_y + 4.0 * unit_y;

        let chart = Chart::new(datasets)
            .x_axis(
                Axis::default()
                    .title(Span::styled("Time", Style::default().fg(Color::Red)))
                    .bounds([min_x, max_x])
                    .labels(vec![
                        Span::styled(min_x_str, Style::default().fg(Color::White)),
                        Span::styled(max_x_str, Style::default().fg(Color::White)),
                    ]),
            )
            .y_axis(
                Axis::default()
                    .title(Span::styled("Temp °C", Style::default().fg(Color::Red)))
                    .bounds([min_y, max_y])
                    .labels(vec![
                        Span::styled(format!("{:.0}", min_y), Style::default().fg(Color::White)),
                        Span::styled(format!("{:.0}", y_1), Style::default().fg(Color::White)),
                        Span::styled(format!("{:.0}", y_2), Style::default().fg(Color::White)),
                        Span::styled(format!("{:.0}", y_3), Style::default().fg(Color::White)),
                        Span::styled(format!("{:.0}", y_4), Style::default().fg(Color::White)),
                        Span::styled(format!("{:.0}", max_y), Style::default().fg(Color::White)),
                    ]),
            );

        f.render_widget(chart, inner_area);
    }
}
