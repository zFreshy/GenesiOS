use gtk4::prelude::*;
use gtk4::{Box, Orientation};

pub struct Panel {
    container: Box,
}

impl Panel {
    pub fn new() -> Self {
        let container = Box::new(Orientation::Horizontal, 0);
        container.add_css_class("panel");
        container.set_height_request(1); // Invisível
        container.set_hexpand(true);

        Self { container }
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
