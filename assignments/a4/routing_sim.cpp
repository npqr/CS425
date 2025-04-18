#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

const int INF = 9999;

void printDVRTable(int node, const vector<vector<int>>& table, const vector<vector<int>>& nextHop) {
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < table.size(); ++i) {
        cout << i << "\t" << table[node][i] << "\t";
        if (i == node || nextHop[node][i] == -1) cout << "-";
        else cout << nextHop[node][i];
        cout << endl;
    }
    cout << endl;
}

void simulateDVR(const vector<vector<int>>& graph) {
    int n = graph.size();
    vector<vector<int>> dist = graph;
    vector<vector<int>> nextHop(n, vector<int>(n, -1));

    // Initialization of next hops
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && graph[i][j] != INF) {
                nextHop[i][j] = j;
            }
        }
    }

    // Print initial tables
    cout << "--- DVR Iteration 0 ---\n";
    for (int i = 0; i < n; ++i) printDVRTable(i, dist, nextHop);

    // Iterative Bellman-Ford updates
    bool updated = true;
    int iteration = 1;
    while (updated) {
        updated = false;
        for (int u = 0; u < n; ++u) {
            for (int v = 0; v < n; ++v) {
                if (graph[u][v] == INF || u == v) continue;
                for (int w = 0; w < n; ++w) {
                    if (dist[v][w] == INF) continue;
                    int newCost = graph[u][v] + dist[v][w];
                    if (newCost < dist[u][w]) {
                        dist[u][w] = newCost;
                        nextHop[u][w] = nextHop[u][v];
                        updated = true;
                    }
                }
            }
        }
        if (updated) {
            cout << "--- DVR Iteration " << iteration << " ---\n";
            for (int i = 0; i < n; ++i) printDVRTable(i, dist, nextHop);
            ++iteration;
        }
    }

    cout << "--- DVR Final Tables ---\n";
    for (int i = 0; i < n; ++i) printDVRTable(i, dist, nextHop);
}

void printLSRTable(int src, const vector<int>& dist, const vector<int>& prev) {
    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < dist.size(); ++i) {
        if (i == src) continue;
        cout << i << "\t" << (dist[i] == INF ? INF : dist[i]) << "\t";
        int hop = i;
        // backtrack to find next hop
        while (prev[hop] != src && prev[hop] != -1) {
            hop = prev[hop];
        }
        if (prev[hop] == -1) cout << "-";
        else cout << hop;
        cout << endl;
    }
    cout << endl;
}

void simulateLSR(const vector<vector<int>>& graph) {
    int n = graph.size();
    for (int src = 0; src < n; ++src) {
        vector<int> dist(n, INF);
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);
        dist[src] = 0;

        // Min-heap priority queue: (distance, node)
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
        pq.push(make_pair(0, src));

        while (!pq.empty()) {
            pair<int,int> top = pq.top(); pq.pop();
            int d = top.first;
            int u = top.second;
            if (visited[u]) continue;
            visited[u] = true;

            for (int v = 0; v < n; ++v) {
                if (graph[u][v] != INF && !visited[v]) {
                    int cost = graph[u][v];
                    if (dist[u] + cost < dist[v]) {
                        dist[v] = dist[u] + cost;
                        prev[v] = u;
                        pq.push(make_pair(dist[v], v));
                    }
                }
            }
        }

        printLSRTable(src, dist, prev);
    }
}

vector<vector<int>> readGraphFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }
    
    int n;
    file >> n;
    vector<vector<int>> graph(n, vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> graph[i][j];

    file.close();
    return graph;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    string filename = argv[1];
    vector<vector<int>> graph = readGraphFromFile(filename);

    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return 0;
}
