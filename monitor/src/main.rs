mod app;
mod utils;
mod sensor;
mod data;
mod draw;
mod chart;

use crossterm::cursor::{Hide, Show};
use crossterm::terminal::{disable_raw_mode, enable_raw_mode};
use crossterm::{
    event::{self, DisableMouseCapture, EnableMouseCapture, KeyCode},
    execute,
    terminal::{EnterAlternateScreen, LeaveAlternateScreen},
};
use ratatui::prelude::CrosstermBackend;
use ratatui::Terminal;
use std::io;

use crate::app::ThermalApp;

fn main() -> io::Result<()> {
    let mut app = ThermalApp::new();
    let mut stdout = io::stdout();
    execute!(stdout, EnterAlternateScreen, EnableMouseCapture, Hide)?;
    let backend = CrosstermBackend::new(stdout);
    let mut terminal = Terminal::new(backend)?;
    enable_raw_mode()?;

    loop {
        if event::poll(app.update_interval)? {
            match event::read()? {
                event::Event::Key(key) => match key.code {
                    KeyCode::Char('q') => break,
                    KeyCode::Left => {
                        if key.modifiers.contains(event::KeyModifiers::SHIFT) {
                            if app.sensor_offset > 0 {
                                app.sensor_offset -= 1;
                            }
                        } else {
                            if app.selected_sensor > 0 {
                                app.selected_sensor -= 1;
                            }
                        }
                    }
                    KeyCode::Right => {
                        if key.modifiers.contains(event::KeyModifiers::SHIFT) {
                            if app.sensor_offset + 1 < app.sensor_names.len() {
                                app.sensor_offset += 1;
                            }
                        } else {
                            if app.selected_sensor + 1 < app.sensor_names.len() {
                                app.selected_sensor += 1;
                            }
                        }
                    }

                    _ => {}
                },
                _ => {}
            }
        }

        app.update_data();
        terminal.draw(|f| app.draw(f))?;
    }

    disable_raw_mode()?;
    execute!(
        terminal.backend_mut(),
        LeaveAlternateScreen,
        DisableMouseCapture,
        Show
    )?;

    Ok(())
}
