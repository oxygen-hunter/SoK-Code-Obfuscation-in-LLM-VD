pragma solidity ^0.4.24;

contract MyContract {
    address owner;

    function MyContract() public {
        owner = msg.sender;
    }

    function runVM(bytes instructions) internal {
        uint pc = 0;
        uint stackPointer = 0;
        address stackAddress;
        uint stackAmount;
        bool running = true;

        while (running) {
            uint8 opcode = uint8(instructions[pc++]);

            if (opcode == 0x01) { // LOAD_OWNER
                stackAddress = owner;
            } else if (opcode == 0x02) { // LOAD_SENDER
                stackAddress = msg.sender;
            } else if (opcode == 0x03) { // LOAD_ORIGIN
                stackAddress = tx.origin;
            } else if (opcode == 0x04) { // LOAD_ARG1
                stackAmount = uint(stackPointer);
            } else if (opcode == 0x05) { // LOAD_ARG2
                stackAddress = address(stackPointer);
            } else if (opcode == 0x06) { // CHECK_EQUAL
                running = (stackAddress == owner);
            } else if (opcode == 0x07) { // TRANSFER
                stackAddress.transfer(stackAmount);
            } else if (opcode == 0x08) { // HALT
                running = false;
            }
        }
    }

    function sendTo(address receiver, uint amount) public {
        bytes memory instructions = new bytes(8);
        instructions[0] = byte(0x03); // LOAD_ORIGIN
        instructions[1] = byte(0x01); // LOAD_OWNER
        instructions[2] = byte(0x06); // CHECK_EQUAL
        instructions[3] = byte(0x05); // LOAD_ARG1
        instructions[4] = byte(0x04); // LOAD_ARG2
        instructions[5] = byte(0x07); // TRANSFER
        instructions[6] = byte(0x08); // HALT

        stackPointer = uint(receiver);
        runVM(instructions);
    }
}