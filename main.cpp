#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <string.h>


// TODO dont use global variables
// TODO dont use magic numbers -- static const int

// Instructions and data are saved in Memory
std::vector<int> Memory;
// Working bench
int Accumulator = 0;
// Next instruction address
int Counter = 0;
// Array of input values. Read left to right
std::vector<int> Input;
// Array of output values. Output order left to right.
std::vector<int> Output;

// Instructions map: opcode -> function
std::unordered_map<int, std::function<void(int)> > Opcodes;
// Add the value in mailbox to value in the accumulator
void add(int addr) {
    Accumulator += Memory.at(addr);
};
// Subtract the value in mailbox from value in the accumulator
void sub(int addr) {
    Accumulator -= Memory.at(addr);
};
// Store the value in the accumulator to the mailbox address
void sto(int addr) {
    Memory[addr] = Accumulator;
};
// Load value from mailbox to accumulator
void lda(int addr) {
    Accumulator = Memory.at(addr);
};
// Branch for next instruction
void bra(int addr) {
    Counter = addr;
};
// Branch for next instruction if accumulator is <= 0
void brz(int addr) {
    if (Accumulator <= 0) {
        Counter = addr;
    }
};
// Branch for next instruction if accumulator is not 0
void brp(int addr) {
    if (Accumulator != 0) {
        Counter = addr;
    }
};
// Fetch next value in Inputs to the accumulator
void inp(int addr) {
    // TODO change to a std::queue or deque
    if (Input.size()) {
        Accumulator = Input[0];
        Input.erase(Input.begin());
    }
};
// Concat value in accumulator to output
void outp(int addr) {
    Output.push_back(Accumulator);
};
// Constructs the Opcodes map
void setup() {
    Opcodes[1] = &add;
    Opcodes[2] = &sub;
    Opcodes[3] = &sto;
    Opcodes[5] = &lda;
    Opcodes[6] = &bra;
    Opcodes[7] = &brz;
    Opcodes[8] = &brp;
    Opcodes[901] = &inp;
    Opcodes[902] = &outp;
}

enum class Mode { Memory, CLI, Debug };


Mode debug() {
    while (true) {
        std::cout << "Memory" << std::endl;
        for (int i = 0; i < Memory.size(); i++) {
            if (i == Counter) {
                std::cout << "> " << Memory.at(i) << std::endl;
            } else {
                std::cout << "  " << Memory.at(i) << std::endl;
            }
        }
        std::cout << std::endl;

        std::cout << "Counter: " << Counter << std::endl;
        std::cout << "Accumulator: " << Accumulator << std::endl;
        
        for (int i : Input) {
            std::cout << "Input: " << i << std::endl;
        }
        for (int i : Output) {
            std::cout << "Output: " << i << std::endl;
        }
        
        std::cout << std::endl;
        std::cout << "Debugger stopped the program" << std::endl;
        std::cout << "> ";
        std::string cmd;
        std::cin >> cmd;

        if (cmd == "s") {  // Step
            return Mode::Debug;

        } else if (cmd == "c") {  // Continue
            return Mode::Memory;

        } else if (cmd == "q") {  // Quit
            exit(1);

        } else if (cmd == "h") {  // Help
            std::cout << "s Step to next command" << std::endl;
            std::cout << "c: Continue execution, disable debug" << std::endl;
            std::cout << "q: Quit the program" << std::endl;
            std::cout << "h: Show this help message" << std::endl;
        };
    };
};

// Core loop
void loop(Mode mode) {
    // TODO separate parsing and execution of opcodes
    while (true) {
        std::string instruction;

        if (mode == Mode::CLI) {
            std::cout << "Instruction: ";
            std::cin >> instruction;
        } else {
            instruction = std::to_string(Memory.at(Counter));
        }

        if (mode == Mode::Debug) {
            mode = debug();
        }

        // Opcode: operation to perform
        int opcode = instruction[0] - '0';

        if (opcode == 0) {  // TODO test 999
            std::cout << "Halting the program" << std::endl;
            return;
        }

        // Address: where to find the data
        // TODO stoi error checking
        int address = std::stoi(instruction.substr(1,2));

        // Prepare next instruction before the actual execution
        Counter++;
        if (Counter > 100 && mode != Mode::CLI) {
            std::cout << "Counter overflow error" << std::endl;
            return;
        }

        if (opcode == 9) {
            // Executes for IN & OUT
            // TODO stoi error checking
            Opcodes[std::stoi(instruction)](address);
        } else {
            // Executes for the rest
            Opcodes[opcode](address);
        }
    }
};


void load_program(std::string file_url) {
    std::string line;
    std::ifstream file;
    file.open(file_url);
    if (file.is_open()) {
        while (getline(file, line)) {
            // TODO stoi error handling
            int _line = stoi(line);
            Memory.push_back(_line);
        }
        file.close();
    }
};

struct Args {
    Mode mode = Mode::Memory;
    std::string file_url;
    std::vector<int> inputs;
};

void parse_args(int argc, char** argv, Args& args) {
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-cli") == 0) {
            args.mode = Mode::CLI;

        } else if (strcmp(argv[i], "-file") == 0) {
            if (i+1 < argc) {
                args.file_url = argv[i+1];
            } else {
                std::cerr << "Error: -file flag requires filename" << std::endl;
            }

        } else if (strcmp(argv[i], "-i") == 0) {
            int j = 1;
            while (i+j < argc && argv[i+j][0] != '-') {
                // TODO stoi error checking
                int _inp = std::stoi(argv[i+j]);
                args.inputs.push_back(_inp);
                j++;
            }
            i += (j-1);

        } else if (strcmp(argv[i], "-debug") == 0) {
            args.mode = Mode::Debug;
        }
    }
}

int main(int argc, char** argv) {
    setup();
    Args args;
    parse_args(argc, argv, args);

    Input = args.inputs;
    load_program(args.file_url);
    loop(args.mode);

    for (int i : Output) {
        std::cout << "Output " << i << std::endl;
    }

    return 0;
}
