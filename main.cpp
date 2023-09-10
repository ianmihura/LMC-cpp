#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <any>
#include <functional>


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
    Accumulator += Memory[addr];
};
// Subtract the value in mailbox from value in the accumulator
void sub(int addr) {
    Accumulator -= Memory[addr];
};
// Store the value in the accumulator to the mailbox address
void sto(int addr) {
    Memory[addr] = Accumulator;
};
// Load value from mailbox to accumulator
void lda(int addr) {
    Accumulator = Memory[addr];
};
// Branch for next instruction
void bra(int addr) {
    Counter = addr;
};
// Branch for next instruction if accumulator is <= 0
void brz(int addr) {
    if (Accumulator <= 0)
        Counter = addr;
};
// Branch for next instruction if accumulator is not 0
void brp(int addr) {
    if (Accumulator != 0)
        Counter = addr;
};
// Fetch next value in Inputs to the accumulator
void inp(int addr) {
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

void fillup_memory() {
    for (int i = 0; i <= 100; i++)
        if (Memory.size() < i)
            Memory.push_back(0);
};

enum Mode { ModeMemory, ModeCLI, ModeDebug };


Mode debug() {
    std::cout << "Memory" << std::endl;
    for (int i = 0; i < Memory.size(); i++)
        if (i == Counter)
            std::cout << "> " << Memory[i] << std::endl;
        else
            std::cout << "  " << Memory[i] << std::endl;
    std::cout << std::endl;

    std::cout << "Counter: " << Counter << std::endl;
    std::cout << "Accumulator: " << Accumulator << std::endl;
    
    for (int i : Input)
        std::cout << "Input: " << i << std::endl;
    for (int i : Output)
        std::cout << "Output: " << i << std::endl;
    
    std::cout << std::endl;
    std::cout << "Debugger stopped the program" << std::endl;
    std::cout << "> ";
    std::string cmd;
    std::cin >> cmd;

    if (cmd == "s") {  // Step
        return ModeDebug;

    } else if (cmd == "c") {  // Continue
        return ModeMemory;

    } else if (cmd == "q") {  // Quit
        exit(1);

    } else if (cmd == "h") {  // Help
        std::cout << "s Step to next command" << std::endl;
        std::cout << "c: Continue execution, disable debug" << std::endl;
        std::cout << "q: Quit the program" << std::endl;
        std::cout << "h: Show this help message" << std::endl;
    };

    return debug();
};

// Core loop
void loop(Mode mode) {
    { // Scope to delete local variables on loop
        std::string instruction;

        if (mode == ModeCLI) {
            std::cout << "Instruction: ";
            std::cin >> instruction;
        } else {
            instruction = std::to_string(Memory[Counter]);
        }

        if (mode == ModeDebug) {
            mode = debug();
        }

        // Opcode: operation to perform
        int opcode = instruction[0] - '0';

        if (opcode == 0) {
            std::cout << "Halting the program" << std::endl;
            return;
        }

        // Address: where to find the data
        int address = std::stoi(instruction.substr(1,2));

        // Prepare next instruction before the actual execution
        Counter++;
        if (Counter > 100 && mode != ModeCLI) {
            std::cout << "Counter overflow error" << std::endl;
            return;
        }

        if (opcode == 9) {
            // Executes for IN & OUT
            Opcodes[std::stoi(instruction)](address);
        } else {
            // Executes for the rest
            Opcodes[opcode](address);
        }
    }

    // Next loop
    loop(mode);
};


void load_program(std::string file_url) {
    std::string line;
    std::ifstream file;
    file.open(file_url);
    if (file.is_open()) {
        while (getline(file, line)) {
            Memory.push_back(stoi(line));
        }
        file.close();
    }
};

// lmc -cli
// lmc -file mult.lmc -i 1 2
// lmc -file mult.lmc -i 1 2 -debug

struct Args {
    Mode mode = ModeMemory;
    std::string file_url;
    std::vector<int> inputs;
};

void parse_args(int argc, char** argv, Args& args) {
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-cli") == 0) {
            args.mode = ModeCLI;

        } else if (strcmp(argv[i], "-file") == 0) {
            args.file_url = argv[i+1];

        } else if (strcmp(argv[i], "-i") == 0) {
            int j = 1;
            while (argv[i+j][0] != '-') {
                args.inputs.push_back(std::stoi(argv[i+j]));
                j++;
            }

        } else if (strcmp(argv[i], "-debug") == 0) {
            args.mode = ModeDebug;
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

    for (int i : Output)
        std::cout << "Output " << i << std::endl;

    return 0;
}
