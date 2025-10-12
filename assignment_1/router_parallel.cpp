// router_parallel.cpp - OpenMP Parallel Version
#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <tuple>
#include <algorithm>
#include <fstream>
#include <cstdint>
#include <utility>
#include <string>
#include <chrono>
#include <atomic>
#include <omp.h>
using namespace std;

// EZGL for GUI (only used if -i passed)
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

// ------------------------ Data types ------------------------
struct Pin
{
    int x, y, p;
};
struct Conn
{
    Pin src, sink;
};

enum class Arch
{
    DISTRIBUTED,
    TOP_BOTTOM
};

// ------------------------ Router ------------------------
struct Router
{
    int n = 0, W = 0;
    Arch arch = Arch::DISTRIBUTED;

    // Segment graph using 64-bit IDs
    unordered_map<uint64_t, vector<uint64_t>> adj;
    unordered_set<uint64_t> unavailable; // used segments (claimed by nets)
    unordered_map<uint64_t, int> segment_to_net; // map segment to net ID for coloring
    long long used_segments = 0;
    int current_net_id = 0; // for tracking which net we're routing

    // ID packing: [horiz:1][roc:16][span:16][track:rest]
    static inline uint64_t pack(bool horiz, int roc, int span, int t)
    {
        return (uint64_t(horiz ? 1 : 0)) | (uint64_t(roc) << 1) | (uint64_t(span) << 17) | (uint64_t(t) << 33);
    }
    static inline void unpack(uint64_t id, bool &horiz, int &roc, int &span, int &t)
    {
        horiz = (id & 1u);
        roc = int((id >> 1) & ((1u << 16) - 1));
        span = int((id >> 17) & ((1u << 16) - 1));
        t = int(id >> 33);
    }

    void build_graph()
    {
        adj.clear();
        unavailable.clear();
        used_segments = 0;

        // Horizontal unit segments along each row-line i (0..n), columns c (0..n-1)
        for (int i = 0; i <= n; ++i)
        {
            for (int c = 0; c < n; ++c)
            {
                for (int t = 0; t < W; ++t)
                {
                    uint64_t a = pack(true, i, c, t);
                    adj[a]; // materialize node
                }
            }
        }
        // Vertical unit segments along each col-line j (0..n), rows r (0..n-1)
        for (int j = 0; j <= n; ++j)
        {
            for (int r = 0; r < n; ++r)
            {
                for (int t = 0; t < W; ++t)
                {
                    uint64_t a = pack(false, j, r, t);
                    adj[a];
                }
            }
        }

        auto add = [&](uint64_t u, uint64_t v)
        { adj[u].push_back(v); adj[v].push_back(u); };

        // Planar (disjoint) switch block at each grid intersection (i,j)
        for (int i = 0; i <= n; ++i)
        {
            for (int j = 0; j <= n; ++j)
            {
                for (int t = 0; t < W; ++t)
                {
                    // Using int64_t to handle -1 properly
                    int64_t Hleft = (j > 0) ? (int64_t)pack(true, i, j - 1, t) : -1;
                    int64_t Hright = (j < n) ? (int64_t)pack(true, i, j, t) : -1;
                    int64_t Vup = (i > 0) ? (int64_t)pack(false, j, i - 1, t) : -1;
                    int64_t Vdown = (i < n) ? (int64_t)pack(false, j, i, t) : -1;

                    if (Hleft != -1 && Hright != -1)
                        add(Hleft, Hright);
                    if (Vup != -1 && Vdown != -1)
                        add(Vup, Vdown);
                    if (Hleft != -1 && Vup != -1)
                        add(Hleft, Vup);
                    if (Hleft != -1 && Vdown != -1)
                        add(Hleft, Vdown);
                    if (Hright != -1 && Vup != -1)
                        add(Hright, Vup);
                    if (Hright != -1 && Vdown != -1)
                        add(Hright, Vdown);
                }
            }
        }
    }

