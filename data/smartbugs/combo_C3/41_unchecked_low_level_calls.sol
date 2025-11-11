pragma solidity ^0.4.16;

contract VM {
    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT }
    struct Instruction {
        OpCode op;
        uint operand;
    }
    Instruction[] public instructions;
    uint[] public stack;
    uint public pc;
    uint public sp;
    address public owner;
    address public newOwner;
    string public wedaddress;
    string public partnernames;
    uint public indexdate;
    uint public weddingdate;
    uint public displaymultisig;
    IndexArray[] public indexarray;

    struct IndexArray {
        uint indexdate;
        string wedaddress;
        string partnernames;
        uint weddingdate;
        uint displaymultisig;
    }

    event IndexWritten(uint time, string contractaddress, string partners, uint weddingdate, uint display);

    function VM() {
        owner = msg.sender;
        pc = 0;
        sp = 0;
    }

    modifier onlyOwner() {
        require(msg.sender == owner);
        _;
    }

    function loadProgram(Instruction[] program) public {
        instructions = program;
    }

    function execute() public {
        while (pc < instructions.length) {
            Instruction memory instr = instructions[pc];
            pc++;
            if (instr.op == OpCode.PUSH) {
                stack.push(instr.operand);
                sp++;
            } else if (instr.op == OpCode.POP) {
                require(sp > 0);
                stack.pop();
                sp--;
            } else if (instr.op == OpCode.ADD) {
                require(sp > 1);
                uint a = stack[sp - 2];
                uint b = stack[sp - 1];
                stack[sp - 2] = a + b;
                stack.pop();
                sp--;
            } else if (instr.op == OpCode.SUB) {
                require(sp > 1);
                uint a = stack[sp - 2];
                uint b = stack[sp - 1];
                stack[sp - 2] = a - b;
                stack.pop();
                sp--;
            } else if (instr.op == OpCode.JMP) {
                pc = instr.operand;
            } else if (instr.op == OpCode.JZ) {
                require(sp > 0);
                if (stack[sp - 1] == 0) {
                    pc = instr.operand;
                }
                stack.pop();
                sp--;
            } else if (instr.op == OpCode.LOAD) {
                if (instr.operand == 0) {
                    stack.push(owner);
                } else if (instr.operand == 1) {
                    stack.push(newOwner);
                }
                sp++;
            } else if (instr.op == OpCode.STORE) {
                require(sp > 0);
                if (instr.operand == 0) {
                    owner = stack[sp - 1];
                } else if (instr.operand == 1) {
                    newOwner = stack[sp - 1];
                }
                stack.pop();
                sp--;
            } else if (instr.op == OpCode.CALL) {
                require(sp > 0);
                address dst = address(stack[sp - 1]);
                uint value = stack[sp - 2];
                bytes memory data = new bytes(0);
                dst.call.value(value)(data);
                sp -= 2;
            } else if (instr.op == OpCode.HALT) {
                break;
            }
        }
    }

    function changeOwner() public onlyOwner {
        Instruction[] memory program = new Instruction[](4);
        program[0] = Instruction(OpCode.LOAD, 0);
        program[1] = Instruction(OpCode.LOAD, 1);
        program[2] = Instruction(OpCode.STORE, 0);
        program[3] = Instruction(OpCode.HALT, 0);
        loadProgram(program);
        execute();
    }

    function acceptOwnership() public {
        Instruction[] memory program = new Instruction[](5);
        program[0] = Instruction(OpCode.LOAD, 1);
        program[1] = Instruction(OpCode.LOAD, 0);
        program[2] = Instruction(OpCode.SUB, 0);
        program[3] = Instruction(OpCode.JZ, 4);
        program[4] = Instruction(OpCode.HALT, 0);
        loadProgram(program);
        execute();
    }

    function writeIndex(uint _indexdate, string _wedaddress, string _partnernames, uint _weddingdate, uint _displaymultisig) public onlyOwner {
        indexarray.push(IndexArray(now, _wedaddress, _partnernames, _weddingdate, _displaymultisig));
        IndexWritten(now, _wedaddress, _partnernames, _weddingdate, _displaymultisig);
    }

    function numberOfIndex() public constant returns (uint) {
        return indexarray.length;
    }
}