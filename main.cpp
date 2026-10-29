
#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <algorithm>
using namespace std;

const int N = 6;

struct Edge
{
    int to, weight;
};

vector<vector<Edge>> graph(N);
bool failed[N] = {};

void addEdge(int u, int v, int w)
{
    graph[u].push_back({v, w});
    graph[v].push_back({u, w});
}

void showNetwork()
{
    cout << "\n--- Network Status ---\n";
    for (int i = 0; i < N; i++)
    {
        cout << "R" << i + 1 << ": "
             << (failed[i] ? "FAILED" : "ACTIVE") << '\n';
    }
}

vector<int> bfs(int s, int d)
{
    vector<int> parent(N, -1);
    vector<bool> visited(N, false);
    queue<int> q;

    if (failed[s] || failed[d])
        return {};

    visited[s] = true;
    q.push(s);

    while (!q.empty())
    {
        int u = q.front();
        q.pop();

        for (auto e : graph[u])
        {
            int v = e.to;
            if (!visited[v] && !failed[v])
            {
                visited[v] = true;
                parent[v] = u;
                q.push(v);
            }
        }
    }

    if (!visited[d])
        return {};

    vector<int> path;
    for (int v = d; v != -1; v = parent[v])
        path.push_back(v);

    reverse(path.begin(), path.end());
    return path;
}

bool dijkstra(int s, int d, vector<int> &path, int &cost)
{
    vector<int> dist(N, INT_MAX), parent(N, -1);
    priority_queue<pair<int, int>,
                   vector<pair<int, int>>,
                   greater<pair<int, int>>>
        pq;

    if (failed[s] || failed[d])
        return false;

    dist[s] = 0;
    pq.push({0, s});

    while (!pq.empty())
    {
        int c = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (c != dist[u])
            continue;

        for (auto e : graph[u])
        {
            int v = e.to;
            if (failed[v])
                continue;

            if (dist[u] + e.weight < dist[v])
            {
                dist[v] = dist[u] + e.weight;
                parent[v] = u;
                pq.push({dist[v], v});
            }
        }
    }

    if (dist[d] == INT_MAX)
        return false;

    path.clear();
    for (int v = d; v != -1; v = parent[v])
        path.push_back(v);

    reverse(path.begin(), path.end());
    cost = dist[d];
    return true;
}

void printPath(vector<int> path)
{
    if (path.empty())
    {
        cout << "No route available.\n";
        return;
    }

    for (int i = 0; i < (int)path.size(); i++)
    {
        cout << "R" << path[i] + 1;
        if (i + 1 < (int)path.size())
            cout << " -> ";
    }
    cout << '\n';
}

bool getRouters(int &s, int &d)
{
    cout << "Enter source router (1-6): ";
    cin >> s;
    cout << "Enter destination router (1-6): ";
    cin >> d;

    if (!cin || s < 1 || s > N || d < 1 || d > N)
    {
        cout << "Invalid input!\n";
        cin.clear();
        cin.ignore(10000, '\n');
        return false;
    }

    --s;
    --d;
    return true;
}

void findBFS()
{
    int s, d;
    if (!getRouters(s, d))
        return;

    cout << "BFS path: ";
    printPath(bfs(s, d));
}

void findDijkstra()
{
    int s, d, cost;
    vector<int> path;
    if (!getRouters(s, d))
        return;

    if (dijkstra(s, d, path, cost))
    {
        cout << "Minimum-cost route: ";
        printPath(path);
        cout << "Total cost: " << cost << '\n';
    }
    else
    {
        cout << "No route available.\n";
    }
}

void simulateFailure()
{
    int r, s, d;
    cout << "Enter router to fail (1-6): ";
    cin >> r;

    if (!cin || r < 1 || r > N)
    {
        cout << "Invalid router!\n";
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }

    failed[r - 1] = true;
    cout << "Router R" << r << " has FAILED!\n";

    if (!getRouters(s, d))
        return;

    cout << "Recovery route: ";
    vector<int> path = bfs(s, d);
    printPath(path);

    if (!path.empty())
        cout << "Recovery successful!\n";
    else
        cout << "Recovery failed!\n";

    vector<int> best;
    int cost;

    if (dijkstra(s, d, best, cost))
    {
        cout << "Minimum-cost recovery route: ";
        printPath(best);
        cout << "Total cost: " << cost << '\n';
    }
}

void restoreRouter()
{
    int r;
    cout << "Enter router to restore (1-6): ";
    cin >> r;

    if (!cin || r < 1 || r > N)
    {
        cout << "Invalid router!\n";
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }

    failed[r - 1] = false;
    cout << "R" << r << " restored successfully!\n";
}

int main()
{
    addEdge(0, 1, 2);
    addEdge(0, 2, 4);
    addEdge(1, 2, 2);
    addEdge(1, 3, 3);
    addEdge(2, 3, 1);
    addEdge(2, 4, 2);
    addEdge(3, 4, 2);
    addEdge(3, 5, 4);
    addEdge(4, 5, 1);

    int choice;

    do
    {
        cout << "\n===== NetShield =====\n";
        cout << "1. Display network status\n";
        cout << "2. Find route using BFS\n";
        cout << "3. Find minimum-cost route using Dijkstra\n";
        cout << "4. Simulate router failure and recovery\n";
        cout << "5. Restore a failed router\n";
        cout << "6. Reset all routers\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";

        cin >> choice;

        switch (choice)
        {
        case 1:
            showNetwork();
            break;
        case 2:
            findBFS();
            break;
        case 3:
            findDijkstra();
            break;
        case 4:
            simulateFailure();
            break;
        case 5:
            restoreRouter();
            break;
        case 6:
            fill(failed, failed + N, false);
            cout << "All routers restored!\n";
            break;
        case 0:
            cout << "Thank you for using NetShield!\n";
            break;
        default:
            cout << "Invalid choice!\n";
        }
    } while (choice != 0);

    return 0;
}
