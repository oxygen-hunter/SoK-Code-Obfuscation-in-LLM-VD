pragma solidity ^0.4.19;

contract ETH_FUND {
    mapping (address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;
    uint lastBlock;

    enum Opcode { 
        PUSH, POP, ADD, SUB, CALL, CMP, JMP, JZ, LOAD, STORE, HALT 
    }

    struct Instruction {
        Opcode opcode;
        uint operand;
    }

    Instruction[] public program;
    uint[] stack;
    uint sp; // stack pointer
    uint pc; // program counter

    function ETH_FUND(address _log) public {
        TransferLog = Log(_log);
        sp = 0;
        pc = 0;
        populateProgram();
    }

    function populateProgram() internal {
        program.push(Instruction(Opcode.LOAD, uint(msg.sender))); // Load msg.sender
        program.push(Instruction(Opcode.PUSH, MinDeposit)); // Push MinDeposit
        program.push(Instruction(Opcode.CMP, 0)); // Compare msg.value with MinDeposit
        program.push(Instruction(Opcode.JZ, 8)); // Jump if not greater to end
        program.push(Instruction(Opcode.LOAD, uint(msg.value))); // Load msg.value
        program.push(Instruction(Opcode.ADD, 0)); // Add to balances[msg.sender]
        program.push(Instruction(Opcode.CALL, 0)); // Call TransferLog.AddMessage
        program.push(Instruction(Opcode.STORE, uint(block.number))); // Store lastBlock
        program.push(Instruction(Opcode.HALT, 0)); // End
    }

    function VM() internal {
        while (pc < program.length) {
            Instruction memory instr = program[pc];
            pc++;
            if (instr.opcode == Opcode.HALT) {
                break;
            } else if (instr.opcode == Opcode.PUSH) {
                stack.push(instr.operand);
            } else if (instr.opcode == Opcode.POP) {
                stack.length--;
            } else if (instr.opcode == Opcode.ADD) {
                uint a = stack[--sp];
                uint b = stack[--sp];
                stack.push(a + b);
                sp++;
            } else if (instr.opcode == Opcode.SUB) {
                uint a = stack[--sp];
                uint b = stack[--sp];
                stack.push(a - b);
                sp++;
            } else if (instr.opcode == Opcode.LOAD) {
                stack.push(instr.operand);
                sp++;
            } else if (instr.opcode == Opcode.STORE) {
                balances[msg.sender] = stack[--sp];
            } else if (instr.opcode == Opcode.CALL) {
                TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
            } else if (instr.opcode == Opcode.CMP) {
                uint a = stack[--sp];
                stack.push(a > stack[--sp] ? 1 : 0);
                sp++;
            } else if (instr.opcode == Opcode.JMP) {
                pc = instr.operand;
            } else if (instr.opcode == Opcode.JZ) {
                if (stack[--sp] == 0) {
                    pc = instr.operand;
                }
            }
        }
    }
    
    function Deposit() public payable {
        program.push(Instruction(Opcode.LOAD, uint(msg.value)));
        VM();
    }
    
    function CashOut(uint _am) public payable {
        if (_am <= balances[msg.sender] && block.number > lastBlock) {
            if (msg.sender.call.value(_am)()) {
                balances[msg.sender] -= _am;
                TransferLog.AddMessage(msg.sender, _am, "CashOut");
            }
        }
    }
    
    function() public payable {}

}

contract Log {
    struct Message {
        address Sender;
        string Data;
        uint Val;
        uint Time;
    }

    Message[] public History;
    Message LastMsg;

    function AddMessage(address _adr, uint _val, string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}