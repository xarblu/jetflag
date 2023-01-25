#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <vector>
#include <string>
#include <sstream>

void printHelp(char* path) {
    std::string basename = std::filesystem::path(path).filename();
    std::cout << "Usage: " << basename << " <action> [<packages>] [<flags>]" << std::endl << std::endl;;
    std::cout << "Action (one of):" << std::endl;
    std::cout << "           set          Set <flags>" << std::endl;
    std::cout << "           get          Get current flags" << std::endl;
    std::cout << "           help         Show this help message" << std::endl << std::endl;
    std::cout << "Packages:" << std::endl;
    std::cout << "           Full portage <category>/<package> atoms" << std::endl;
    std::cout << "           If left empty " << basename << " will operate globally on make.conf" << std::endl << std::endl;
    std::cout << "Flags (applies to \"set\"):" << std::endl;
    std::cout << "           +arg    explicitly enable arg" << std::endl;
    std::cout << "           -arg    explicitly disable arg" << std::endl;
    std::cout << "           %arg    reset arg to the default state (remove it from the file)" << std::endl;
}

std::string readFile(std::string file) {
            std::ifstream inFile;
            inFile.open(file);
            std::stringstream strStream;
            strStream << inFile.rdbuf();
            std::string str = strStream.str();
            inFile.close();
            return str;
}

int main(int argc, char *argv[]) {
        if (argc < 2) {
            std::cerr << "Not enough arguments!" << std::endl;
            printHelp(argv[0]);
            return 1;
        }

        std::string arg = argv[1];
        int action = 0;
        if (arg == "set") {
            action = 1;
        } else if (arg == "get") {
            action = 2;
        } else if (arg == "help") {
            action = 3;
        }

        if (action == 0) {
            std::cerr << "Invalid action!" << std::endl;
            printHelp(argv[0]);
            return 1;
        }

        std::vector<std::string> pkgs, enableFlags, disableFlags, resetFlags;
        std::regex pkgRegex("(([a-zA-Z]+((-)[a-zA-Z]+)*)+(/)([a-zA-Z]+((-)[a-zA-Z]+)*)+)|((([><]?(=)?)|(~)?)(([a-zA-Z]+((-)[a-zA-Z]+)*)+(/)([a-zA-Z]+((-)[a-zA-Z]+)*)+)-([0-9]+(([.])[0-9]+)*)+((_p)[0-9]+)?((-r)[0-9]+)?)");
        std::regex enableFlagRegex("([+])([a-zA-Z0-9])+");
        std::regex disableFlagRegex("(-)([a-zA-Z0-9])+");
        std::regex resetFlagRegex("(%)([a-zA-Z0-9])+");
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (std::regex_match(arg, pkgRegex)) {
                pkgs.push_back(arg);
            } else if (std::regex_match(arg, enableFlagRegex)) {
                enableFlags.push_back(arg);
            } else if (std::regex_match(arg, disableFlagRegex)) {
                disableFlags.push_back(arg);
            } else if (std::regex_match(arg, resetFlagRegex)) {
                resetFlags.push_back(arg);
            } else {
                std::cerr << arg << " is not a valid atom or flag!" << std::endl;
                return 1;
            }
        }

        std::string portageConfigRoot = "/etc/portage";
        if (std::getenv("PORTAGE_CONFIGROOT") != nullptr) {
            portageConfigRoot = std::getenv("PORTAGE_CONFIGROOT");
        }
        std::string packageUseDir = portageConfigRoot + "/package.use/";
        std::string makeConf = portageConfigRoot + "/make.conf";

        if (pkgs.empty()) {
            if (action == 2) {
                std::string contents = readFile(makeConf);
                std::regex useRegex("s*(USE=)([\"\'])(.*)([\"\'])");
                std::smatch match;
                std::string flagsOut;
                if (std::regex_search(contents, match, useRegex)) {
                    flagsOut = match[3];
                    flagsOut = std::regex_replace(flagsOut, std::regex("  "), " ");
                    std::cout << flagsOut << std::endl;
                    return 0;
                } else {
                    return 1;
                }
            }
        }
}
