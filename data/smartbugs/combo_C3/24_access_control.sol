pragma solidity ^0.4.15;

contract ObfuscatedVM {
    address private owner;

    uint8 constant PUSH = 0;
    uint8 constant POP = 1;
    uint8 constant LOAD = 2;
    uint8 constant STORE = 3;
    uint8 constant JZ = 4;
    uint8 constant JMP = 5;
    uint8 constant CALL = 6;
    uint8 constant RET = 7;

    uint256[] stack;
    uint256[] memorySpace;
    uint256 pc;
    bool running;

    function ObfuscatedVM() public {
        owner = msg.sender;
        memorySpace = new uint256[](256);
        stack = new uint256[](256);
        pc = 0;
        running = true;
        execute();
    }

    function execute() internal {
        while (running) {
            uint8 instruction = getNextInstruction();
            if (instruction == PUSH) {
                uint256 value = getNextInstruction();
                stack.push(value);
            } else if (instruction == POP) {
                stack.pop();
            } else if (instruction == LOAD) {
                uint256 addr = stack.pop();
                stack.push(memorySpace[addr]);
            } else if (instruction == STORE) {
                uint256 addr = stack.pop();
                uint256 value = stack.pop();
                memorySpace[addr] = value;
            } else if (instruction == JZ) {
                uint256 addr = getNextInstruction();
                if (stack.pop() == 0) {
                    pc = addr;
                }
            } else if (instruction == JMP) {
                uint256 addr = getNextInstruction();
                pc = addr;
            } else if (instruction == CALL) {
                uint256 addr = getNextInstruction();
                stack.push(pc);
                pc = addr;
            } else if (instruction == RET) {
                pc = stack.pop();
            } else {
                running = false;
            }
        }
    }

    function getNextInstruction() internal returns (uint8) {
        return uint8(memorySpace[pc++]);
    }

    function changeOwner(address _newOwner) public {
        runVMForChangeOwner(_newOwner);
    }

    function runVMForChangeOwner(address _newOwner) internal {
        memorySpace[0] = uint256(PUSH);
        memorySpace[1] = uint256(_newOwner);
        memorySpace[2] = uint256(STORE);
        memorySpace[3] = uint256(0); // store at memory[0]
        memorySpace[4] = uint256(CALL);
        memorySpace[5] = uint256(8); // call address 8
        memorySpace[6] = uint256(JMP);
        memorySpace[7] = uint256(14); // jump to end (halt)
        memorySpace[8] = uint256(PUSH);
        memorySpace[9] = uint256(msg.sender);
        memorySpace[10] = uint256(LOAD);
        memorySpace[11] = uint256(0); // load owner
        memorySpace[12] = uint256(JZ);
        memorySpace[13] = uint256(0); // jump to halt if equal
        memorySpace[14] = uint256(LOAD);
        memorySpace[15] = uint256(0); // load new owner
        memorySpace[16] = uint256(STORE);
        memorySpace[17] = uint256(1); // store in owner

        pc = 0;
        running = true;
        execute();
        owner = address(memorySpace[1]);
    }
}