    // Map a pin to all adjacent track segments (Fc = W)
    vector<uint64_t> pin_to_adjacent_segments(int x, int y, int p) const
    {
        vector<uint64_t> out;
        
        if (arch == Arch::DISTRIBUTED)
        {
            // Pin 1: LEFT, Pin 2: RIGHT, Pin 3: TOP, Pin 4: BOTTOM (output)
            if (p == 1) // LEFT side
            {
                int j = x; // vertical channel on left of block
                if (j >= 0 && j <= n)
                {
                    for (int t = 0; t < W; ++t)
                        out.push_back(pack(false, j, y, t));
                }
            }
            else if (p == 2) // RIGHT side
            {
                int j = x + 1; // vertical channel on right of block
                if (j >= 0 && j <= n)
                {
                    for (int t = 0; t < W; ++t)
                        out.push_back(pack(false, j, y, t));
                }
            }
            else if (p == 3) // TOP side
            {
                int i = y + 1; // horizontal channel ABOVE the block
                if (i >= 0 && i <= n)
                {
                    for (int t = 0; t < W; ++t)
                        out.push_back(pack(true, i, x, t));
                }
            }
            else if (p == 4) // BOTTOM side (output)
            {
                int i = y; // horizontal channel BELOW the block
                if (i >= 0 && i <= n)
                {
                    for (int t = 0; t < W; ++t)
                        out.push_back(pack(true, i, x, t));
                }
            }
        }
        else // TOP_BOTTOM architecture
        {
            // Pins 1,2: TOP side, Pins 3,4: BOTTOM side
            if (p == 1 || p == 2) // TOP side
            {
                int i = y + 1; // horizontal channel ABOVE the block
                if (i >= 0 && i <= n)
                {
                    for (int t = 0; t < W; ++t)
                        out.push_back(pack(true, i, x, t));
                }
            }
            else // p == 3 or p == 4, BOTTOM side
            {
                int i = y; // horizontal channel BELOW the block
                if (i >= 0 && i <= n)
                {
                    for (int t = 0; t < W; ++t)
                        out.push_back(pack(true, i, x, t));
                }
            }
        }
        
        return out;
    }

    // Multi-source Lee–Moore BFS with OpenMP parallelization (optimized)
    vector<uint64_t> bfs_route(const vector<uint64_t> &src_seed,
                               const unordered_set<uint64_t> &tgt_adj)
    {
        // Create dense mapping: segment ID -> index (for bitset)
        vector<uint64_t> id_to_index;
        unordered_map<uint64_t, size_t> index_map;
        size_t idx = 0;
        for (auto &kv : adj) {
            index_map[kv.first] = idx++;
            id_to_index.push_back(kv.first);
        }
        
        // Use atomic bitset for visited (much faster than unordered_set)
        vector<atomic<bool>> visited(id_to_index.size());
        for (size_t i = 0; i < visited.size(); i++) {
            visited[i].store(false, memory_order_relaxed);
        }
        
        // Parent tracking
        unordered_map<uint64_t, uint64_t> parent;
        
        vector<uint64_t> current_wave = src_seed;
        
        // Initialize source nodes
        for (auto s : src_seed) {
            parent[s] = UINT64_MAX;
            visited[index_map[s]].store(true, memory_order_relaxed);
        }
        
        atomic<uint64_t> hit(UINT64_MAX);
        
        // BFS wave expansion
        while (!current_wave.empty() && hit.load(memory_order_relaxed) == UINT64_MAX) {
            vector<uint64_t> next_wave;
            size_t estimated_size = current_wave.size() * 6; // ~6 neighbors per node
            
            #pragma omp parallel
            {
                // Thread-local preallocated storage
                vector<pair<uint64_t, uint64_t>> my_discoveries;
                my_discoveries.reserve(estimated_size / omp_get_num_threads());
                
                #pragma omp for schedule(dynamic, 16) nowait
                for (size_t i = 0; i < current_wave.size(); i++) {
                    uint64_t u = current_wave[i];
                    
                    // Check if we found the target
                    if (tgt_adj.count(u)) {
                        uint64_t expected = UINT64_MAX;
                        hit.compare_exchange_strong(expected, u, memory_order_relaxed);
                        continue;
                    }
                    
                    // Explore neighbors
                    if (adj.count(u)) {
                        for (auto v : adj.at(u)) {
                            // Quick check: skip if unavailable
                            if (unavailable.count(v)) continue;
                            
                            size_t v_idx = index_map[v];
                            
                            // Atomic check-and-set: only one thread will succeed per node
                            bool expected_false = false;
                            if (visited[v_idx].compare_exchange_strong(expected_false, true, 
                                                                       memory_order_relaxed)) {
                                // ON BEHALF OF HUMANITY, We successfully claimed this node, LOL
                                my_discoveries.push_back({v, u});
                            }
                        }
                    }
                }
                
                // Merge thread-local discoveries (no deduplication needed - atomics handle it)
                #pragma omp critical(merge_wave)
                {
                    for (auto [node, par] : my_discoveries) {
                        parent[node] = par;
                        next_wave.push_back(node);
                    }
                }
            }
            
            if (hit.load(memory_order_relaxed) != UINT64_MAX) break;
            
            current_wave = move(next_wave);
        }
        
        uint64_t final_hit = hit.load(memory_order_relaxed);
        if (final_hit == UINT64_MAX)
            return {};
        
        // Backtrace to construct path
        vector<uint64_t> path;
        for (auto cur = final_hit; cur != UINT64_MAX; cur = parent[cur])
            path.push_back(cur);
        reverse(path.begin(), path.end());
        return path;
    }

