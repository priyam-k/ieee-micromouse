#include <iostream>
#include <string>
//#include <vector>

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

struct Pair {
    Pair() : first(0), second(0) {}
    Pair(int first, int second) : first(first), second(second) {}

    bool operator==(const Pair& other) const {
        return first == other.first && second == other.second;
    }

    bool operator!=(const Pair& other) const {
        return first != other.first || second != other.second;
    }

    Pair operator+(const Pair& other) const {
        return Pair(first + other.first, second + other.second);
    }

    int first;
    int second;
};

class Queue {
private:
    Pair* data;      // Dynamic array for queue storage
    int front;       // Index of the front element
    int back;        // Index of the back element
    int capacity;    // Current capacity of the queue
    int size;        // Current number of elements in the queue

    // Function to resize the array when needed
    void resize(int new_capacity) {
        Pair* new_data = new Pair[new_capacity];
        // Copy data in correct order from the circular queue
        for (int i = 0; i < size; ++i) {
            new_data[i] = data[(front + i) % capacity];
        }
        delete[] data;    // Free the old memory
        data = new_data;  // Point to the new array
        front = 0;        // Reset front to 0
        back = size;      // Reset back to size (new end of queue)
        capacity = new_capacity;
    }

public:
    // Constructor
    Queue(int initial_capacity = 4) : front(0), back(0), capacity(initial_capacity), size(0) {
        data = new Pair[capacity];
    }

    // Destructor
    ~Queue() {
        delete[] data;
    }

    // Enqueue an element at the back
    void enqueue(const Pair& value) {
        if (size == capacity) {
            // Double the capacity when full
            resize(2 * capacity);
        }
        data[back] = value;
        back = (back + 1) % capacity;  // Wrap around using modulo
        ++size;
    }

    // Dequeue an element from the front
    Pair dequeue() {
        if (size == 0) {
            throw std::underflow_error("Queue is empty");
        }
        Pair value = data[front];
        front = (front + 1) % capacity; // Move front forward and wrap around
        --size;

        // Shrink the array if usage drops significantly (quarter full)
        if (size > 0 && size == capacity / 4) {
            resize(capacity / 2);
        }

        return value;
    }

    // Check if the queue is empty
    bool isEmpty() const {
        return size == 0;
    }

    // Get the size of the queue
    int getSize() const {
        return size;
    }

    // Print the entire queue for debugging purposes
    void printQueue() const {
        std::cout << "Queue: ";
        for (int i = 0; i < size; ++i) {
            Pair p = data[(front + i) % capacity];
            std::cout << "(" << p.first << ", " << p.second << ") ";
        }
        std::cout << std::endl;
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
    }
    return false;
}

void flood_fill(int startx, int starty) {
    // fill flood array with 999
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            flood[i][j] = 999;
        }
    }

    flood[startx][starty] = 0; // starting point is distance 0
    Queue queue; // queue of cells to visit
    queue.enqueue({startx, starty}); // add first cell to queue

    while (!queue.isEmpty()){
        // get the first cell in the queue
        Pair begin = queue.dequeue();
        int x = begin.first;
        int y = begin.second;

        // check all neighboring cells
        if (x > 0 && flood[y][x-1] == 999 && is_path(x, y, x-1, y)){ // left
            flood[y][x-1] = flood[y][x] + 1;
            queue.enqueue({x-1, y});
        }
        if (x < 15 && flood[y][x+1] == 999 && is_path(x, y, x+1, y)){ // right
            flood[y][x+1] = flood[y][x] + 1;
            queue.enqueue({x+1, y});
        }
        if (y > 0 && flood[y-1][x] == 999 && is_path(x, y, x, y-1)){ // up
            flood[y-1][x] = flood[y][x] + 1;
            queue.enqueue({x, y-1});
        }
        if (y < 15 && flood[y+1][x] == 999 && is_path(x, y, x, y+1)){ // down
            flood[y+1][x] = flood[y][x] + 1;
            queue.enqueue({x, y+1});
        }
    }
    //free(&queue);
}

void updateGUIflood() {
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            API::setText(j, 15 - i, std::to_string(flood[i][j]));
        }
    }
}

Pair orient(char dir, const Pair& dxy) {
    Pair ndxy;  // New dxy after turning
    
    if (tolower(dir) == 'r') {
        // Turn right (rotate 90 degrees clockwise)
        ndxy = Pair(-dxy.second, dxy.first);
    } else if (tolower(dir) == 'l') {
        // Turn left (rotate 90 degrees counter-clockwise)
        ndxy = Pair(dxy.second, -dxy.first);
    } else if (tolower(dir) == 'b') {
        // Turn back (rotate 180 degrees)
        ndxy = Pair(-dxy.first, -dxy.second);
    } else {
        // No turn
        ndxy = dxy;
    }
    
    return ndxy;
}

std::string get_dir(const std::string& dir, const Pair& dxy) {
    if (dxy == Pair(0, -1)) { // up
        if (dir == "front") return "top";
        if (dir == "back") return "bottom";
        if (dir == "left") return "left";
        if (dir == "right") return "right";
    } else if (dxy == Pair(0, 1)) { // down
        if (dir == "front") return "bottom";
        if (dir == "back") return "top";
        if (dir == "left") return "right";
        if (dir == "right") return "left";
    } else if (dxy == Pair(-1, 0)) { // left
        if (dir == "front") return "left";
        if (dir == "back") return "right";
        if (dir == "left") return "bottom";
        if (dir == "right") return "top";
    } else if (dxy == Pair(1, 0)) { // right
        if (dir == "front") return "right";
        if (dir == "back") return "left";
        if (dir == "left") return "top";
        if (dir == "right") return "bottom";
    }
    return ""; // in case of an invalid direction
}

bool inMaze(const Pair& pos) {
    return pos.first >= 0 && pos.first < 16 && pos.second >= 0 && pos.second < 16;
}

void runMouse(Pair& pos, Pair& dxy, const Pair& target) {
    while (pos != target) {
        cells[pos.second][pos.first] //set front wall
            .setByName(
                get_dir("front", dxy),
                API::wallFront());
        if (inMaze(pos + dxy)) { //set other side of front wall
            cells[(pos+dxy).second][(pos+dxy).first]
                .setByName(
                    get_dir("back", dxy),
                    API::wallFront());
        }

        cells[pos.second][pos.first] //set right wall
            .setByName(
                get_dir("right", dxy),
                API::wallRight());
        if (inMaze(pos+orient('r', dxy))) { //set other side of right wall
            cells[(pos+orient('r', dxy)).second][(pos+orient('r', dxy)).first]
                .setByName(
                    get_dir("left", dxy),
                    API::wallRight());
        }

        cells[pos.second][pos.first] //set left wall
            .setByName(
                get_dir("left", dxy),
                API::wallLeft());
        if (inMaze(pos+orient('l', dxy))) { //set other side of left wall
            cells[(pos+orient('l', dxy)).second][(pos+orient('l', dxy)).first]
                .setByName(
                    get_dir("right", dxy),
                    API::wallLeft());
        }
        
        flood_fill(target.first, target.second);
        updateGUIflood();

        Pair next = pos+dxy;
        while (!is_path(pos.first, pos.second, next.first, next.second)
            || !(flood[next.second][next.first] < flood[pos.second][pos.first])) {
            API::turnRight();
            dxy = orient('r', dxy);
            next = pos+dxy;
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

    Pair dxy = Pair(0, -1); // direction vector
    Pair pos = Pair(0, 15); // starting position

    runMouse(pos, dxy, Pair(7, 7));

    API::setColor(0, 0, 'G');
    API::setText(0, 0, "abhdbekf");
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