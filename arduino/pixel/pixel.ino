#include <StandardCplusplus.h>

//#include <iostream>
//#include <string>
//#include <vector>

#include <SparkFun_TB6612.h>
#include "Simple_MPU6050.h"

#define MPU6050_DEFAULT_ADDRESS     0x68

Simple_MPU6050 mpu;

Motor motor24 = Motor(10, 4, 5, 1, 9);
Motor motor13 = Motor(7, 8, 6, 1, 9);

int speed13 = 0, speed24 = 0;
float angle = 0;

int orientation = 0;

const float wheel_diam = 6.7;
const float pi = 3.14159;
const float encoder_slots = 18;

int clicksR = 0, clicksL = 0;
bool cRrising = false, cLrising = false;
float dist = 0;

// data type for each cell in maze
struct Cell {
    bool top = false;
    bool right = false;
    bool bottom = false;
    bool left = false;
    void setByName(const String& name, bool value) {
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
            //throw std::underflow_error("Queue is empty");
            return {-1,-1};
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
    // void printQueue() const {
    //     std::cout << "Queue: ";
    //     for (int i = 0; i < size; ++i) {
    //         Pair p = data[(front + i) % capacity];
    //         std::cout << "(" << p.first << ", " << p.second << ") ";
    //     }
    //     std::cout << std::endl;
    // }
};


void updateClicks() {
  int cL = digitalRead(2), cR = digitalRead(3);
  if (cL == 0 && cLrising) cLrising = false;
  else if (cL == 1 && !cLrising) {
    cLrising = true;
    clicksL++;
  }
  if (cR == 0 && cRrising) cRrising = false;
  else if (cR == 1 && !cRrising) {
    cRrising = true;
    clicksR++;
  }
  dist = (wheel_diam * pi) / encoder_slots * (clicksR + clicksL) / 2;
}

void updateAngle(int16_t *gyro, int16_t *accel, int32_t *quat) {
  Quaternion q;
  VectorFloat gravity;
  float ypr[3] = { 0, 0, 0 };
  float xyz[3] = { 0, 0, 0 };
  mpu.GetQuaternion(&q, quat);
  mpu.GetGravity(&gravity, &q);
  mpu.GetYawPitchRoll(ypr, &q, &gravity);
  mpu.ConvertToDegrees(ypr, xyz);
  angle = xyz[0];
  if (xyz[0] < 0) angle = 360 + xyz[0];
  Serial.print("angle: ");
  Serial.print(angle);
  Serial.println();
}

void readGyro() {
  mpu.dmp_read_fifo(false);
}

void setSpeeds(int newspeed13, int newspeed24) {
  if (speed13 != newspeed13) {
    speed13 = newspeed13;
    motor13.drive(speed13);
  }
  if (speed24 != newspeed24) {
    speed24 = newspeed24;
    motor24.drive(speed24);
  }
}

void straight(float d) {
  dist = 0;
  clicksR = 0;
  clicksL = 0;
  cRrising = false;
  cLrising = false;
  float a = orientation, r = 1;
  while (dist < d) {
    readGyro();
    updateClicks();
    float newspeed13 = speed13, newspeed24 = speed24;
    float an = angle;
    if (orientation == 0 && an > 180) an -= 360;
    if (an > a + r) {
      newspeed24 = 185;
      newspeed13 = 135;
    }
    else if (an < a - r) {
      newspeed13 = 185;
      newspeed24 = 135;
    }
    else {
      newspeed13 = 135;
      newspeed24 = 135;
    }
    setSpeeds(newspeed13, newspeed24);
  }
  setSpeeds(0, 0);
  delay(100);
}

void left() {
  orientation -= 90;
  if (orientation < 0) orientation += 360;
  float t = millis(), a = orientation != 270 ? orientation : -90, r = 0.5;
  while (millis() - t < 1500) {
    readGyro();
    float newspeed13 = speed13, newspeed24 = speed24;
    float an = angle;
    if ((orientation == 270 || orientation == 0) && an > 180) an -= 360;
    if (an > a + r) {
      newspeed24 = 175;
      newspeed13 = -175;
    }
    else if (an < a - r) {
      newspeed24 = -125;
      newspeed13 = 125;
    }
    else {
      newspeed13 = 0;
      newspeed24 = 0;
    }
    setSpeeds(newspeed13, newspeed24);
  }
  setSpeeds(0, 0);
  delay(100);
}

void right() {
  orientation += 90;
  if (orientation >= 360) orientation = 0;
  float t = millis(), a = orientation, r = 0.5;
  while (millis() - t < 1500) {
    readGyro();
    float newspeed13 = speed13, newspeed24 = speed24;
    float an = angle;
    if ((orientation == 0 || orientation == 90) && an > 180) an -= 360;
    if (an > a + r) {
      newspeed24 = 175;
      newspeed13 = -175;
    }
    else if (an < a - r) {
      newspeed24 = -125;
      newspeed13 = 125;
    }
    else {
      newspeed13 = 0;
      newspeed24 = 0;
    }
    setSpeeds(newspeed13, newspeed24);
  }
  setSpeeds(0, 0);
  delay(100);
}

void calibrateGyro() {
  mpu.begin();
  Serial.println("beginned");
  mpu.Set_DMP_Output_Rate_Hz(200);
  Serial.println("hertzd");
  mpu.CalibrateMPU();
  Serial.println("calibrated");
  mpu.load_DMP_Image();
  Serial.println("imaged");
  mpu.on_FIFO(updateAngle);
  Serial.println("angled");
}

volatile int lastEncoded = 0; // Here updated value of encoder store.
volatile long encoderValue = 0; // Raw encoder value


void updateEncoder(){
  int MSB = digitalRead(12); //MSB = most significant bit
  int LSB = digitalRead(13); //LSB = least significant bit

  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue --;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue ++;

  lastEncoded = encoded; //store this value for next time
  //Serial.print("encoder: ");
  Serial.println(encoderValue);
}








//#include "API.h"

bool wallLeft() {}
bool wallRight() {}
bool wallFront() {}

void turnRight() {}
void turnLeft() {}
void moveForward() {}

// void log(const std::string& text) {
//     std::cerr << text << std::endl;
// }




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
    Queue queue;
    //std::vector<Pair> queue; // queue of cells to visit
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
    
}

