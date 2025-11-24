#include "graphics.h"
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iostream>

// Global storage
std::vector<TreeNode> g_tree_nodes;
int g_max_depth = 0;
std::vector<int> g_block_order;
bool g_graphics_enabled = false;

// Local state for drawing
static std::string g_circuit_name;
static int g_optimal_cost;
static int g_nodes_visited;

int record_tree_node(int depth, int block_id, Side side, double x_position, int parent_index) {
    if (!g_graphics_enabled) return -1;
    
    TreeNode node;
    node.depth = depth;
    node.block_id = block_id;
    node.side = side;
    node.pruned = false;
    node.is_solution = false;
    node.x_position = x_position;  // Should stay in (0,1) naturally with standard scheme
    node.parent_index = parent_index;
    
    int index = (int)g_tree_nodes.size();
    g_tree_nodes.push_back(node);
    
    if (depth > g_max_depth) {
        g_max_depth = depth;
    }
    
    return index;
}

void mark_node_pruned(int node_index) {
    if (!g_graphics_enabled || node_index < 0) return;
    if (node_index < (int)g_tree_nodes.size()) {
        g_tree_nodes[node_index].pruned = true;
    }
}

void mark_node_solution(int node_index) {
    if (!g_graphics_enabled || node_index < 0) return;
    if (node_index < (int)g_tree_nodes.size()) {
        g_tree_nodes[node_index].is_solution = true;
    }
}

void clear_tree_nodes() {
    g_tree_nodes.clear();
    g_max_depth = 0;
}

