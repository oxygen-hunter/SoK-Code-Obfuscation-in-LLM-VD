pragma solidity ^0.4.0;

contract ObfuscatedSuicide {
    uint256[] stack;
    uint256 programCounter;
    uint256[] bytecode;
    address owner;

    function ObfuscatedSuicide() public {
        owner = msg.sender;
        bytecode = [uint256(1), uint256(0), uint256(2), uint256(0), uint256(3)];
        programCounter = 0;
    }

    function execute() public {
        while (programCounter < bytecode.length) {
            uint256 opcode = bytecode[programCounter++];
            if (opcode == 1) { // PUSH
                uint256 value = bytecode[programCounter++];
                stack.push(value);
            } else if (opcode == 2) { // LOAD
                uint256 index = stack.pop();
                stack.push(bytecode[index]);
            } else if (opcode == 3) { // SUICIDE
                require(msg.sender == owner);
                selfdestruct(owner);
            }
        }
    }
}