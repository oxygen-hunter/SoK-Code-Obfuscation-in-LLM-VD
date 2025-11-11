pragma solidity 0.4.25;

contract Overflow_Add_VM {
    uint public balance = 1;

    function execute(uint8[] program, uint256[] data) public {
        uint256 pointer = 0;
        uint256 stackPointer = 0;
        uint256[10] memory stack;
        uint256 dataPointer = 0;

        while (pointer < program.length) {
            uint8 opcode = program[pointer];

            if (opcode == 0) { // PUSH
                stack[stackPointer] = data[dataPointer];
                stackPointer++;
                dataPointer++;
            } else if (opcode == 1) { // POP
                require(stackPointer > 0);
                stackPointer--;
            } else if (opcode == 2) { // ADD
                require(stackPointer > 1);
                stack[stackPointer - 2] = stack[stackPointer - 2] + stack[stackPointer - 1];
                stackPointer--;
            } else if (opcode == 3) { // LOAD_BALANCE
                stack[stackPointer] = balance;
                stackPointer++;
            } else if (opcode == 4) { // STORE_BALANCE
                require(stackPointer > 0);
                balance = stack[stackPointer - 1];
                stackPointer--;
            } else if (opcode == 5) { // HALT
                break;
            } else {
                revert("Invalid opcode");
            }

            pointer++;
        }
    }
}