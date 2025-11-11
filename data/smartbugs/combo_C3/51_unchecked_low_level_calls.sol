pragma solidity ^0.4.24;

contract VM {
    enum Instructions {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, TRANSFER
    }

    struct Stack {
        uint[] data;
    }

    struct Program {
        uint[] code;
        uint pc;
    }

    function execute(Program memory program, Stack memory stack) internal {
        while (program.pc < program.code.length) {
            Instructions instruction = Instructions(program.code[program.pc]);
            program.pc++;
            if (instruction == Instructions.PUSH) {
                uint value = program.code[program.pc];
                program.pc++;
                stack.data.push(value);
            } else if (instruction == Instructions.POP) {
                require(stack.data.length > 0);
                stack.data.pop();
            } else if (instruction == Instructions.ADD) {
                require(stack.data.length > 1);
                uint b = stack.data[stack.data.length - 1];
                stack.data.pop();
                uint a = stack.data[stack.data.length - 1];
                stack.data.pop();
                stack.data.push(a + b);
            } else if (instruction == Instructions.SUB) {
                require(stack.data.length > 1);
                uint b = stack.data[stack.data.length - 1];
                stack.data.pop();
                uint a = stack.data[stack.data.length - 1];
                stack.data.pop();
                stack.data.push(a - b);
            } else if (instruction == Instructions.JMP) {
                uint target = program.code[program.pc];
                program.pc = target;
            } else if (instruction == Instructions.JZ) {
                uint target = program.code[program.pc];
                program.pc++;
                uint value = stack.data[stack.data.length - 1];
                stack.data.pop();
                if (value == 0) {
                    program.pc = target;
                }
            } else if (instruction == Instructions.LOAD) {
                uint index = program.code[program.pc];
                program.pc++;
                stack.data.push(stack.data[index]);
            } else if (instruction == Instructions.STORE) {
                uint index = program.code[program.pc];
                program.pc++;
                stack.data[index] = stack.data[stack.data.length - 1];
            } else if (instruction == Instructions.CALL) {
                address target = address(stack.data[stack.data.length - 1]);
                stack.data.pop();
                uint value = stack.data[stack.data.length - 1];
                stack.data.pop();
                bytes memory data = abi.encodePacked(stack.data[stack.data.length - 1]);
                stack.data.pop();
                target.call.value(value)(data);
            } else if (instruction == Instructions.TRANSFER) {
                address recipient = address(stack.data[stack.data.length - 1]);
                stack.data.pop();
                uint amount = stack.data[stack.data.length - 1];
                stack.data.pop();
                recipient.transfer(amount);
            }
        }
    }
}

contract SimpleWallet is VM {
    address public owner = msg.sender;
    uint public depositsCount;

    modifier onlyOwner {
        require(msg.sender == owner);
        _;
    }

    function() public payable {
        depositsCount++;
    }

    function withdrawAll() public onlyOwner {
        Stack memory stack;
        Program memory program = Program({
            code: new uint[](5),
            pc: 0
        });
        program.code[0] = uint(Instructions.PUSH);
        program.code[1] = uint(address(this).balance);
        program.code[2] = uint(Instructions.TRANSFER);
        program.code[3] = uint(Instructions.PUSH);
        program.code[4] = uint(msg.sender);
        execute(program, stack);
    }

    function withdraw(uint _value) public onlyOwner {
        Stack memory stack;
        Program memory program = Program({
            code: new uint[](4),
            pc: 0
        });
        program.code[0] = uint(Instructions.PUSH);
        program.code[1] = _value;
        program.code[2] = uint(Instructions.TRANSFER);
        program.code[3] = uint(Instructions.PUSH);
        program.code[4] = uint(msg.sender);
        execute(program, stack);
    }

    function sendMoney(address _target, uint _value, bytes _data) public onlyOwner {
        Stack memory stack;
        Program memory program = Program({
            code: new uint[](7),
            pc: 0
        });
        program.code[0] = uint(Instructions.PUSH);
        program.code[1] = uint(_target);
        program.code[2] = uint(Instructions.PUSH);
        program.code[3] = _value;
        program.code[4] = uint(Instructions.PUSH);
        program.code[5] = uint(keccak256(_data));
        program.code[6] = uint(Instructions.CALL);
        execute(program, stack);
    }
}