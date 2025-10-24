// router.cpp — single-thread Lee–Moore with EZGL
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
using namespace std;

#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

// ------------------- structs --------------------
struct Pin { int x, y, p; };
struct Conn { Pin src, sink; };

enum class Arch { DISTRIBUTED, TOP_BOTTOM };

// ----------------------- router -----------------------
struct Router {
  int n = 0, W = 0;
  Arch arch = Arch::DISTRIBUTED;

  unordered_map<uint64_t, vector<uint64_t>> adj;   // segment graph
  unordered_set<uint64_t> unavailable;             // claimed segments (by nets)
  unordered_map<uint64_t, int> segment_to_net;     // for GUI coloring
  long long used_segments = 0;
  int current_net_id = 0;

  // ID packing: [horiz:1][roc:16][span:16][track:rest]
  static inline uint64_t pack(bool horiz, int roc, int span, int t) {
    return (uint64_t(horiz ? 1 : 0))
         | (uint64_t(roc)  << 1)
         | (uint64_t(span) << 17)
         | (uint64_t(t)    << 33);
  }
  static inline void unpack(uint64_t id, bool &horiz, int &roc, int &span, int &t) {
    horiz = (id & 1u);
    roc   = int((id >> 1)  & ((1u << 16) - 1));
    span  = int((id >> 17) & ((1u << 16) - 1));
    t     = int(id >> 33);
  }

  void build_graph() {
    adj.clear(); unavailable.clear(); used_segments = 0;

    // materialize nodes
    for (int i = 0; i <= n; ++i)
      for (int c = 0; c < n; ++c)
        for (int t = 0; t < W; ++t)
          adj[pack(true, i, c, t)];
    for (int j = 0; j <= n; ++j)
      for (int r = 0; r < n; ++r)
        for (int t = 0; t < W; ++t)
          adj[pack(false, j, r, t)];

    auto add = [&](uint64_t u, uint64_t v){ adj[u].push_back(v); adj[v].push_back(u); };

    // Planar (disjoint) switch block wiring, bounds-checked (no sentinels)
    for (int i = 0; i <= n; ++i) {
      for (int j = 0; j <= n; ++j) {
        for (int t = 0; t < W; ++t) {
          if (j > 0 && j < n) {
            auto hl = pack(true,  i, j-1, t);
            auto hr = pack(true,  i, j,   t);
            add(hl, hr); // straight H
          }
          if (i > 0 && i < n) {
            auto vu = pack(false, j, i-1, t);
            auto vd = pack(false, j, i,   t);
            add(vu, vd); // straight V
          }
          if (j > 0 && i > 0) add(pack(true,  i, j-1, t), pack(false, j, i-1, t)); // Hleft ↔ Vup
          if (j > 0 && i < n) add(pack(true,  i, j-1, t), pack(false, j, i,   t)); // Hleft ↔ Vdown
          if (j < n && i > 0) add(pack(true,  i, j,   t), pack(false, j, i-1, t)); // Hright ↔ Vup
          if (j < n && i < n) add(pack(true,  i, j,   t), pack(false, j, i,   t)); // Hright ↔ Vdown
        }
      }
    }
  }

  // Map a pin to all adjacent track segments (Fc=W).
  // For block at (x,y):
  //   Horizontal channels lie on row lines i = 0..n. The block’s TOP is i=y, BOTTOM is i=y+1.
  //   Vertical   channels lie on col lines j = 0..n. The block’s LEFT is j=x, RIGHT is j=x+1.
  vector<uint64_t> pin_to_adjacent_segments(int x, int y, int p) const {
    vector<uint64_t> out;

    if (arch == Arch::DISTRIBUTED) {
      // Sides per your chosen convention:
      // 1=LEFT, 2=RIGHT, 3=TOP, 4=BOTTOM
      if (p == 1) { // LEFT  -> j = x
        int j = x; if (0 <= j && j <= n) for (int t=0;t<W;++t) out.push_back(pack(false, j, y, t));
      } else if (p == 2) { // RIGHT -> j = x+1
        int j = x+1; if (0 <= j && j <= n) for (int t=0;t<W;++t) out.push_back(pack(false, j, y, t));
      } else if (p == 3) { // TOP   -> i = y
        int i = y; if (0 <= i && i <= n) for (int t=0;t<W;++t) out.push_back(pack(true,  i, x, t));
      } else {             // BOTTOM-> i = y+1 (p==4)
        int i = y+1; if (0 <= i && i <= n) for (int t=0;t<W;++t) out.push_back(pack(true,  i, x, t));
      }
    } else {
      // TOP/BOTTOM: 1,2 on TOP; 3,4 on BOTTOM
      if (p == 1 || p == 2) { // TOP   -> i = y
        int i = y; if (0 <= i && i <= n) for (int t=0;t<W;++t) out.push_back(pack(true,  i, x, t));
      } else {                // BOTTOM-> i = y+1
        int i = y+1; if (0 <= i && i <= n) for (int t=0;t<W;++t) out.push_back(pack(true,  i, x, t));
      }
    }
    return out;
  }

