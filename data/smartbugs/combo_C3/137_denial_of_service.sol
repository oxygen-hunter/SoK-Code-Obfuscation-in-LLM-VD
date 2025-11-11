pragma solidity ^0.4.25;

contract DosOneFuncVM {
    address[] listAddresses;

    function executeVM(bytes memory bytecode) internal returns (bool) {
        uint8[] memory stack = new uint8[](1024);
        uint256 sp = 0; 
        uint256 pc = 0; 
        uint256 codeLength = bytecode.length;

        while (pc < codeLength) {
            uint8 opcode = uint8(bytecode[pc]);
            pc++;
            if (opcode == 0) { 
                stack[sp] = uint8(listAddresses.length);
                sp++;
            } else if (opcode == 1) { 
                stack[sp] = 1500;
                sp++;
            } else if (opcode == 2) { 
                sp--;
                sp--;
                stack[sp] = stack[sp] < stack[sp + 1] ? 1 : 0;
                sp++;
            } else if (opcode == 3) { 
                sp--;
                if (stack[sp] == 0) {
                    pc += 2;
                }
            } else if (opcode == 4) { 
                stack[sp] = 350;
                sp++;
            } else if (opcode == 5) { 
                stack[sp] = 0;
                sp++;
            } else if (opcode == 6) { 
                sp--;
                uint8 count = stack[sp];
                for (uint8 i = 0; i < count; i++) {
                    listAddresses.push(msg.sender);
                }
            } else if (opcode == 7) { 
                return true;
            } else if (opcode == 8) { 
                listAddresses = new address[](0);
            } else if (opcode == 9) { 
                return false;
            } else if (opcode == 10) { 
                uint8 offset = uint8(bytecode[pc]);
                pc += offset;
            }
        }
        return false;
    }

    function ifillArray() public returns (bool) {
        bytes memory bytecode = new bytes(11);
        bytecode[0] = 0; 
        bytecode[1] = 1; 
        bytecode[2] = 2; 
        bytecode[3] = 3; 
        bytecode[4] = 4; 
        bytecode[5] = 6; 
        bytecode[6] = 7; 
        bytecode[7] = 5; 
        bytecode[8] = 8; 
        bytecode[9] = 9; 
        bytecode[10] = 10; 

        return executeVM(bytecode);
    }
}