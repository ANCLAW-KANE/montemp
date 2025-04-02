use crate::app::ThermalApp;

use ratatui::layout::{Constraint, Direction, Layout, Rect};
use ratatui::widgets::{Padding, Paragraph, Wrap};
use ratatui::{
    prelude::*,
    widgets::{Block, Borders, LineGauge},
};

impl ThermalApp {
    

    pub fn draw(&self, f: &mut Frame) {
        let size = f.area();

        let main_block = Block::default()
            .title(" Thermal Monitor (q to quit) ")
            .borders(Borders::ALL);
        let inner_area = main_block.inner(size);

        f.render_widget(&main_block, size);

        let chunks = Layout::default()
            .direction(Direction::Vertical)
            .constraints([
                Constraint::Length(3),
                Constraint::Length(7),
                Constraint::Min(10),
            ])
            .split(inner_area);

        let cpu = self.history.last().map(|(_, t, _)| *t).unwrap_or(0.0);
        let cpu_ratio = (cpu / 100.0).clamp(0.0, 1.0);
        let cpu_gauge = LineGauge::default()
            .block(Block::default().title(" CPU Usage "))
            .filled_style(
                Style::default()
                    .fg(match cpu {
                        t if t > 90.0 => Color::Red,
                        t if t > 70.0 => Color::Yellow,
                        _ => Color::Green,
                    })
                    .bg(Color::Black),
            )
            .ratio(cpu_ratio)
            .line_set(symbols::line::THICK);

        f.render_widget(cpu_gauge, chunks[0]);

        self.draw_sensor_info(f, chunks[1]);

        self.draw_chart(f, chunks[2]);
    }

    fn draw_sensor_info(&self, f: &mut Frame, area: Rect) {
        let block = Block::default()
            .title("| Sensors Information. Move: ←→. Scroll: shift + ←→ |")
            .borders(Borders::ALL);

        let inner_area = block.inner(area);

        f.render_widget(&block, area);

        if let Some((_, _, sensors)) = self.history.last() {
            let chunks = Layout::default()
                .direction(Direction::Horizontal)
                .constraints(
                    vec![
                    Constraint::Length(inner_area.width / 3),
                    Constraint::Length(inner_area.width / 3),
                    Constraint::Length(inner_area.width / 3)  
                ]
                )
                .split(inner_area);

            let visible_sensors = &sensors[self.sensor_offset..];
            let max_visible = chunks.len().min(visible_sensors.len());

            for (i, sensor) in visible_sensors.iter().take(max_visible).enumerate() {
                let is_selected = i + self.sensor_offset == self.selected_sensor;

                let style = if is_selected {
                    Style::default().fg(Color::Red).bg(Color::Black)
                } else {
                    Style::default()
                };

                let title = if sensor.sensor_type.is_empty() || sensor.label.is_empty() {
                    format!("{}{}", sensor.sensor_type, sensor.label)
                } else {
                    format!("{} | {}", sensor.sensor_type, sensor.label)
                };

                let text = format!(
                    "{}: {:.1}°C (Max: {:.1}, Crit: {:.1})",
                    title, sensor.temp, sensor.max_temp, sensor.crit_temp
                );

                let paragraph = Paragraph::new(text)
                    .block(
                        Block::default()
                            .style(style)
                            .padding(Padding::horizontal(1))
                            .borders(Borders::ALL),
                    )
                    .alignment(Alignment::Center)
                    .wrap(Wrap { trim: false });
                f.render_widget(paragraph, chunks[i]);
            }
        }
    }

    
}
