pragma solidity ^0.4.11;  

contract Owned {
    function Owned() {
        owner = msg.sender;
    }

    address public owner;

    modifier onlyOwner { if (msg.sender == owner) _; }

    function changeOwner(address _newOwner) onlyOwner {
        owner = _newOwner;
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }
}

contract Token {
    function transfer(address, uint) returns(bool);
    function balanceOf(address) constant returns (uint);
}

contract TokenSender is Owned {
    Token public token;  
    uint public totalToDistribute;
    uint public next;

    struct Transfer {
        address addr;
        uint amount;
    }

    Transfer[] public transfers;

    function TokenSender(address _token) {
        token = Token(_token);
    }

    uint constant D160 = 0x0010000000000000000000000000000000000000000;

    enum Opcode { PUSH, ADD, MUL, DIV, MOD, LT, GT, EQ, AND, OR, NOT, JUMP, JUMPI, LOAD, STORE, BALANCE, CALL, GAS, THROW, HALT }
    
    struct Instruction {
        Opcode opcode;
        uint operand;
    }
    
    Instruction[] public instructions;
    uint public pc;
    uint[] public stack;

    function executeVM() internal {
        while (pc < instructions.length) {
            Instruction memory instr = instructions[pc];
            pc++;
            if (instr.opcode == Opcode.HALT) {
                break;
            } else if (instr.opcode == Opcode.PUSH) {
                stack.push(instr.operand);
            } else if (instr.opcode == Opcode.ADD) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a + b);
            } else if (instr.opcode == Opcode.MUL) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a * b);
            } else if (instr.opcode == Opcode.DIV) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a / b);
            } else if (instr.opcode == Opcode.MOD) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a % b);
            } else if (instr.opcode == Opcode.LT) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a < b ? 1 : 0);
            } else if (instr.opcode == Opcode.GT) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a > b ? 1 : 0);
            } else if (instr.opcode == Opcode.EQ) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a == b ? 1 : 0);
            } else if (instr.opcode == Opcode.AND) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a & b);
            } else if (instr.opcode == Opcode.OR) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a | b);
            } else if (instr.opcode == Opcode.NOT) {
                uint a = stack.pop();
                stack.push(~a);
            } else if (instr.opcode == Opcode.JUMP) {
                pc = instr.operand;
            } else if (instr.opcode == Opcode.JUMPI) {
                uint cond = stack.pop();
                if (cond != 0) {
                    pc = instr.operand;
                }
            } else if (instr.opcode == Opcode.LOAD) {
                stack.push(stack[instr.operand]);
            } else if (instr.opcode == Opcode.STORE) {
                stack[instr.operand] = stack.pop();
            } else if (instr.opcode == Opcode.BALANCE) {
                stack.push(token.balanceOf(this));
            } else if (instr.opcode == Opcode.CALL) {
                uint amount = stack.pop();
                address addr = address(stack.pop());
                if (!token.transfer(addr, amount)) {
                    revert();
                }
            } else if (instr.opcode == Opcode.GAS) {
                stack.push(gas());
            } else if (instr.opcode == Opcode.THROW) {
                revert();
            }
        }
    }

    function loadInstructions(uint[] ops, uint[] operands) onlyOwner {
        require(ops.length == operands.length);
        for (uint i = 0; i < ops.length; i++) {
            instructions.push(Instruction(Opcode(ops[i]), operands[i]));
        }
    }

    function runVM() onlyOwner {
        executeVM();
    }

    function fill(uint[] data) onlyOwner {
        if (next>0) revert();
        uint acc;
        uint offset = transfers.length;
        transfers.length = transfers.length + data.length;
        for (uint i = 0; i < data.length; i++ ) {
            address addr = address( data[i] & (D160-1) );
            uint amount = data[i] / D160;
            transfers[offset + i].addr = addr;
            transfers[offset + i].amount = amount;
            acc += amount;
        }
        totalToDistribute += acc;
    }
    
    function gas() internal constant returns (uint _gas) {
        assembly {
            _gas:= gas
        }
    }

    function hasTerminated() constant returns (bool) {
        if (transfers.length == 0) return false;
        if (next < transfers.length) return false;
        return true;
    }

    function nTransfers() constant returns (uint) {
        return transfers.length;
    }
}