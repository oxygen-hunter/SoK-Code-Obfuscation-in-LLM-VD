pragma solidity ^0.4.2;

contract VirtualMachine {
    
    enum Instructions { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET }
    
    uint256[] stack;
    mapping(uint256 => uint256) memoryStorage;
    uint256 pc = 0;
    uint256[] program;
    bool halt;

    address owner;
    Player[2] players;
    uint8 tot;

    struct Player {
        address addr;
        uint number;
    }
    
    function VirtualMachine() {
        owner = msg.sender;
    }

    function execute(uint256[] initProgram) internal {
        program = initProgram;
        halt = false;
        while (!halt) {
            uint256 instr = program[pc++];
            if (instr == uint256(Instructions.PUSH)) {
                stack.push(program[pc++]);
            } else if (instr == uint256(Instructions.POP)) {
                stack.pop();
            } else if (instr == uint256(Instructions.ADD)) {
                uint256 b = stack.pop();
                uint256 a = stack.pop();
                stack.push(a + b);
            } else if (instr == uint256(Instructions.SUB)) {
                uint256 b = stack.pop();
                uint256 a = stack.pop();
                stack.push(a - b);
            } else if (instr == uint256(Instructions.JMP)) {
                pc = stack.pop();
            } else if (instr == uint256(Instructions.JZ)) {
                uint256 addr = stack.pop();
                uint256 condition = stack.pop();
                if (condition == 0) pc = addr;
            } else if (instr == uint256(Instructions.LOAD)) {
                uint256 addr = stack.pop();
                stack.push(memoryStorage[addr]);
            } else if (instr == uint256(Instructions.STORE)) {
                uint256 addr = stack.pop();
                uint256 value = stack.pop();
                memoryStorage[addr] = value;
            } else if (instr == uint256(Instructions.CALL)) {
                program[pc++];
            } else if (instr == uint256(Instructions.RET)) {
                halt = true;
            }
        }
    }

    function play(uint number) payable {
        if (msg.value != 1 ether) throw;
        players[tot] = Player(msg.sender, number);
        tot++;
        if (tot == 2) {
            uint256[] memory bytecode = new uint256[](14);
            bytecode[0] = uint256(Instructions.PUSH);
            bytecode[1] = uint256(players[0].number);
            bytecode[2] = uint256(Instructions.PUSH);
            bytecode[3] = uint256(players[1].number);
            bytecode[4] = uint256(Instructions.ADD);
            bytecode[5] = uint256(Instructions.PUSH);
            bytecode[6] = 2;
            bytecode[7] = uint256(Instructions.SUB);
            bytecode[8] = uint256(Instructions.JZ);
            bytecode[9] = 12;
            bytecode[10] = uint256(Instructions.PUSH);
            bytecode[11] = uint256(players[0].addr);
            bytecode[12] = uint256(Instructions.CALL);
            bytecode[13] = uint256(Instructions.RET);
            execute(bytecode);
            delete players;
            tot = 0;
        }
    }

    function getProfit() {
        if(msg.sender != owner) throw;
        bool res = msg.sender.send(this.balance);
    }
}