  // Multi-source Lee–Moore BFS, with whitelist so fanout can grow from its own tree.
  vector<uint64_t> bfs_route(const vector<uint64_t> &src_seed,
                             const unordered_set<uint64_t> &tgt_adj,
                             const unordered_set<uint64_t> &whitelist) {
    queue<uint64_t> q;
    unordered_map<uint64_t, uint64_t> parent;
    unordered_set<uint64_t> visited;

    auto can_use = [&](uint64_t u){ return whitelist.count(u) || !unavailable.count(u); };
    auto push = [&](uint64_t u){
      if (!visited.count(u) && can_use(u)) { visited.insert(u); q.push(u); }
    };

    for (auto s : src_seed) { parent[s] = UINT64_MAX; push(s); }

    uint64_t hit = UINT64_MAX;
    while (!q.empty()) {
      auto u = q.front(); q.pop();
      if (tgt_adj.count(u)) { hit = u; break; }
      for (auto v : adj.at(u)) {
        if (!visited.count(v) && can_use(v)) { visited.insert(v); parent[v] = u; q.push(v); }
      }
    }
    if (hit == UINT64_MAX) return {};
    vector<uint64_t> path;
    for (auto cur = hit; cur != UINT64_MAX; cur = parent[cur]) path.push_back(cur);
    reverse(path.begin(), path.end());
    return path;
  }

  // Route one sink; seed includes existing tree for fanout
  bool route_one(const Pin &src, const Pin &sink, vector<uint64_t> &existing_tree) {
    vector<uint64_t> src_adj = pin_to_adjacent_segments(src.x, src.y, src.p);
    vector<uint64_t> tgt_vec = pin_to_adjacent_segments(sink.x, sink.y, sink.p);
    unordered_set<uint64_t> tgt_adj(tgt_vec.begin(), tgt_vec.end());

    vector<uint64_t> seed = src_adj;
    seed.insert(seed.end(), existing_tree.begin(), existing_tree.end());

    unordered_set<uint64_t> wl(existing_tree.begin(), existing_tree.end());
    auto path = bfs_route(seed, tgt_adj, wl);
    if (path.empty()) return false;

    for (auto seg : path) {
      if (!unavailable.count(seg)) { unavailable.insert(seg); ++used_segments; }
      segment_to_net[seg] = current_net_id;
      existing_tree.push_back(seg);
    }
    return true;
  }
};

// ------------------------ EZGL drawing ------------------------
static const Router *gR = nullptr;
static map<int, vector<Pin>> *gNetPins = nullptr;

static ezgl::rectangle get_initial_world(int n) {
  double padding = 50.0;
  double world_size = n * 100.0;
  return ezgl::rectangle({-padding, -padding}, {world_size + padding, world_size + padding});
}
static double track_offset(int t, int W) {
  if (W == 1) return 0.0;
  double spacing = 30.0 / (W - 1);
  return -15.0 + t * spacing;
}
static pair<ezgl::point2d, ezgl::point2d> h_seg_endpoints(int row, int col, int track, int W) {
  double y = row * 100.0 + track_offset(track, W);
  return {{col * 100.0, y}, {(col + 1) * 100.0, y}};
}
static pair<ezgl::point2d, ezgl::point2d> v_seg_endpoints(int col, int row, int track, int W) {
  double x = col * 100.0 + track_offset(track, W);
  return {{x, row * 100.0}, {x, (row + 1) * 100.0}};
}

