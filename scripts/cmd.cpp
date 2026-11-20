#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    std::string cmd;

    while (true) {
        std::cout << fs::current_path().string() << "> ";

        if (!std::getline(std::cin, cmd)) {
            break;
        }

        if (cmd.empty()) {
            continue;
        }

        if (cmd == "exit") {
            break;
        }

        if (cmd.rfind("cd ", 0) == 0) {
            std::string path = cmd.substr(3);
            size_t first = path.find_first_not_of(' ');
            if (first != std::string::npos) {
                path = path.substr(first);
            }

            try {
                fs::current_path(path);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "The system cannot find the path specified.\n";
            }
        } else {
            std::system(cmd.c_str());
        }
    }

    return 0;
}