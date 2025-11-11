pragma solidity ^0.4.24;

contract ObfuscatedMissing {
    address private _o;
    uint8 private _pc;
    uint256[] private _stack;
    bytes private _instructions;

    function ObfuscatedConstructor() public {
        _o = msg.sender;
        _instructions.push(0);  // LOAD
        _instructions.push(1);  // PUSH
        _instructions.push(2);  // ADD
        _instructions.push(3);  // STORE
        _instructions.push(4);  // JZ
        _instructions.push(5);  // JMP
        _instructions.push(6);  // PUSH
        _instructions.push(7);  // CALL
        _instructions.push(8);  // STOP
    }

    function () payable {}

    modifier onlyowner {
        executeVM();
        require(_stack[_stack.length - 1] == 1);
        _;
    }

    function withdraw() public onlyowner {
        executeVM();
        _o.transfer(this.balance);
    }

    function executeVM() internal {
        _pc = 0;
        while (_pc < _instructions.length) {
            uint8 instruction = _instructions[_pc];
            if (instruction == 0) {  // LOAD
                _stack.push(uint256(msg.sender));
                _pc++;
            } else if (instruction == 1) {  // PUSH
                _stack.push(uint256(_o));
                _pc++;
            } else if (instruction == 2) {  // ADD
                uint256 a = _stack[_stack.length - 1];
                _stack.pop();
                uint256 b = _stack[_stack.length - 1];
                _stack.pop();
                _stack.push(a + b);
                _pc++;
            } else if (instruction == 3) {  // STORE
                _o = address(_stack[_stack.length - 1]);
                _stack.pop();
                _pc++;
            } else if (instruction == 4) {  // JZ
                uint256 value = _stack[_stack.length - 1];
                _stack.pop();
                if (value == 0) {
                    _pc = uint8(_stack[_stack.length - 1]);
                    _stack.pop();
                } else {
                    _pc++;
                }
            } else if (instruction == 5) {  // JMP
                _pc = uint8(_stack[_stack.length - 1]);
                _stack.pop();
            } else if (instruction == 6) {  // PUSH
                _stack.push(1);
                _pc++;
            } else if (instruction == 7) {  // CALL
                // Simply an example, no actual function call
                _pc++;
            } else if (instruction == 8) {  // STOP
                break;
            }
        }
    }
}