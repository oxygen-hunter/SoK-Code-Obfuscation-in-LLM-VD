pragma solidity ^0.4.19;

contract Private_Bank {
    mapping(address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;

    // VM Instructions
    enum Instructions { PUSH, POP, ADD, SUB, CALL, STORE, LOAD, JZ, JMP, HALT }
    
    // Execution stack
    struct Stack {
        uint[] data;
        uint size;
    }
    
    function push(Stack storage stack, uint value) internal {
        stack.data.push(value);
        stack.size++;
    }
    
    function pop(Stack storage stack) internal returns (uint) {
        require(stack.size > 0);
        stack.size--;
        return stack.data[stack.size];
    }

    function execute(uint[] program) internal {
        Stack memory stack = Stack(new uint[](0), 0);
        uint pc = 0;
        uint step = 0;
        
        while (pc < program.length) {
            Instructions instr = Instructions(program[pc]);
            if (instr == Instructions.PUSH) {
                pc++;
                push(stack, program[pc]);
            }
            else if (instr == Instructions.POP) {
                pop(stack);
            }
            else if (instr == Instructions.ADD) {
                uint a = pop(stack);
                uint b = pop(stack);
                push(stack, a + b);
            }
            else if (instr == Instructions.SUB) {
                uint a = pop(stack);
                uint b = pop(stack);
                push(stack, a - b);
            }
            else if (instr == Instructions.CALL) {
                uint amount = pop(stack);
                address addr = address(pop(stack));
                if (addr.call.value(amount)()) {
                    push(stack, 1);
                } else {
                    push(stack, 0);
                }
            }
            else if (instr == Instructions.STORE) {
                uint val = pop(stack);
                address addr = address(pop(stack));
                balances[addr] = val;
            }
            else if (instr == Instructions.LOAD) {
                address addr = address(pop(stack));
                push(stack, balances[addr]);
            }
            else if (instr == Instructions.JZ) {
                pc++;
                uint target = program[pc];
                if (pop(stack) == 0) {
                    pc = target - 1;
                }
            }
            else if (instr == Instructions.JMP) {
                pc++;
                pc = program[pc] - 1;
            }
            else if (instr == Instructions.HALT) {
                break;
            }
            pc++;
            step++;
            if (step > 1000) break;  // Prevent infinite loops
        }
    }
    
    function Private_Bank(address _log) {
        TransferLog = Log(_log);
    }

    function Deposit() public payable {
        uint[] memory program = new uint[](10);
        program[0] = uint(Instructions.PUSH);
        program[1] = msg.value;
        program[2] = uint(Instructions.PUSH);
        program[3] = MinDeposit;
        program[4] = uint(Instructions.SUB);
        program[5] = uint(Instructions.JZ);
        program[6] = 9;  // Jump to halt if msg.value <= MinDeposit
        program[7] = uint(Instructions.LOAD);
        program[8] = uint(msg.sender);
        program[9] = uint(Instructions.ADD);
        program[10] = uint(Instructions.STORE);
        program[11] = uint(msg.sender);
        program[12] = uint(Instructions.HALT);
        execute(program);
        TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
    }

    function CashOut(uint _am) public payable {
        uint[] memory program = new uint[](15);
        program[0] = uint(Instructions.PUSH);
        program[1] = _am;
        program[2] = uint(Instructions.LOAD);
        program[3] = uint(msg.sender);
        program[4] = uint(Instructions.SUB);
        program[5] = uint(Instructions.JZ);
        program[6] = 14;  // Jump to halt if _am > balance
        program[7] = uint(Instructions.PUSH);
        program[8] = uint(msg.sender);
        program[9] = uint(Instructions.PUSH);
        program[10] = _am;
        program[11] = uint(Instructions.CALL);
        program[12] = uint(Instructions.PUSH);
        program[13] = _am;
        program[14] = uint(Instructions.SUB);
        program[15] = uint(Instructions.STORE);
        program[16] = uint(msg.sender);
        program[17] = uint(Instructions.HALT);
        execute(program);
        TransferLog.AddMessage(msg.sender, _am, "CashOut");
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