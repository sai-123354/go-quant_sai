#include "env_loader.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
using namespace std;
void loadEnv(const std::string& filename) {
 
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open .env file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            setenv(key.c_str(), value.c_str(), 1);
        }
    }

    file.close();
}