    // Route one sink; seed includes existing tree for fanout
    bool route_one(const Pin &src, const Pin &sink, vector<uint64_t> &existing_tree)
    {
        vector<uint64_t> src_adj = pin_to_adjacent_segments(src.x, src.y, src.p);
        vector<uint64_t> tgt_vec = pin_to_adjacent_segments(sink.x, sink.y, sink.p);
        unordered_set<uint64_t> tgt_adj(tgt_vec.begin(), tgt_vec.end());

        vector<uint64_t> seed = src_adj;
        seed.insert(seed.end(), existing_tree.begin(), existing_tree.end());

        auto path = bfs_route(seed, tgt_adj);
        if (path.empty())
            return false;

        for (auto seg : path)
        {
            if (!unavailable.count(seg))
            {
                unavailable.insert(seg);
                ++used_segments;
            }
            segment_to_net[seg] = current_net_id;
            existing_tree.push_back(seg);
        }
        return true;
    }
};

// ------------------------ EZGL drawing ------------------------
static const Router *gR = nullptr;
static map<int, vector<Pin>> *gNetPins = nullptr;

static ezgl::rectangle get_initial_world(int n)
{
    double padding = 50.0;
    double world_size = n * 100.0;
    return ezgl::rectangle({-padding, -padding}, 
                          {world_size + padding, world_size + padding});
}

static double track_offset(int t, int W)
{
    if (W == 1) return 0.0;
    double spacing = 30.0 / (W - 1);
    return -15.0 + t * spacing;
}

static pair<ezgl::point2d, ezgl::point2d>
h_seg_endpoints(int row, int col, int track, int W)
{
    double y = row * 100.0 + track_offset(track, W);
    return {{col * 100.0, y}, {(col + 1) * 100.0, y}};
}

static pair<ezgl::point2d, ezgl::point2d>
v_seg_endpoints(int col, int row, int track, int W)
{
    double x = col * 100.0 + track_offset(track, W);
    return {{x, row * 100.0}, {x, (row + 1) * 100.0}};
}