// void updateGUIflood() {
//     for (int i = 0; i < 16; ++i) {
//         for (int j = 0; j < 16; ++j) {
//             API::setText(j, 15 - i, std::to_string(flood[i][j]));
//         }
//     }
// }

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

String get_dir(const String& dir, const Pair& dxy) {
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
                wallFront());
        if (inMaze(pos + dxy)) { //set other side of front wall
            cells[(pos+dxy).second][(pos+dxy).first]
                .setByName(
                    get_dir("back", dxy),
                    wallFront());
        }

        cells[pos.second][pos.first] //set right wall
            .setByName(
                get_dir("right", dxy),
                wallRight());
        if (inMaze(pos+orient('r', dxy))) { //set other side of right wall
            cells[(pos+orient('r', dxy)).second][(pos+orient('r', dxy)).first]
                .setByName(
                    get_dir("left", dxy),
                    wallRight());
        }

        cells[pos.second][pos.first] //set left wall
            .setByName(
                get_dir("left", dxy),
                wallLeft());
        if (inMaze(pos+orient('l', dxy))) { //set other side of left wall
            cells[(pos+orient('l', dxy)).second][(pos+orient('l', dxy)).first]
                .setByName(
                    get_dir("right", dxy),
                    wallLeft());
        }
        
        flood_fill(target.first, target.second);
        //updateGUIflood();

        Pair next = pos+dxy;
        while (!is_path(pos.first, pos.second, next.first, next.second)
            || !(flood[next.second][next.first] < flood[pos.second][pos.first])) {
            turnRight();
            dxy = orient('r', dxy);
            next = pos+dxy;
        }
        moveForward();
        pos = next;
    }
}


int main(int argc, char* argv[]) {
    // log("Running...");

    flood_fill(7, 7);
    //updateGUIflood();
    // log("Done!");

    Pair dxy = Pair(0, -1); // direction vector
    Pair pos = Pair(0, 15); // starting position

    runMouse(pos, dxy, Pair(7, 7));

    //API::setColor(0, 0, 'G');
    //API::setText(0, 0, "abc");
    while (false) {
        if (!wallLeft()) {
            turnLeft();
        }
        while (wallFront()) {
            turnRight();
        }
        moveForward();
    }
}







void setup() {
  Serial.begin(115200);

  calibrateGyro();
  Serial.println("done stuff");

  motor24.drive(255);
  motor13.drive(255);
  
  
}


void loop() {
  //readGyro();
  updateEncoder();
}
