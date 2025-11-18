#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <string.h>


enum class Mode { Memory, CLI, Debug };


class LMC {
public:
    LMC() {
        setupOpcodes();
    }
    void load_program(std::string file_url) {
        std::string line;
        std::ifstream file;
        file.open(file_url);
        if (file.is_open()) {
            while (getline(file, line)) {
                // TODO stoi error handling
                int _line = stoi(line);
                memory_.push_back(_line);
            }
            file.close();
        }
    }

    // Core loop
    void loop(Mode mode) {
        while (true) {
            std::string instruction;

            if (mode == Mode::CLI) {
                std::cout << "Instruction: ";
                std::cin >> instruction;
            } else {
                instruction = std::to_string(memory_.at(counter_));
            }

            if (mode == Mode::Debug) {
                // Pauses execution
                mode = debug();
            }

            // Opcode: operation to perform
            int opcode = instruction[0] - '0';

            if (opcode == HALT) {
                std::cout << "Halting the program" << std::endl;
                return;
            }

            // Address: where to find the data
            // TODO stoi error checking
            int address = std::stoi(instruction.substr(1,2));

            // Prepare next instruction before the actual execution
            counter_++;
            if (counter_ > MAX_MEMORY && mode != Mode::CLI) {
                std::cout << "Counter overflow error" << std::endl;
                return;
            }

            if (opcode == 9) {
                // Executes for IN & OUT
                opcodes_[std::stoi(instruction)](address);
            } else {
                // Executes for the rest
                opcodes_[opcode](address);
            }
        }
    };

    Mode debug() {
        while (true) {
            std::cout << "Memory" << std::endl;
            for (int i = 0; i < memory_.size(); i++) {
                if (i == counter_) {
                    std::cout << "> " << memory_.at(i) << std::endl;
                } else {
                    std::cout << "  " << memory_.at(i) << std::endl;
                }
            }
            std::cout << std::endl;

            std::cout << "Counter: " << counter_ << std::endl;
            std::cout << "Accumulator: " << accumulator_ << std::endl;
            
            for (int i : input_) {
                std::cout << "Input: " << i << std::endl;
            }
            for (int i : output_) {
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

    // Getters and setters
    std::vector<int> output() const {
        return output_;
    }
    void setinput(std::vector<int> value) {
        input_ = value;
    }

private:    
    static const int HALT = 0;
    static const int MAX_MEMORY = 100;

    // Instructions and data are saved in Memory
    std::vector<int> memory_;

    // Working bench
    int accumulator_ = 0;

    // Next instruction address
    int counter_ = 0;

    // Array of input values. Read left to right
    std::vector<int> input_;

    // Array of output values. Output order left to right.
    std::vector<int> output_;

    // Instructions map: opcode -> function
    // std::unordered_map<int, void (LMC::*)(int)> opcodes_;
    std::unordered_map<int, std::function<void(int)> > opcodes_;
    
    // Add the value in mailbox to value in the accumulator
    void add(int addr) {
        accumulator_ += memory_.at(addr);
    };
    // Subtract the value in mailbox from value in the accumulator
    void sub(int addr) {
        accumulator_ -= memory_.at(addr);
    };
    // Store the value in the accumulator to the mailbox address
    void sto(int addr) {
        memory_[addr] = accumulator_;
    };
    // Load value from mailbox to accumulator
    void lda(int addr) {
        accumulator_ = memory_.at(addr);
    };
    // Branch for next instruction
    void bra(int addr) {
        counter_ = addr;
    };
    // Branch for next instruction if accumulator is <= 0
    void brz(int addr) {
        if (accumulator_ <= 0) {
            counter_ = addr;
        }
    };
    // Branch for next instruction if accumulator is not 0
    void brp(int addr) {
        if (accumulator_ != 0) {
            counter_ = addr;
        }
    };
    // Fetch next value in Inputs to the accumulator
    void inp(int addr) {
        // TODO change to a std::queue or deque
        if (input_.size()) {
            accumulator_ = input_[0];
            input_.erase(input_.begin());
        }
    };
    // Concat value in accumulator to output
    void outp(int addr) {
        output_.push_back(accumulator_);
    };
    // Constructs the Opcodes map
    void setupOpcodes() {
        opcodes_[1]   = [this](int addr) { this->add(addr);  };
        opcodes_[2]   = [this](int addr) { this->sub(addr);  };
        opcodes_[3]   = [this](int addr) { this->sto(addr);  };
        opcodes_[5]   = [this](int addr) { this->lda(addr);  };
        opcodes_[6]   = [this](int addr) { this->bra(addr);  };
        opcodes_[7]   = [this](int addr) { this->brz(addr);  };
        opcodes_[8]   = [this](int addr) { this->brp(addr);  };
        opcodes_[901] = [this](int addr) { this->inp(addr);  };
        opcodes_[902] = [this](int addr) { this->outp(addr); };
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
    LMC lmc = LMC();

    Args args;
    parse_args(argc, argv, args);

    lmc.setinput(args.inputs);
    lmc.load_program(args.file_url);
    lmc.loop(args.mode);

    for (int i : lmc.output()) {
        std::cout << "Output " << i << std::endl;
    }

    return 0;
}
