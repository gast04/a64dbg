#include <iostream>
#include <fstream>

using namespace std;

int main() {
	string filename = "/data/local/tmp/testfile";

	fstream my_file;
	my_file.open(filename, ios::in);
	if (!my_file) {
		cout << "Could not open file!\n";
	}

	string content;
	my_file >> content;
	cout << "Content: " << content << endl;

	my_file.close(); 
	return 0;
}
