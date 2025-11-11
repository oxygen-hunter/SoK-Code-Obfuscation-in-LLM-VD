pragma solidity ^0.4.18;

contract VMExecutor {
    uint256[] stack;
    uint256 programCounter;
    address owner;
    address newOwner;
    address msgSender;
    mapping (address => uint) Holders;
    uint MinDeposit;

    enum Instructions { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALLDATA, SENDER, BALANCE, CALL, HALT }

    function VMExecutor() public {
        owner = msg.sender;
        MinDeposit = 1 ether;
    }

    function execute(uint256[] program, address _msgSender, address _addr, address _token, uint _amount, uint _wei) public payable {
        programCounter = 0;
        msgSender = _msgSender;
        while (programCounter < program.length) {
            Instructions instruction = Instructions(program[programCounter]);
            programCounter++;
            runInstruction(instruction, program, _addr, _token, _amount, _wei);
        }
    }

    function runInstruction(Instructions instruction, uint256[] program, address _addr, address _token, uint _amount, uint _wei) internal {
        if (instruction == Instructions.PUSH) {
            stack.push(program[programCounter++]);
        } else if (instruction == Instructions.POP) {
            stack.length--;
        } else if (instruction == Instructions.ADD) {
            uint256 b = stack[stack.length - 1];
            uint256 a = stack[stack.length - 2];
            stack[stack.length - 2] = a + b;
            stack.length--;
        } else if (instruction == Instructions.SUB) {
            uint256 b = stack[stack.length - 1];
            uint256 a = stack[stack.length - 2];
            stack[stack.length - 2] = a - b;
            stack.length--;
        } else if (instruction == Instructions.JMP) {
            programCounter = stack[stack.length - 1];
            stack.length--;
        } else if (instruction == Instructions.JZ) {
            uint256 addr = stack[stack.length - 1];
            stack.length--;
            if (stack[stack.length - 1] == 0) {
                programCounter = addr;
            }
            stack.length--;
        } else if (instruction == Instructions.LOAD) {
            uint256 index = stack[stack.length - 1];
            stack[stack.length - 1] = Holders[address(index)];
        } else if (instruction == Instructions.STORE) {
            uint256 index = stack[stack.length - 1];
            uint256 value = stack[stack.length - 2];
            Holders[address(index)] = value;
            stack.length -= 2;
        } else if (instruction == Instructions.CALLDATA) {
            stack.push(program[programCounter++]);
        } else if (instruction == Instructions.SENDER) {
            stack.push(uint256(msgSender));
        } else if (instruction == Instructions.BALANCE) {
            stack.push(address(this).balance);
        } else if (instruction == Instructions.CALL) {
            address dest = address(stack[stack.length - 3]);
            uint256 amount = stack[stack.length - 2];
            bytes4 signature = bytes4(stack[stack.length - 1]);
            dest.call(signature, amount);
            stack.length -= 3;
        } else if (instruction == Instructions.HALT) {
            return;
        }
    }

    function Deposit(uint256[] program, address _msgSender) public payable {
        execute(program, _msgSender, address(0), address(0), 0, 0);
    }

    function WitdrawTokenToHolder(uint256[] program, address _msgSender, address _to, address _token, uint _amount) public {
        execute(program, _msgSender, _to, _token, _amount, 0);
    }

    function WithdrawToHolder(uint256[] program, address _msgSender, address _addr, uint _wei) public payable {
        execute(program, _msgSender, _addr, address(0), 0, _wei);
    }

    function Bal(uint256[] program, address _msgSender) public view returns(uint) {
        uint256[] memory stackCopy = stack;
        execute(program, _msgSender, address(0), address(0), 0, 0);
        stack = stackCopy;
        return stack[stack.length - 1];
    }
}