// Simple terminal 2D game in plain C++ (no external libraries).
// Compile: g++ -std=c++17 game.cpp -o game
// Controls: W A S D then Enter to move. Reach G, collect * for points, avoid E.

#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <thread>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <cstdlib>
#endif

using namespace std;

struct Pos { int r, c; bool operator==(Pos const& o) const { return r == o.r && c == o.c; } bool operator!=(Pos const& o) const {
    return !(*this == o);
}
};

int rows = 12;
int cols = 30;
int numStars = 6;
int numEnemies = 3;

vector<string> makeEmptyBoard(int r, int c) {
    return vector<string>(r, string(c, ' '));
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    // ANSI clear: works on most Unix terminals and modern Windows terminals
    cout << "\x1B[2J\x1B[H";
#endif
}

void drawBoard(const vector<string>& board, int score, int turns) {
    clearScreen();
    for (auto& row : board) cout << '|' << row << "|\n";
    cout << "\nScore: " << score << "    Turns: " << turns << "\n";
    cout << "Controls: W A S D + Enter. Reach 'G' to win. Collect '*' for +1. Avoid 'E'.\n";
}

bool inside(Pos p) { return p.r >= 0 && p.r < rows&& p.c >= 0 && p.c < cols; }

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // seeded random
    std::mt19937 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> dr(0, rows - 1), dc(0, cols - 1);

    // create board and place items
    auto board = makeEmptyBoard(rows, cols);
    Pos player{ rows / 2, cols / 2 };
    Pos goal;
    do { goal = { dr(rng), dc(rng) }; } while (goal == player);

    vector<Pos> stars;
    while ((int)stars.size() < numStars) {
        Pos s{ dr(rng), dc(rng) };
        if (s == player || s == goal) continue;
        if (find(stars.begin(), stars.end(), s) == stars.end()) stars.push_back(s);
    }

    vector<Pos> enemies;
    while ((int)enemies.size() < numEnemies) {
        Pos e{ dr(rng), dc(rng) };
        if (e == player || e == goal) continue;
        if (find(enemies.begin(), enemies.end(), e) == enemies.end()) enemies.push_back(e);
    }

    int score = 0;
    int turns = 0;
    bool alive = true;
    

    while (alive) {
        // clear board
        board = makeEmptyBoard(rows, cols);

        // put goal
        board[goal.r][goal.c] = 'G';

        // put stars
        for (auto& s : stars) board[s.r][s.c] = '*';

        // put enemies
        for (auto& e : enemies) board[e.r][e.c] = 'E';

        // put player (overwrites if on same tile, we handle collision later)
        board[player.r][player.c] = '@';

        // draw
        drawBoard(board, score, turns);

        // check win/lose immediate (in case spawn overlap)
        if (player == goal) { cout << "\nYou reached the goal. You win!\n"; break; }
        for (auto& e : enemies) if (player == e) { cout << "\nYou bumped into an enemy. Game over.\n"; alive = false; break; }
        if (!alive) break;

        // get input (single char + Enter)
        cout << "Move (W/A/S/D): ";
        string line;
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        char cmd = line[0];
        Pos np = player;
        if (cmd == 'w' || cmd == 'W') np.r--;
        else if (cmd == 's' || cmd == 'S') np.r++;
        else if (cmd == 'a' || cmd == 'A') np.c--;
        else if (cmd == 'd' || cmd == 'D') np.c++;
        else { cout << "Invalid key. Use W/A/S/D.\n"; std::this_thread::sleep_for(std::chrono::milliseconds(250)); continue; }

        if (inside(np)) player = np;
        // collect star?
        auto it = find(stars.begin(), stars.end(), player);
        if (it != stars.end()) { score++; stars.erase(it); }

        // reached goal?
        if (player == goal) {
            drawBoard(board, score, turns);
            cout << "\nYou reached the goal. You win!\n";
            break;
        }

        // enemies move randomly (simple AI)
        for (auto& e : enemies) {
            // choose random direction or stay
            int dir = std::uniform_int_distribution<int>(0, 4)(rng);
            Pos ne = e;
            if (dir == 0) ne.r--;
            else if (dir == 1) ne.r++;
            else if (dir == 2) ne.c--;
            else if (dir == 3) ne.c++;
            if (inside(ne) && ne != goal) e = ne;
        }

        // enemy-player collisions
        for (auto& e : enemies) if (e == player) { drawBoard(board, score, turns); cout << "\nAn enemy caught you. Game over.\n"; alive = false; break; }
        if (!alive) break;

        // maybe spawn a new star occasionally
        if (turns % 12 == 0 && (int)stars.size() < numStars) {
            Pos s;
            int tries = 0;
            do { s = { dr(rng), dc(rng) }; tries++; } while ((s == player || s == goal) && tries < 50);
            if (!(s == player || s == goal)) stars.push_back(s);
        }

        // small victory condition: collect everything then reach goal for bonus
        if (stars.empty()) {
            cout << "\nAll stars collected. Now go to G for a bonus!\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        turns++;
    }

    cout << "\nFinal score: " << score << "   Turns: " << turns << "\nThanks for playing.\n";
    return 0;
}
