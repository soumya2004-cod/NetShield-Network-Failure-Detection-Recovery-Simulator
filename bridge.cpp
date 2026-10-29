
#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <climits>
#include <algorithm>
using namespace std;

const int N = 6;

struct Edge
{
    int to, weight;
};

vector<Edge> graph[N];

int getRouter(string name)
{
    if (name.length() == 2 && name[0] == 'R' &&
        name[1] >= '1' && name[1] <= '6')
    {
        return name[1] - '1';
    }
    return -1;
}

string routerName(int r)
{
    return "R" + to_string(r + 1);
}

void addEdge(int u, int v, int w)
{
    graph[u].push_back({v, w});
    graph[v].push_back({u, w});
}

void buildGraph()
{
    for (int i = 0; i < N; i++)
        graph[i].clear();

    addEdge(0, 1, 2);
    addEdge(0, 2, 4);
    addEdge(1, 2, 2);
    addEdge(1, 3, 3);
    addEdge(2, 3, 1);
    addEdge(2, 4, 2);
    addEdge(3, 4, 2);
    addEdge(3, 5, 4);
    addEdge(4, 5, 1);
}

void printError(string message)
{
    cout << "{\"ok\":false,\"error\":\"" << message << "\"}";
}

int main(int argc, char *argv[])
{
    buildGraph();

    if (argc < 2)
    {
        printError("Missing command");
        return 0;
    }

    string command = argv[1];

    if (command == "status")
    {
        cout << "{\"ok\":true,\"message\":\"NetShield C++ Bridge Ready\"}";
        return 0;
    }

    if (command != "bfs" && command != "dijkstra")
    {
        printError("Unknown command");
        return 0;
    }

    if (argc < 4)
    {
        printError("Provide source and destination routers");
        return 0;
    }

    int source = getRouter(argv[2]);
    int destination = getRouter(argv[3]);

    if (source == -1 || destination == -1)
    {
        printError("Router names must be R1 to R6");
        return 0;
    }

    bool failed[N] = {false};

    for (int i = 4; i < argc; i++)
    {
        int r = getRouter(argv[i]);

        if (r == -1)
        {
            printError("Invalid failed router");
            return 0;
        }

        failed[r] = true;
    }

    if (failed[source] || (command == "dijkstra" && failed[destination]))
    {
        printError("Source or destination router has failed");
        return 0;
    }

    vector<int> path;
    int cost = 0;

    if (command == "bfs")
    {
        queue<int> q;
        bool visited[N] = {false};
        q.push(source);
        visited[source] = true;

        while (!q.empty())
        {
            int u = q.front();
            q.pop();
            path.push_back(u);

            for (const Edge &e : graph[u])
            {
                int v = e.to;
                if (!visited[v] && !failed[v])
                {
                    visited[v] = true;
                    q.push(v);
                }
            }
        }
    }
    else
    {
        vector<int> dist(N, INT_MAX);
        vector<int> parent(N, -1);
        priority_queue<
            pair<int, int>,
            vector<pair<int, int>>,
            greater<pair<int, int>>>
            pq;

        dist[source] = 0;
        pq.push({0, source});

        while (!pq.empty())
        {
            int d = pq.top().first;
            int u = pq.top().second;
            pq.pop();

            if (d != dist[u])
                continue;

            for (const Edge &e : graph[u])
            {
                int v = e.to;

                if (!failed[v] && d + e.weight < dist[v])
                {
                    dist[v] = d + e.weight;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        if (dist[destination] == INT_MAX)
        {
            printError("No available route");
            return 0;
        }

        cost = dist[destination];
        for (int v = destination; v != -1; v = parent[v])
            path.push_back(v);
        reverse(path.begin(), path.end());
    }

    cout << "{\"ok\":true,\"algorithm\":\"" << command
         << "\",\"path\":[";
    for (int i = 0; i < (int)path.size(); i++)
    {
        if (i > 0)
            cout << ",";
        cout << "\"" << routerName(path[i]) << "\"";
    }

    cout << "],\"cost\":" << cost << "}";
    return 0;
}