// Main drawing callback
void draw_main_canvas(ezgl::renderer *g) {
    ezgl::rectangle world = g->get_visible_world();
    
    // White background
    g->set_color(ezgl::WHITE);
    g->fill_rectangle(world);
    
    if (g_tree_nodes.empty()) {
        g->set_color(ezgl::BLACK);
        g->set_font_size(14);
        g->draw_text({world.center_x(), world.center_y()}, "No tree data");
        return;
    }
    
    // node radius -> proportional to visible world width so it appears constant on screen
    double node_radius = world.width() / 400.0;
    
    // fixed canvas dimensions
    double full_width = 1000.0;
    double full_height = std::max(800.0, (g_max_depth + 2) * 20.0);
    
    double margin_left = 50.0;
    double margin_right = 10.0;
    double margin_top = 30.0;
    double margin_bottom = 50.0;
    
    double draw_width = full_width - margin_left - margin_right;
    double draw_height = full_height - margin_top - margin_bottom;
    double level_height = draw_height / (g_max_depth + 1);
    
    // Draw block labels on Y axis (FIXED positions in world coordinates)
    g->set_color(ezgl::BLACK);
    g->set_font_size(10);
    for (int d = 0; d <= g_max_depth && d < (int)g_block_order.size(); d++) {
        double y = full_height - margin_top - d * level_height - level_height / 2;
        // Only draw if visible
        if (y >= world.bottom() && y <= world.top()) {
            g->draw_text({margin_left / 2, y}, std::to_string(g_block_order[d]));
        }
    }
    
    // Draw grid lines (faint)
    g->set_color(ezgl::color(240, 240, 240));
    g->set_line_width(1);
    for (int d = 0; d <= g_max_depth + 1; d++) {
        double y = full_height - margin_top - d * level_height;
        if (y >= world.bottom() && y <= world.top()) {
            g->draw_line({margin_left, y}, {full_width - margin_right, y});
        }
    }
    
    // draw edges first, so nodes are on top
    for (size_t i = 0; i < g_tree_nodes.size(); i++) {
        const auto& node = g_tree_nodes[i];
        if (node.parent_index < 0) continue;
        
        const auto& parent = g_tree_nodes[node.parent_index];
        
        double x = margin_left + node.x_position * draw_width;
        double y = full_height - margin_top - node.depth * level_height - level_height / 2;
        double px = margin_left + parent.x_position * draw_width;
        double py = full_height - margin_top - parent.depth * level_height - level_height / 2;
        
        // Skip if completely outside visible area
        double min_x = std::min(x, px);
        double max_x = std::max(x, px);
        double min_y = std::min(y, py);
        double max_y = std::max(y, py);
        
        if (max_x < world.left() || min_x > world.right()) continue;
        if (max_y < world.bottom() || min_y > world.top()) continue;
        
        if (node.pruned) {
            g->set_color(ezgl::RED);
        } else if (node.is_solution) {
            g->set_color(ezgl::GREEN);
        } else {
            g->set_color(ezgl::BLACK);
        }
        g->set_line_width(1);
        g->draw_line({px, py}, {x, y});
    }
    
    // Draw nodes
    for (const auto& node : g_tree_nodes) {
        double x = margin_left + node.x_position * draw_width;
        double y = full_height - margin_top - node.depth * level_height - level_height / 2;
        
        // Skip if outside visible area
        if (x < world.left() - node_radius || x > world.right() + node_radius) continue;
        if (y < world.bottom() - node_radius || y > world.top() + node_radius) continue;
        
        if (node.is_solution) {
            g->set_color(ezgl::GREEN);
        } else if (node.pruned) {
            g->set_color(ezgl::RED);  // Pruned = red
        } else {
            g->set_color(ezgl::BLACK);
        }
        
        g->fill_arc({x, y}, node_radius, 0, 360);
    }
    
    // Legend at bottom left of visible area (moves with pan)
    g->set_font_size(10);
    double ly = world.bottom() + world.height() * 0.03;
    double lx = world.left() + world.width() * 0.05;
    double spacing = world.width() * 0.12;
    
    g->set_color(ezgl::BLACK);
    g->fill_arc({lx, ly}, node_radius, 0, 360);
    g->draw_text({lx + node_radius * 3, ly}, "Explored");
    
    lx += spacing;
    g->set_color(ezgl::RED);  // Pruned = red
    g->fill_arc({lx, ly}, node_radius, 0, 360);
    g->set_color(ezgl::BLACK);
    g->draw_text({lx + node_radius * 3, ly}, "Pruned");
    
    lx += spacing;
    g->set_color(ezgl::GREEN);
    g->fill_arc({lx, ly}, node_radius, 0, 360);
    g->set_color(ezgl::BLACK);
    g->draw_text({lx + node_radius * 3, ly}, "Solution");
    
    lx += spacing;
    std::stringstream info;
    info << "Optimal: " << g_optimal_cost << "  Nodes: " << g_nodes_visited;
    g->draw_text({lx, ly}, info.str());
}

void initial_setup(ezgl::application *app, bool /*new_window*/) {
    std::stringstream msg;
    msg << g_circuit_name << " | Depth: " << g_max_depth 
        << " | Tree nodes: " << g_tree_nodes.size();
    app->update_message(msg.str());
}

void run_graphics(const std::string& circuit_name, int optimal_cost, int nodes_visited) {
    g_circuit_name = circuit_name;
    g_optimal_cost = optimal_cost;
    g_nodes_visited = nodes_visited;
    
    std::cout << "\nLaunching graphics..." << std::endl;
    std::cout << "  Tree nodes recorded: " << g_tree_nodes.size() << std::endl;
    std::cout << "  Max depth: " << g_max_depth << std::endl;
    
    ezgl::application::settings settings;
    settings.main_ui_resource = "main.ui";
    settings.window_identifier = "MainWindow";
    settings.canvas_identifier = "MainCanvas";
    
    ezgl::application application(settings);
    
    // Fixed width, height scales with depth
    double width = 1000.0;
    double height = std::max(800.0, (g_max_depth + 2) * 20.0);
    
    std::cout << "  Canvas size: " << width << " x " << height << std::endl;
    
    ezgl::rectangle initial_world({0, 0}, width, height);
    application.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    
    application.run(initial_setup, nullptr, nullptr, nullptr);
}