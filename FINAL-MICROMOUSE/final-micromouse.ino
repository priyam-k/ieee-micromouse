#include <SparkFun_TB6612.h>
#include "Simple_MPU6050.h"

#define MPU6050_DEFAULT_ADDRESS     0x68

Simple_MPU6050 mpu;

Motor rMotor = Motor(PB6, PB5, PB4, 1, PC7);  // motor A according to schematic
Motor lMotor = Motor(PA8, PA3, PB10, 1, PC7); // motor B according to schematic

int ir1 = PA0, ir2 = PA1, ir3 = PA4, ir4 = PB0, ir5 = PC0, ir6 = PC1; // Need to use properly

int lSpeed = 0, rSpeed = 0;
float angle = 0;

int orientation = 0;

const float DIAMETER = 80;
//const float PI = 3.14159;

float dist = 0;

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
  // Serial.print("angle: ");
  // Serial.print(angle);
  // Serial.println();
}

void readGyro() {
  mpu.dmp_read_fifo(false);
}

void setSpeeds(int newLSpeed, int newRSpeed) {
  if (lSpeed != newLSpeed) {
    lSpeed = newLSpeed;
    lMotor.drive(lSpeed);
    if (lSpeed == 0){
      //stopMotor(D10, D4, D5);
      lMotor.brake();
      Serial.println("LBRAKE");
    }
  }
  if (rSpeed != newRSpeed) {
    rSpeed = newRSpeed;
    rMotor.drive(rSpeed);
    if (rSpeed == 0){
      //stopMotor(D7, D8, D6);
      rMotor.brake();
      Serial.println("RBRAKE");
    }
  }
}

int lLastEncoded, lEncoderValue, rLastEncoded, rEncoderValue;

void resetEncoders() {
  lLastEncoded = lEncoderValue = rLastEncoded = rEncoderValue = dist = 0;
}


// EncoderResult updateEncoder(int last, int val, int a, int b){
//   Serial.println("shouldnt be updateEncodering");
//   int MSB = digitalRead(a);
//   int LSB = digitalRead(b);

//   int encoded = (MSB << 1) | LSB;
//   int sum  = (last << 2) | encoded;

//   if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) val++;
//   if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) val--;

//   return {encoded, val};
// }

float encoderToDist(int enc) {
  return enc / 280.0 * DIAMETER * PI;
}

void stopMotor(int in1, int in2, int pwm){  // not using
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(pwm, HIGH);
  Serial.println("should have stopped a motor");
}

// not sure if using right pins for update encoder functions
void updateEncoderLeft(){
  int MSB = digitalRead(PA2);
  int LSB = digitalRead(PB3);
  int encoded = (MSB << 1) | LSB;
  int sum = (lLastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) lEncoderValue++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) lEncoderValue--;
  lLastEncoded = encoded;
  // Serial.println(encoderToDist(lEncoderValue));
}

void updateEncoderRight(){
  int MSB = digitalRead(PA6);
  int LSB = digitalRead(PA5);
  int encoded = (MSB << 1) | LSB;
  int sum = (rLastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) rEncoderValue--;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) rEncoderValue++;
  rLastEncoded = encoded;
  // Serial.println(encoderToDist(rEncoderValue));
}

// void updateEncoders() {
//   Serial.println("shoudlnt be updating encoeders!!!");
//   updateEncoderLeft();
//   updateEncoderRight();
//   dist = (lEncoderValue + rEncoderValue) / 2.0 / 280.0 * DIAMETER * PI;
//   //Serial.println("noooooooooooooooo dist: " + dist + "    ldist: " + encoderToDist(lEncoderValue) + "    rdist: " + encoderToDist(rEncoderValue));
// }

float rad(float deg) {
  return (deg * PI) / 180.0;
}

void straight(float d) {
  const int TOPSPEED = 200;
  const float VRATIO = 0.5;
  const float per = 0.8;
  int speed = TOPSPEED;
  resetEncoders();
  float rDist = 0;
  float lDist = 0;
  int rPrevEnc = rEncoderValue;
  int lPrevEnc = lEncoderValue;
  float a = orientation, r = 1;
  float t = millis();
  while (dist < (d/per)) {
    
    

    dist = (lEncoderValue + rEncoderValue) / 2.0 / 280.0 * DIAMETER * PI;
    //Serial.println("new spd: " + String(speed) + "    dist: " + String(dist) + "    ldist: " + String(encoderToDist(lEncoderValue)) + "    rdist: " + String(encoderToDist(rEncoderValue)));
    readGyro();
    float newLSpeed = lSpeed, newRSpeed = rSpeed;
    float an = angle;

    rDist += encoderToDist(rEncoderValue - rPrevEnc) * cos(rad(abs(an - a)));
    rPrevEnc = rEncoderValue;
    lDist += encoderToDist(lEncoderValue - lPrevEnc) * cos(rad(abs(an - a)));
    lPrevEnc = lEncoderValue;
    //Serial.println("ldist: " + String(lDist) + "   rdist: " + String(rDist) + "    an diff: " + String(an-a));

    dist = (rDist + lDist)/2;

    if (orientation == 0 && an > 180) an -= 360;
    Serial.println(an);
    
    speed = TOPSPEED * (1 + (dist*(VRATIO-1)) / d);
    //Serial.println("spd: " + String(speed) + "   dist: " + String(dist) + "   d: " + String(d) + "   fac: " + String((1 + (dist*(VRATIO-1)) / d)));
    //Serial.println("an: " + String(an) + "    target: " + String(a));
    if (an > a + r) {
      newRSpeed = speed;
      newLSpeed = 0.9*speed;
    }
    else if (an < a - r) {
      newLSpeed = speed;
      newRSpeed = -1.25*speed;
    }
    else {
      newLSpeed = speed;
      newRSpeed = speed;
    }
    if (dist < d) {
      setSpeeds(newLSpeed, newRSpeed);
      if (millis() - t > 15*d && abs(dist - d) < 10) break;
    } else {
      setSpeeds(-TOPSPEED, -TOPSPEED);
    }
  }
  setSpeeds(0, 0);
  delay(1000);
}