static void draw_main_canvas(ezgl::renderer *g) {
  if (!gR) return;
  int n = gR->n, W = gR->W;

  vector<tuple<int,int,int>> colors = {
    {50,120,255},{255,80,80},{80,200,120},{255,165,0},
    {148,0,211},{255,192,203},{0,206,209},{255,215,0}
  };

  // blocks
  g->set_color(240,240,240);
  for (int i=0;i<n;++i) for (int j=0;j<n;++j) {
    double cx=j*100.0+50.0, cy=i*100.0+50.0, half=30.0;
    g->fill_rectangle({cx-half,cy-half},{cx+half,cy+half});
  }
  g->set_line_width(2); g->set_color(100,100,100);
  for (int i=0;i<n;++i) for (int j=0;j<n;++j) {
    double cx=j*100.0+50.0, cy=i*100.0+50.0, half=30.0;
    g->draw_rectangle({cx-half,cy-half},{cx+half,cy+half});
  }

  // available segments light
  g->set_line_width(1); g->set_color(230,230,230);
  for (auto &kv : gR->adj) {
    uint64_t seg = kv.first;
    if (gR->unavailable.count(seg)) continue;
    bool H; int roc, span, t; Router::unpack(seg, H, roc, span, t);
    auto ends = H ? h_seg_endpoints(roc, span, t, W) : v_seg_endpoints(roc, span, t, W);
    g->draw_line(ends.first, ends.second);
  }

  // used segments per net
  g->set_line_width(2);
  map<int, vector<uint64_t>> nets_segments;
  for (auto &kv : gR->segment_to_net) nets_segments[kv.second].push_back(kv.first);
  for (auto it = nets_segments.begin(); it != nets_segments.end(); ++it) {
    int net_id = it->first;
    auto &segs = it->second;
    auto color = colors[net_id % colors.size()];
    int r = get<0>(color);
    int gc = get<1>(color);
    int b = get<2>(color);
    g->set_color(r,gc,b);
    for (auto seg : segs) {
      bool H; int roc, span, t; Router::unpack(seg, H, roc, span, t);
      auto ends = H ? h_seg_endpoints(roc, span, t, W) : v_seg_endpoints(roc, span, t, W);
      g->draw_line(ends.first, ends.second);
    }
  }

  // coords
  g->set_color(80,80,80);
  g->format_font("monospace", ezgl::font_slant::normal, ezgl::font_weight::normal, 9);
  for (int i=0;i<n;++i) for (int j=0;j<n;++j) {
    double cx=j*100.0+50.0, cy=i*100.0+50.0;
    g->draw_text({cx,cy}, "("+to_string(j)+","+to_string(i)+")", 60.0, 20.0);
  }

  // pins
  if (gNetPins) {
    for (map<int, vector<Pin>>::const_iterator it = gNetPins->begin(); it != gNetPins->end(); ++it) {
      int net_id = it->first;
      const vector<Pin> &pins = it->second;
      auto color = colors[net_id % colors.size()];
      int r = get<0>(color);
      int gc = get<1>(color);
      int b = get<2>(color);
      for (const Pin &pin : pins) {
        double cx = pin.x*100.0+50.0, cy = pin.y*100.0+50.0, half=30.0;
        g->set_color(r,gc,b);
        if (gR->arch == Arch::DISTRIBUTED) {
          if (pin.p==1) g->fill_rectangle({cx-half-4, cy-4}, {cx-half+4, cy+4});
          else if (pin.p==2) g->fill_rectangle({cx+half-4, cy-4}, {cx+half+4, cy+4});
          else if (pin.p==3) g->fill_rectangle({cx-4, cy+half-4}, {cx+4, cy+half+4});
          else               g->fill_rectangle({cx-4, cy-half-4}, {cx+4, cy-half+4});
        } else {
          if (pin.p==1) g->fill_rectangle({cx-10-4, cy+half-4}, {cx-10+4, cy+half+4});
          else if (pin.p==2) g->fill_rectangle({cx+10-4, cy+half-4}, {cx+10+4, cy+half+4});
          else if (pin.p==3) g->fill_rectangle({cx-10-4, cy-half-4}, {cx-10+4, cy-half+4});
          else               g->fill_rectangle({cx+10-4, cy-half-4}, {cx+10+4, cy-half+4});
        }
      }
    }
  }
}