static void draw_main_canvas(ezgl::renderer *g)
{
    if (!gR)
        return;
    int n = gR->n;
    int W = gR->W;

    vector<tuple<int, int, int>> colors = {
        {50, 120, 255}, {255, 80, 80}, {80, 200, 120}, {255, 165, 0},
        {148, 0, 211}, {255, 192, 203}, {0, 206, 209}, {255, 215, 0}
    };

    // Draw logic blocks
    g->set_color(240, 240, 240);
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            double cx = j * 100.0 + 50.0;
            double cy = i * 100.0 + 50.0;
            double half = 30.0;
            g->fill_rectangle({cx - half, cy - half}, {cx + half, cy + half});
        }
    }

    g->set_line_width(2);
    g->set_color(100, 100, 100);
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            double cx = j * 100.0 + 50.0;
            double cy = i * 100.0 + 50.0;
            double half = 30.0;
            g->draw_rectangle({cx - half, cy - half}, {cx + half, cy + half});
        }
    }

    // Draw available segments
    g->set_line_width(1);
    g->set_color(230, 230, 230);
    for (auto &kv : gR->adj)
    {
        uint64_t seg = kv.first;
        if (gR->unavailable.count(seg))
            continue;
        bool H;
        int roc, span, t;
        Router::unpack(seg, H, roc, span, t);
        auto ends = H ? h_seg_endpoints(roc, span, t, W)
                      : v_seg_endpoints(roc, span, t, W);
        g->draw_line(ends.first, ends.second);
    }

    // Draw routed segments grouped by net
    g->set_line_width(2);
    map<int, vector<uint64_t>> nets_segments;
    for (auto &kv : gR->segment_to_net)
    {
        nets_segments[kv.second].push_back(kv.first);
    }
    
    for (auto &[net_id, segments] : nets_segments)
    {
        auto [r, g_val, b] = colors[net_id % colors.size()];
        g->set_color(r, g_val, b);
        
        for (uint64_t seg : segments)
        {
            bool H;
            int roc, span, t;
            Router::unpack(seg, H, roc, span, t);
            auto ends = H ? h_seg_endpoints(roc, span, t, W)
                          : v_seg_endpoints(roc, span, t, W);
            g->draw_line(ends.first, ends.second);
        }
    }

    // Draw block coordinates
    g->set_color(80, 80, 80);
    g->format_font("monospace", ezgl::font_slant::normal, ezgl::font_weight::normal, 9);
    for (int i = 0; i < n; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            double cx = j * 100.0 + 50.0;
            double cy = i * 100.0 + 50.0;
            string label = "(" + to_string(j) + "," + to_string(i) + ")";
            g->draw_text({cx, cy}, label, 60.0, 20.0);
        }
    }

    // Draw pins with matching net colors
    if (gNetPins)
    {
        for (auto &[net_id, pins] : *gNetPins)
        {
            auto [r, g_val, b] = colors[net_id % colors.size()];
            
            for (const Pin &pin : pins)
            {
                double cx = pin.x * 100.0 + 50.0;
                double cy = pin.y * 100.0 + 50.0;
                double half = 30.0;
                
                g->set_color(r, g_val, b);
                
                if (gR->arch == Arch::DISTRIBUTED)
                {
                    if (pin.p == 1)
                        g->fill_rectangle({cx - half - 4, cy - 4}, {cx - half + 4, cy + 4});
                    else if (pin.p == 2)
                        g->fill_rectangle({cx + half - 4, cy - 4}, {cx + half + 4, cy + 4});
                    else if (pin.p == 3)
                        g->fill_rectangle({cx - 4, cy + half - 4}, {cx + 4, cy + half + 4});
                    else if (pin.p == 4)
                        g->fill_rectangle({cx - 4, cy - half - 4}, {cx + 4, cy - half + 4});
                }
                else
                {
                    if (pin.p == 1)
                        g->fill_rectangle({cx - 10 - 4, cy + half - 4}, {cx - 10 + 4, cy + half + 4});
                    else if (pin.p == 2)
                        g->fill_rectangle({cx + 10 - 4, cy + half - 4}, {cx + 10 + 4, cy + half + 4});
                    else if (pin.p == 3)
                        g->fill_rectangle({cx - 10 - 4, cy - half - 4}, {cx - 10 + 4, cy - half + 4});
                    else if (pin.p == 4)
                        g->fill_rectangle({cx + 10 - 4, cy - half - 4}, {cx + 10 + 4, cy - half + 4});
                }
            }
        }
    }
}

static void run_gui(const Router &R, const map<int, vector<Pin>> &netPins)
{
    gR = &R;
    gNetPins = const_cast<map<int, vector<Pin>>*>(&netPins);

    ezgl::application::settings s;
    s.main_ui_resource = "main.ui";
    s.window_identifier = "MainWindow";
    s.canvas_identifier = "MainCanvas";

    ezgl::application app(s);
    ezgl::rectangle initial_world = get_initial_world(R.n);
    app.add_canvas("MainCanvas", draw_main_canvas, initial_world);
    app.run(nullptr, nullptr, nullptr, nullptr);
}

