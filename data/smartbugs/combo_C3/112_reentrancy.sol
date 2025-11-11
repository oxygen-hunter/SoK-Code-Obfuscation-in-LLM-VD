pragma solidity ^0.4.19;

contract ObfuscatedVM {
    mapping (address => uint) private _credit;
    uint private _balance;
    uint private _pc;
    uint[] private _stack;

    function execute(uint[] program) private {
        _pc = 0;
        while (_pc < program.length) {
            uint instruction = program[_pc];
            if (instruction == 1) { // PUSH
                _pc++;
                _stack.push(program[_pc]);
            } else if (instruction == 2) { // POP
                _stack.length--;
            } else if (instruction == 3) { // ADD
                uint b = _stack[_stack.length - 1];
                uint a = _stack[_stack.length - 2];
                _stack[_stack.length - 2] = a + b;
                _stack.length--;
            } else if (instruction == 4) { // SUB
                uint b = _stack[_stack.length - 1];
                uint a = _stack[_stack.length - 2];
                _stack[_stack.length - 2] = a - b;
                _stack.length--;
            } else if (instruction == 5) { // JMP
                _pc = _stack[_stack.length - 1] - 1;
                _stack.length--;
            } else if (instruction == 6) { // JZ
                if (_stack[_stack.length - 2] == 0) {
                    _pc = _stack[_stack.length - 1] - 1;
                }
                _stack.length -= 2;
            } else if (instruction == 7) { // LOAD
                _stack.push(_credit[msg.sender]);
            } else if (instruction == 8) { // STORE
                _credit[msg.sender] = _stack[_stack.length - 1];
                _stack.length--;
            } else if (instruction == 9) { // LOADBAL
                _stack.push(_balance);
            } else if (instruction == 10) { // STOREBAL
                _balance = _stack[_stack.length - 1];
                _stack.length--;
            } else if (instruction == 11) { // CALL
                uint value = _stack[_stack.length - 1];
                bool callResult = msg.sender.call.value(value)();
                require(callResult);
                _stack.length--;
            }
            _pc++;
        }
    }

    function withdrawAll() public {
        execute([7, 1, 0, 6, 7, 1, _balance, 4, 10, 11, 8]);
    }

    function deposit() public payable {
        execute([1, msg.value, 7, 3, 8, 1, msg.value, 9, 3, 10]);
    }
}