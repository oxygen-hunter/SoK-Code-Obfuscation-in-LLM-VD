pragma solidity ^0.4.19;

contract ETH_VAULT {
    mapping (address => uint) private balances;
    
    Log private TransferLog;
    
    uint private MinDeposit = 1 ether;

    uint[] private stack;
    uint private pc;
    bytes private instructions;
    
    function ETH_VAULT(address _log) public {
        TransferLog = Log(_log);
    }
    
    function Deposit() public payable {
        instructions = new bytes(7);
        instructions[0] = byte(0x01); // LOAD msg.value
        instructions[1] = byte(0x02); // PUSH MinDeposit
        instructions[2] = byte(0x03); // CMP GT
        instructions[3] = byte(0x04); // JZ end
        instructions[4] = byte(0x05); // ADD msg.value to balances
        instructions[5] = byte(0x06); // LOG "Deposit"
        instructions[6] = byte(0x07); // END
        
        pc = 0;
        execute();
    }
    
    function CashOut(uint _am) public payable {
        instructions = new bytes(9);
        instructions[0] = byte(0x01); // LOAD _am
        instructions[1] = byte(0x08); // CMP LE balances[msg.sender]
        instructions[2] = byte(0x04); // JZ end
        instructions[3] = byte(0x09); // CALL _am
        instructions[4] = byte(0x04); // JZ end
        instructions[5] = byte(0x0A); // SUB _am from balances
        instructions[6] = byte(0x0B); // LOG "CashOut"
        instructions[7] = byte(0x07); // END
        instructions[8] = byte(0x07); // END
        
        pc = 0;
        execute();
    }
    
    function() public payable {}

    function execute() private {
        while (pc < instructions.length) {
            uint8 opcode = uint8(instructions[pc]);
            if (opcode == 0x01) { // LOAD
                stack.push(msg.value);
                pc++;
            } else if (opcode == 0x02) { // PUSH MinDeposit
                stack.push(MinDeposit);
                pc++;
            } else if (opcode == 0x03) { // CMP GT
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a > b ? 1 : 0);
                pc++;
            } else if (opcode == 0x04) { // JZ
                uint cond = stack.pop();
                if (cond == 0) {
                    pc = instructions.length;
                } else {
                    pc++;
                }
            } else if (opcode == 0x05) { // ADD
                balances[msg.sender] += msg.value;
                pc++;
            } else if (opcode == 0x06) { // LOG "Deposit"
                TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
                pc++;
            } else if (opcode == 0x07) { // END
                break;
            } else if (opcode == 0x08) { // CMP LE
                uint am = stack.pop();
                stack.push(am <= balances[msg.sender] ? 1 : 0);
                pc++;
            } else if (opcode == 0x09) { // CALL
                if (!msg.sender.call.value(stack.pop())()) {
                    stack.push(0);
                } else {
                    stack.push(1);
                }
                pc++;
            } else if (opcode == 0x0A) { // SUB
                balances[msg.sender] -= stack.pop();
                pc++;
            } else if (opcode == 0x0B) { // LOG "CashOut"
                TransferLog.AddMessage(msg.sender, stack.pop(), "CashOut");
                pc++;
            }
        }
    }
}

contract Log {
    struct Message {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] public History;
    
    Message private LastMsg;
    
    function AddMessage(address _adr, uint _val, string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}