void left() {
  right();
  right();
  right();
  // orientation -= 90;
  // if (orientation < 0) orientation += 360;
  // float t = millis(), a = orientation, r = 1;
  // while (millis() - t < 25000) {
  //   readGyro();
  //   float newLSpeed = lSpeed, newRSpeed = rSpeed;
  //   float an = angle;
  //   if ((orientation == 270 || orientation == 0) && an > 180) an -= 360;
  //   if (an > a + r) {
  //     Serial.println("RIGHTING");
  //     newLSpeed = 250;
  //     newRSpeed = -250;
  //   }
  //   else if (an < a - r) {
  //     Serial.println("LEFTING");
  //     newLSpeed = -250;
  //     // newRSpeed = 250;
  //   }
  //   else {
  //     newLSpeed = 0;
  //     newRSpeed = 0;
  //   }
  //   setSpeeds(newLSpeed, newRSpeed);
  // }
  // setSpeeds(0, 0);
  // delay(100);
}

void rightNew() {
   orientation += 90;
  if (orientation >= 360) orientation = 0;
  float t = millis(), a = orientation, r = 1;
    
    /*if (an > a + r) {
      while (an > a + r) {
        setSpeeds(-250, 0);
      }
    }*/
    float newLSpeed = lSpeed, newRSpeed = rSpeed;
    float an = angle;
    if ((orientation == 0 || orientation == 90) && an > 180) an -= 360;
    setSpeeds(250,250);
    delay(100);
    while (an < a - r) {
      readGyro();
      setSpeeds(250, 0);
      delay(400);
      setSpeeds(0, -250);
      delay(300);
      //newRSpeed = -250;
      //newLSpeed = 170;
      an = angle;
    if ((orientation == 0 || orientation == 90) && an > 180) an -= 360;
    }
    
    //setSpeeds(newLSpeed, newRSpeed);
  setSpeeds(0, 0);
  delay(100);
}

void right() {
  orientation += 90;
  if (orientation >= 360) orientation = 0;
  float t = millis(), a = orientation, r = 1;
  while (millis() - t < 2500) { 
    readGyro();
    float newLSpeed = lSpeed, newRSpeed = rSpeed;
    float an = angle;
    if ((orientation == 0 || orientation == 90) && an > 180) an -= 360;
    if (an > a + r) {
      newRSpeed = 250;
      newLSpeed = -170;
    }
    else if (an < a - r) {
      newRSpeed = -250;
      newLSpeed = 170;
    }
    else {
      newLSpeed = 0;
      newRSpeed = 0;
    }
    setSpeeds(newLSpeed, newRSpeed);
  }
  setSpeeds(0, 0);
  delay(100);
}

void calibrateGyro() {
  mpu.begin();
  mpu.Set_DMP_Output_Rate_Hz(200);
  mpu.CalibrateMPU();
  mpu.load_DMP_Image();
  mpu.on_FIFO(updateAngle);
}

bool wallLeft() {
  return analogRead(ir4) > 0.5;
}
bool wallRight() {
  return analogRead(ir2) > 0.5;
}
bool wallFront() {
  return analogRead(ir1) > 0.5;
}

void turnRight() {
  rightNew();
}
void turnLeft() {
  left();
}
void moveForward() {
  straight(90);
}

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

void runMouse(Pair pos, Pair dxy, const Pair target) {
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

void setup() {
  Serial.begin(115200);
  calibrateGyro();

  attachInterrupt(PA5, updateEncoderRight, CHANGE); 
  attachInterrupt(PA6, updateEncoderRight, CHANGE);
  attachInterrupt(PB3, updateEncoderLeft, CHANGE); 
  attachInterrupt(PA2, updateEncoderLeft, CHANGE);

  runMouse({0, 15}, {0, -1}, {7, 7});

  // resetEncoders();
  //straight(3000);
  //straight(300);
  // left();
  //straight(100);
  // left();

  //straight(40);
  //right();

  // Code from main function
  // flood_fill(7, 7);
  // //updateGUIflood();
  // // log("Done!");

  // Pair dxy = Pair(0, -1); // direction vector
  // Pair pos = Pair(0, 15); // starting position

  // runMouse(pos, dxy, Pair(7, 7));

  // //API::setColor(0, 0, 'G');
  // //API::setText(0, 0, "abc");
  // while (false) {
  //     if (!wallLeft()) {
  //         turnLeft();
  //     }
  //     while (wallFront()) {
  //         turnRight();
  //     }
  //     moveForward();
  // }
}

void loop() {
  // Serial.print(digitalRead(irF2));
  // Serial.print("    ");
  // Serial.print(digitalRead(irR)); // right
  // Serial.print("    ");
  // Serial.print(digitalRead(irF1));
  // Serial.print("    ");
  // Serial.print(digitalRead(irL)); // left
  // Serial.println("    ");
  //Serial.println("left: " + String(wallLeft()) + "   right: " + String(wallRight()) + "   front: " + String(digitalRead(irF1)) + "  " + String(digitalRead(irF)));
}