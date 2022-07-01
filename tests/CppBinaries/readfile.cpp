#include <iostream>
#include <fstream>

#include <unistd.h>

using namespace std;

void openFile(std::string filename) {

    fstream my_file;
    my_file.open(filename, ios::in);
    if (!my_file) {
        cout << "Could not open file!\n";
        return;
    }

    string content;
    my_file >> content;
    cout << "Content: " << content << endl;

    my_file.close();
}

int main() {

    // open syscall test
    openFile("/data/local/tmp/testfile");

    // fork test for checking follow fork and clone
    pid_t child = fork();
    if (child == 0) {
        printf("child process\n");
        printf("do we catch this writes in a64dbg?\n");
        openFile("/data/local/tmp/childfile");
        return 0;
    }

    return 0;
}
