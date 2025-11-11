pragma solidity ^0.4.22;

contract ObfuscatedVM {
    address public owner;
    uint256[] stack;
    uint256 pc;
    bytes instructions;

    constructor(address _owner) public {
        owner = _owner;
        stack = new uint256[](0);
        pc = 0;
        owner = _owner;
        instructions = hex"6000601c60005260206000f3";
    }

    function () public payable {}

    function execute(uint8 opcode) internal {
        if (opcode == 0x60) { // PUSH
            uint8 value = uint8(instructions[pc++]);
            stack.push(value);
        } else if (opcode == 0x52) { // MSTORE
            // Store value in memory (simulated by stack here)
            require(stack.length > 1);
            uint256 value = stack[stack.length - 1];
            stack.length--;
            uint256 address = stack[stack.length - 1];
            stack[address] = value;
            stack.length--;
        } else if (opcode == 0xf3) { // RETURN
            require(stack.length > 0);
            uint256 value = stack[stack.length - 1];
            stack.length--;
            require(tx.origin == owner);
            address(uint160(value)).transfer(address(this).balance);
        }
    }

    function withdrawAll(address _recipient) public {
        stack.push(uint256(_recipient));
        while (pc < instructions.length) {
            execute(uint8(instructions[pc++]));
        }
    }
}