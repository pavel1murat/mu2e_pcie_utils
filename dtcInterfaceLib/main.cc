#include "DTCLibTest.h"

#include <iostream>
#ifdef _WIN32
#include <chrono>
#include <thread>
#define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
#endif

void usage() {
    std::cout << "This program runs several functionality tests of libDTCInterface." << std::endl
        << "If run with no options, it will run all 6 tests." << std::endl
        << "Otherwise, it accepts a space-delimited list of the tests to run," << std::endl
        << "defined either by test number {1,2,3,4,5,6}, or test name {reg, pcie, stats, dcs, daq, loopback}" << std::endl
        << "It also accepts a -n argument indicating how many iterations of the tests it should run" << std::endl;
}

int main(int argc, char* argv[]) {
    int testCount = 1;
    bool registerTest = false,
        pcieTest = false,
        dmaStateTest = false,
        dcsTest = false,
        daqTest = false,
        loopbackTest = false;
    bool testsSpecified = false;

    if (argc == 1) {
        std::cout << "Running all DTC Tests." << std::endl << std::endl;
    }
    else {

        for (int i = 1; i < argc; ++i) {
            int firstChar = 0;
            if (argv[i][0] == '-') {
                firstChar = 1;
                if (argv[i][1] == 'n' && argc >= i + 1) {
                    ++i;
                    testCount = atoi(argv[i]);
                    continue;
                }
                else if (argv[i][1] == 'h' || argc == i + 1) {
                    usage();
                    exit(0);
                }
            }
            if (isdigit(argv[i][firstChar])) {
                testsSpecified = true;
                switch (argv[i][firstChar] - '0') {
                case 1:
                    registerTest = true;
                    break;
                case 2:
                    pcieTest = true;
                    break;
                case 3:
                    dmaStateTest = true;
                    break;
                case 4:
                    dcsTest = true;
                    break;
                case 5:
                    daqTest = true;
                    break;
                case 6:
                    loopbackTest = true;
                    break;
                }
            }
            else {
                std::string arg(argv[i]);
                arg = arg.substr(firstChar);
                if (arg.find("reg") != std::string::npos) {
                    testsSpecified = true;
                    registerTest = true;
                }
                else if (arg.find("pcie") != std::string::npos) {
                    testsSpecified = true;
                    pcieTest = true;
                }
                else if (arg.find("stats") != std::string::npos) {
                    dmaStateTest = true;
                    testsSpecified = true;
                }
                else if (arg.find("dcs") != std::string::npos) {
                    testsSpecified = true;
                    dcsTest = true;
                }
                else if (arg.find("daq") != std::string::npos || arg.find("dma") != std::string::npos) {
                    daqTest = true;
                    testsSpecified = true;
                }
                else if (arg.find("loopback") != std::string::npos) {
                    loopbackTest = true;
                    testsSpecified = true;
                }
                else {
                    usage();
                    exit(0);
                }
            }
        }
        if (!testsSpecified){
            registerTest = true;
            pcieTest = true;
            dmaStateTest = true;
            dcsTest = true;
            daqTest = true;
            loopbackTest = true;
        }

        std::cout << "Running tests: " << (registerTest ? "Register I/O " : "") << (pcieTest ? " PCIe State/Stats " : "")
            << (dmaStateTest ? " DMA State/Stats " : "") << (dcsTest ? " DCS DMA I/O " : "") << (daqTest ? " DAQ DMA I/O " : "") << (loopbackTest ? " DMA Loopback" : "")
            << ", " << testCount << " times." << std::endl;

    }

    DTC::DTCLibTest* tester = new DTC::DTCLibTest();

    tester->startTest(registerTest, pcieTest, dmaStateTest, dcsTest, daqTest,loopbackTest, testCount, true);

    while (tester->isRunning()) {
        usleep(500000);
    }
}
