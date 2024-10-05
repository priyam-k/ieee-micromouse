#include <iostream>
#include <vector>
#include <string>

using namespace std;

int main() {
    cout << "Hello, World!" << endl;

    vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};
    for (const string& word : msg) {
        cout << word << " ";
    }

    return 0;
}