#include "graphics.h"
#include "data_structures.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <iostream>
#include <algorithm>

// Forward declaration of draw function
void draw_main_canvas(ezgl::renderer *g);

void run_graphics() {
    std::cout << "\nOpening Graphics Window" << std::endl;
    
    // Calculate world bounds from block positions
    double min_x = 1e9, max_x = -1e9;
    double min_y = 1e9, max_y = -1e9;
    
    for (const auto& [id, block] : blocks) {
        if (block.x < min_x) min_x = block.x;
        if (block.x > max_x) max_x = block.x;
        if (block.y < min_y) min_y = block.y;
        if (block.y > max_y) max_y = block.y;
    }
    
    // Add 10% margin
    double margin_x = (max_x - min_x) * 0.1;
    double margin_y = (max_y - min_y) * 0.1;
    
    min_x -= margin_x;
    max_x += margin_x;
    min_y -= margin_y;
    max_y += margin_y;
    
    std::cout << "World bounds: [" << min_x << ", " << max_x << "] x ["
              << min_y << ", " << max_y << "]" << std::endl;
    
    // Setup EZGL application
    ezgl::application::settings settings;
    settings.main_ui_resource = "main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    // Define world coordinate system
    ezgl::rectangle initial_world({min_x, min_y}, {max_x, max_y});
    
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    
    std::cout << "Graphics window opened. Click 'Proceed' to continue." << std::endl;
    
    // Run the GUI (blocking call)
    application.run(nullptr, nullptr, nullptr, nullptr);
}

void draw_main_canvas(ezgl::renderer *g) {
    // Set background to white
    g->set_color(ezgl::WHITE);
    
    
    // STEP 1: Draw bin grid (40×40)
    
    g->set_color(ezgl::PINK);  // Pink for grid
    g->set_line_width(1);
    
    const int GRID_SIZE = 40;
    const double GRID_MIN = 0.0;
    const double GRID_MAX = 40.0;
    
    // Draw vertical lines
    for (int i = 0; i <= GRID_SIZE; i++) {
        double x = GRID_MIN + i;
        g->draw_line({x, GRID_MIN}, {x, GRID_MAX});
    }
    
    // Draw horizontal lines
    for (int i = 0; i <= GRID_SIZE; i++) {
        double y = GRID_MIN + i;
        g->draw_line({GRID_MIN, y}, {GRID_MAX, y});
    }
    
    // Draw thicker border around placement area
    g->set_color(ezgl::BLACK);
    g->set_line_width(3);
    g->draw_rectangle({GRID_MIN, GRID_MIN}, {GRID_MAX, GRID_MAX});
    
    
    // STEP 2: Draw nets (behind blocks)
    
    g->set_color(ezgl::GREY_75);  // Grey for nets (better contrast)
    g->set_line_width(1);
    
    for (const auto& [net_id, net] : nets) {
        if (net.blocks.size() < 2) continue;
        
        // Draw star configuration (all pins to first pin)
        int first_block = net.blocks[0];
        double x1 = blocks.at(first_block).x;
        double y1 = blocks.at(first_block).y;
        
        for (size_t i = 1; i < net.blocks.size(); i++) {
            int block_id = net.blocks[i];
            double x2 = blocks.at(block_id).x;
            double y2 = blocks.at(block_id).y;
            
            g->draw_line({x1, y1}, {x2, y2});
        }
    }
    
    
    // STEP 3: Draw blocks
    
    double block_size = 1;  // Assuming each block is 1x1 unit square
    
    for (const auto& [id, block] : blocks) {
        
        // Skip drawing anchor blocks (they're just imaginary)
        if (block.is_anchor) continue;

        double half_size = block_size / 2.0;
        
        // Choose color based on fixed vs moveable
        if (block.is_fixed) {
            g->set_color(ezgl::RED);  // Fixed blocks = red
        } else {
            if (block.type == 0) {
                g->set_color(ezgl::BLUE);  // Type 0 = blue
            } else {
                g->set_color(ezgl::GREEN);  // Type 1 = green
            }
        }
        
        // Draw filled rectangle for block
        ezgl::rectangle block_rect(
            {block.x - half_size, block.y - half_size},
            {block.x + half_size, block.y + half_size}
        );
        g->fill_rectangle(block_rect);
        
        // Draw black outline
        g->set_color(ezgl::BLACK);
        g->set_line_width(1);
        g->draw_rectangle(block_rect);
        
        // Draw block ID label (only if zoomed in enough)
        ezgl::rectangle visible = g->get_visible_world();
        double visible_width = visible.width();

        // Only show labels when zoomed in (visible width < 10)
        if (visible_width < 40.0) {
            g->set_color(ezgl::BLACK);
            g->set_font_size(20);
            g->draw_text({block.x, block.y}, std::to_string(block.id));
        }
    }
    
    
    // STEP 4: Draw legend
    
    g->set_color(ezgl::BLACK);
    g->set_font_size(14);
    
    // Get visible world to place legend
    ezgl::rectangle visible = g->get_visible_world();
    double legend_x = visible.left() + (visible.width() * 0.02);
    double legend_y = visible.top() - (visible.height() * 0.05);
    double legend_spacing = visible.height() * 0.04;
    double box_size = visible.height() * 0.015;
    
    g->draw_text({legend_x, legend_y}, "Legend:");
    legend_y -= legend_spacing;
    
    // Red = Fixed I/O
    g->set_color(ezgl::RED);
    g->fill_rectangle({{legend_x - box_size, legend_y - box_size}, 
                       {legend_x + box_size, legend_y + box_size}});
    g->set_color(ezgl::BLACK);
    g->draw_text({legend_x + box_size * 3, legend_y}, "Fixed I/O");
    legend_y -= legend_spacing;
    
    // Blue = Type 0
    g->set_color(ezgl::BLUE);
    g->fill_rectangle({{legend_x - box_size, legend_y - box_size}, 
                       {legend_x + box_size, legend_y + box_size}});
    g->set_color(ezgl::BLACK);
    g->draw_text({legend_x + box_size * 3, legend_y}, "Type 0 (Moveable)");
    legend_y -= legend_spacing;
    
    // Green = Type 1
    g->set_color(ezgl::GREEN);
    g->fill_rectangle({{legend_x - box_size, legend_y - box_size}, 
                       {legend_x + box_size, legend_y + box_size}});
    g->set_color(ezgl::BLACK);
    g->draw_text({legend_x + box_size * 3, legend_y}, "Type 1 (Moveable)");
    legend_y -= legend_spacing;
    
    // Grid info
    g->set_color(ezgl::BLACK);
    g->draw_text({legend_x, legend_y}, "Grid: 40x40 bins (1x1 each)");
}