// ------------------------ CLI / main ------------------------
struct Config
{
    string circuit_file;
    bool gui = false;
    Arch arch = Arch::DISTRIBUTED;
    int force_W = -1;
    int num_threads = 4; // OpenMP thread count
};

static Config parse_args(int argc, char **argv)
{
    Config cfg;
    for (int i = 1; i < argc; ++i)
    {
        string a = argv[i];
        if (a == "-h" || a == "--help")
        {
            cerr << "Usage: router_parallel -f <file> [-i] [-a distributed|topbottom] [-w N] [-t threads]\n";
            exit(0);
        }
        else if (a == "-f" && i + 1 < argc)
        {
            cfg.circuit_file = argv[++i];
        }
        else if (a == "-i")
        {
            cfg.gui = true;
        }
        else if (a == "-a" && i + 1 < argc)
        {
            string v = argv[++i];
            if (v == "distributed")
                cfg.arch = Arch::DISTRIBUTED;
            else if (v == "topbottom" || v == "top_bottom" || v == "tb")
                cfg.arch = Arch::TOP_BOTTOM;
            else
            {
                cerr << "Unknown architecture: " << v << "\n";
                exit(1);
            }
        }
        else if (a == "-w" && i + 1 < argc)
        {
            cfg.force_W = stoi(argv[++i]);
        }
        else if (a == "-t" && i + 1 < argc)
        {
            cfg.num_threads = stoi(argv[++i]);
        }
        else
        {
            cerr << "Unknown arg: " << a << "\n";
            exit(1);
        }
    }
    if (cfg.circuit_file.empty())
    {
        cerr << "Error: -f <file> is required.\n";
        exit(1);
    }
    return cfg;
}

int main(int argc, char **argv)
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Config cfg = parse_args(argc, argv);
    
    // Set OpenMP thread count
    omp_set_num_threads(cfg.num_threads);

    ifstream fin(cfg.circuit_file);
    if (!fin)
    {
        cerr << "Cannot open " << cfg.circuit_file << "\n";
        return 2;
    }

    int n, W;
    if (!(fin >> n >> W))
    {
        cerr << "Bad header in circuit file.\n";
        return 3;
    }
    if (cfg.force_W > 0)
        W = cfg.force_W;

    vector<Conn> conns;
    while (true)
    {
        int X1, Y1, P1, X2, Y2, P2;
        if (!(fin >> X1 >> Y1 >> P1 >> X2 >> Y2 >> P2))
            break;
        if (X1 == -1 && Y1 == -1 && P1 == -1 && X2 == -1 && Y2 == -1 && P2 == -1)
            break;
        conns.push_back({{X1, Y1, P1}, {X2, Y2, P2}});
    }

    Router R;
    R.n = n;
    R.W = W;
    R.arch = cfg.arch;
    R.build_graph();

    map<tuple<int, int, int>, vector<Pin>> fanouts;
    for (auto &c : conns)
        fanouts[{c.src.x, c.src.y, c.src.p}].push_back(c.sink);

    map<int, vector<Pin>> netPins;
    
    auto start_time = chrono::high_resolution_clock::now();
    
    bool ok = true;
    int net_id = 0;
    for (auto &kv : fanouts)
    {
        vector<uint64_t> tree;
        Pin source{get<0>(kv.first), get<1>(kv.first), get<2>(kv.first)};
        
        R.current_net_id = net_id;
        netPins[net_id].push_back(source);
        
        for (const Pin &sink : kv.second)
        {
            netPins[net_id].push_back(sink);
            
            if (!R.route_one(source, sink, tree))
            {
                ok = false;
                break;
            }
        }
        if (!ok)
            break;
        
        net_id++;
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
    double runtime_ms = duration.count() / 1000.0;

    cout << (cfg.arch == Arch::DISTRIBUTED ? "Architecture: distributed\n"
                                           : "Architecture: top/bottom\n");
    cout << "Threads: " << cfg.num_threads << "\n";
    if (ok)
    {
        cout << "Routed successfully.\n";
        cout << "Used segments = " << R.used_segments << "\n";
        cout << "Routing time: " << runtime_ms << " ms\n";
        if (cfg.gui)
            run_gui(R, netPins);
    }
    else
    {
        cout << "Routing failed with W=" << W << "\n";
    }
    return 0;
}