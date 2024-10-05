#include <iostream>
#include <string>
#include <vector>

#include "API.h"

void log(const std::string& text) {
    std::cerr << text << std::endl;
}

// data type for each cell in maze
struct Cell {
    bool top = false;
    bool right = false;
    bool bottom = false;
    bool left = false;
    char color;
    std::string text;
    bool getByName(const std::string& name) {
        if (name == "top") return top;
        if (name == "right") return right;
        if (name == "bottom") return bottom;
        if (name == "left") return left;
        return false;
    }
    void setByName(const std::string& name, bool value) {
        if (name == "top") top = value;
        if (name == "right") right = value;
        if (name == "bottom") bottom = value;
        if (name == "left") left = value;
    }
};

Cell cells[16][16]; // 16x16 maze (of known cells)
int flood[16][16]; // flood fill values

bool is_path(int x1, int y1, int x2, int y2){
    if (x1 == x2) {
        if (y1 == y2 + 1) {
            return !cells[y1][x1].top && !cells[y2][x2].bottom;
        } else if (y1 == y2 - 1) {
            return !cells[y1][x1].bottom && !cells[y2][x2].top;
        }
    } else if (y1 == y2) {
        if (x1 == x2 + 1) {
            return !cells[y1][x1].left && !cells[y2][x2].right;
        } else if (x1 == x2 - 1) {
            return !cells[y1][x1].right && !cells[y2][x2].left;
        }
    } else {
        return false;
    }
}

void flood_fill(int startx, int starty) {
    // fill flood array with 999
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flood[i][j] = 999;
        }
    }

    flood[startx][starty] = 0; // starting point is distance 0
    std::vector<std::pair<int, int>> queue; // queue of cells to visit
    queue.push_back({startx, starty}); // add first cell to queue

    while (!queue.empty()){
        // get the first cell in the queue
        std::pair<int,int> begin = queue.front();
        queue.erase(queue.begin());
        int x = begin.first;
        int y = begin.second;

        // check all neighboring cells
        if (x > 0 && flood[y][x-1] == 999 && is_path(x, y, x-1, y)){ // left
            flood[y][x-1] = flood[y][x] + 1;
            queue.push_back({x-1, y});
        }
        if (x < 15 && flood[y][x+1] == 999 && is_path(x, y, x+1, y)){ // right
            flood[y][x+1] = flood[y][x] + 1;
            queue.push_back({x+1, y});
        }
        if (y > 0 && flood[y-1][x] == 999 && is_path(x, y, x, y-1)){ // up
            flood[y-1][x] = flood[y][x] + 1;
            queue.push_back({x, y-1});
        }
        if (y < 15 && flood[y+1][x] == 999 && is_path(x, y, x, y+1)){ // down
            flood[y+1][x] = flood[y][x] + 1;
            queue.push_back({x, y+1});
        }
    }
}

void updateGUIflood() {
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            API::setText(j, 15 - i, std::to_string(flood[i][j]));
        }
    }
}

std::pair<int, int> orient(char dir, const std::pair<int, int>& dxy) {
    std::pair<int, int> ndxy;  // New dxy after turning
    
    if (tolower(dir) == 'r') {
        // Turn right (rotate 90 degrees clockwise)
        ndxy = std::make_pair(-dxy.second, dxy.first);
    } else if (tolower(dir) == 'l') {
        // Turn left (rotate 90 degrees counter-clockwise)
        ndxy = std::make_pair(dxy.second, -dxy.first);
    } else if (tolower(dir) == 'b') {
        // Turn back (rotate 180 degrees)
        ndxy = std::make_pair(-dxy.first, -dxy.second);
    } else {
        // No turn
        ndxy = dxy;
    }
    
    return ndxy;
}

std::string get_dir(const std::string& dir, const std::pair<int, int>& dxy) {
    if (dxy == std::make_pair(0, -1)) { // up
        if (dir == "front") return "top";
        if (dir == "back") return "bottom";
        if (dir == "left") return "left";
        if (dir == "right") return "right";
    } else if (dxy == std::make_pair(0, 1)) { // down
        if (dir == "front") return "bottom";
        if (dir == "back") return "top";
        if (dir == "left") return "right";
        if (dir == "right") return "left";
    } else if (dxy == std::make_pair(-1, 0)) { // left
        if (dir == "front") return "left";
        if (dir == "back") return "right";
        if (dir == "left") return "bottom";
        if (dir == "right") return "top";
    } else if (dxy == std::make_pair(1, 0)) { // right
        if (dir == "front") return "right";
        if (dir == "back") return "left";
        if (dir == "left") return "top";
        if (dir == "right") return "bottom";
    }
    return ""; // in case of an invalid direction
}

bool inMaze(const std::pair<int, int>& pos) {
    return pos.first >= 0 && pos.first < 16 && pos.second >= 0 && pos.second < 16;
}

std::pair<int,int> addPair(const std::pair<int, int>& pos, const std::pair<int, int>& dxy) {
    return std::make_pair(pos.first + dxy.first, pos.second + dxy.second);
}

void runMouse(std::pair<int, int>& pos, std::pair<int,int>& dxy, const std::pair<int,int>& target) {
    while (pos != target) {
        cells[pos.second][pos.first] //set front wall
            .setByName(
                get_dir("front", dxy),
                API::wallFront());
        if (inMaze(addPair(pos, dxy))) { //set other side of front wall
            cells[addPair(pos, dxy).second][addPair(pos, dxy).first]
                .setByName(
                    get_dir("back", dxy),
                    API::wallFront());
        }

        cells[pos.second][pos.first] //set right wall
            .setByName(
                get_dir("right", dxy),
                API::wallRight());
        if (inMaze(addPair(pos, orient('r', dxy)))) { //set other side of right wall
            cells[addPair(pos, orient('r', dxy)).second][addPair(pos, orient('r', dxy)).first]
                .setByName(
                    get_dir("left", dxy),
                    API::wallRight());
        }

        cells[pos.second][pos.first] //set left wall
            .setByName(
                get_dir("left", dxy),
                API::wallLeft());
        if (inMaze(addPair(pos, orient('l', dxy)))) { //set other side of left wall
            cells[addPair(pos, orient('l', dxy)).second][addPair(pos, orient('l', dxy)).first]
                .setByName(
                    get_dir("right", dxy),
                    API::wallLeft());
        }
        
        flood_fill(target.first, target.second);
        updateGUIflood();

        std::pair<int, int> next = addPair(pos, dxy);
        while (!is_path(pos.first, pos.second, next.first, next.second)
            || !(flood[next.second][next.first] < flood[pos.second][pos.first])) {
            API::turnRight();
            dxy = orient('r', dxy);
            next = addPair(pos, dxy);
        }
        API::moveForward();
        pos = next;
    }
}


int main(int argc, char* argv[]) {
    log("Running...");

    flood_fill(7, 7);
    updateGUIflood();
    log("Done!");

    std::pair<int,int> dxy = std::make_pair(0, -1); // direction vector
    std::pair<int,int> pos = std::make_pair(0, 15); // starting position

    runMouse(pos, dxy, std::make_pair(7, 7));

    API::setColor(0, 0, 'G');
    API::setText(0, 0, "abc");
    while (false) {
        if (!API::wallLeft()) {
            API::turnLeft();
        }
        while (API::wallFront()) {
            API::turnRight();
        }
        API::moveForward();
    }
}