static void run_gui(const Router &R, const map<int, vector<Pin>> &netPins) {
  static const Router* gR_local=nullptr; gR_local=&R; gR=gR_local;
  static map<int, vector<Pin>> const* np_local=nullptr; np_local=&netPins; gNetPins = const_cast<map<int, vector<Pin>>*>(np_local);

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
struct Config {
  string circuit_file;
  bool gui = false;
  Arch arch = Arch::DISTRIBUTED;
  int force_W = -1;
};

static Config parse_args(int argc, char **argv) {
  Config cfg;
  for (int i=1;i<argc;++i) {
    string a = argv[i];
    if (a=="-h"||a=="--help"){ cerr<<"Usage: router -f <file> [-i] [-a distributed|topbottom] [-w N]\n"; exit(0); }
    else if (a=="-f" && i+1<argc) cfg.circuit_file = argv[++i];
    else if (a=="-i") cfg.gui = true;
    else if (a=="-a" && i+1<argc) {
      string v = argv[++i];
      if (v=="distributed") cfg.arch = Arch::DISTRIBUTED;
      else if (v=="topbottom" || v=="top_bottom" || v=="tb") cfg.arch = Arch::TOP_BOTTOM;
      else { cerr<<"Unknown architecture: "<<v<<"\n"; exit(1); }
    } else if (a=="-w" && i+1<argc) cfg.force_W = stoi(argv[++i]);
    else { cerr<<"Unknown arg: "<<a<<"\n"; exit(1); }
  }
  if (cfg.circuit_file.empty()) { cerr<<"Error: -f <file> is required.\n"; exit(1); }
  return cfg;
}

static inline pair<bool, Conn> normalize_conn(int X1,int Y1,int P1,int X2,int Y2,int P2){
  if (P1<1||P1>4||P2<1||P2>4) return make_pair(false, Conn{});
  bool a_is_drv=(P1==4), b_is_drv=(P2==4);
  if (a_is_drv==b_is_drv) return make_pair(false, Conn{}); // both drivers or both sinks -> invalid
  if (a_is_drv) return make_pair(true, Conn{Pin{X1,Y1,P1}, Pin{X2,Y2,P2}});
  else          return make_pair(true, Conn{Pin{X2,Y2,P2}, Pin{X1,Y1,P1}});
}

int main(int argc, char **argv) {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  Config cfg = parse_args(argc, argv);

  ifstream fin(cfg.circuit_file);
  if (!fin){ cerr<<"Cannot open "<<cfg.circuit_file<<"\n"; return 2; }

  int n, W;
  if (!(fin>>n>>W)){ cerr<<"Bad header in circuit file.\n"; return 3; }
  if (cfg.force_W > 0) W = cfg.force_W;

  vector<Conn> conns;
  while (true){
    int X1,Y1,P1,X2,Y2,P2;
    if(!(fin>>X1>>Y1>>P1>>X2>>Y2>>P2)) break;
    if(X1==-1&&Y1==-1&&P1==-1&&X2==-1&&Y2==-1&&P2==-1) break;
    {
      auto nc = normalize_conn(X1,Y1,P1,X2,Y2,P2);
      if (nc.first) conns.push_back(nc.second);
      else cerr<<"Skipping invalid connection.\n";
    }
  }

  Router R; R.n=n; R.W=W; R.arch=cfg.arch; R.build_graph();

  // group by driver pin (x,y,p=4)
  map<tuple<int,int,int>, vector<Pin>> fanouts;
  for (auto &c : conns) fanouts[{c.src.x,c.src.y,c.src.p}].push_back(c.sink);

  map<int, vector<Pin>> netPins;

  auto start = chrono::high_resolution_clock::now();

  bool ok = true; int net_id = 0;
  for (auto &kv : fanouts) {
    vector<uint64_t> tree;
    Pin source{get<0>(kv.first), get<1>(kv.first), get<2>(kv.first)};
    R.current_net_id = net_id;
    netPins[net_id].push_back(source);
    for (const Pin &sink : kv.second) {
      netPins[net_id].push_back(sink);
      if (!R.route_one(source, sink, tree)) { ok=false; break; }
    }
    if (!ok) break;
    ++net_id;
  }

  auto end = chrono::high_resolution_clock::now();
  double ms = chrono::duration_cast<chrono::microseconds>(end-start).count()/1000.0;

  cout << (cfg.arch==Arch::DISTRIBUTED ? "Architecture: distributed\n" : "Architecture: top/bottom\n");
  if (ok) {
    cout << "Routed successfully.\n";
    cout << "Used segments = " << R.used_segments << "\n";
    cout << "Routing time: " << ms << " ms\n";
    if (cfg.gui) run_gui(R, netPins);   // during debug, you can call run_gui regardless of ok
  } else {
    cout << "Routing failed with W=" << W << "\n";
    // if you want to see pins/grid even on failure, uncomment:
    // if (cfg.gui) run_gui(R, netPins);
  }
  return 0;
}
