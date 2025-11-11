pragma solidity ^0.4.19;

contract DEP_BANK {
    mapping(address => uint256) private balances;
    uint private MinSum;
    LogFile private Log;
    bool private intitalized;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, EXIT }

    struct VM {
        uint[] stack;
        uint pc;
        bool running;
    }

    function execute(VM memory vm, uint[] memory code) internal {
        vm.running = true;
        while (vm.running) {
            Opcode op = Opcode(code[vm.pc]);
            if (op == Opcode.PUSH) {
                vm.stack.push(code[++vm.pc]);
            } else if (op == Opcode.POP) {
                vm.stack.length--;
            } else if (op == Opcode.ADD) {
                uint b = vm.stack[vm.stack.length - 1];
                uint a = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a + b;
                vm.stack.length--;
            } else if (op == Opcode.SUB) {
                uint b = vm.stack[vm.stack.length - 1];
                uint a = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a - b;
                vm.stack.length--;
            } else if (op == Opcode.JMP) {
                vm.pc = code[++vm.pc] - 1;
            } else if (op == Opcode.JZ) {
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = code[++vm.pc] - 1;
                } else {
                    vm.pc++;
                }
                vm.stack.length--;
            } else if (op == Opcode.LOAD) {
                vm.stack.push(balances[address(code[++vm.pc])]);
            } else if (op == Opcode.STORE) {
                balances[address(code[++vm.pc])] = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
            } else if (op == Opcode.CALL) {
                address target = address(code[++vm.pc]);
                uint value = code[++vm.pc];
                target.call.value(value)();
            } else if (op == Opcode.EXIT) {
                vm.running = false;
            }
            vm.pc++;
        }
    }

    function SetMinSum(uint _val) public {
        if (intitalized) return;
        execute(VM(new uint[](0), 0, true), new uint[](5) {uint(Opcode.PUSH), _val, uint(Opcode.STORE), uint(address(this)), uint(Opcode.EXIT)});
    }

    function SetLogFile(address _log) public {
        if (intitalized) return;
        Log = LogFile(_log);
    }

    function Initialized() public {
        intitalized = true;
    }

    function Deposit() public payable {
        execute(VM(new uint[](0), 0, true), new uint[](7) {
            uint(Opcode.PUSH), msg.value,
            uint(Opcode.ADD), uint(Opcode.LOAD), uint(address(msg.sender)),
            uint(Opcode.STORE), uint(address(msg.sender)), 
            uint(Opcode.EXIT)
        });
        Log.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am) public payable {
        uint[] memory code = new uint[](15);
        code[0] = uint(Opcode.LOAD);
        code[1] = uint(address(msg.sender));
        code[2] = uint(Opcode.PUSH);
        code[3] = MinSum;
        code[4] = uint(Opcode.SUB);
        code[5] = uint(Opcode.JZ);
        code[6] = 14;
        code[7] = uint(Opcode.LOAD);
        code[8] = uint(address(msg.sender));
        code[9] = uint(Opcode.PUSH);
        code[10] = _am;
        code[11] = uint(Opcode.SUB);
        code[12] = uint(Opcode.JZ);
        code[13] = 14;
        code[14] = uint(Opcode.CALL);
        code[15] = uint(msg.sender);
        code[16] = _am;
        code[17] = uint(Opcode.PUSH);
        code[18] = _am;
        code[19] = uint(Opcode.SUB);
        code[20] = uint(Opcode.STORE);
        code[21] = uint(address(msg.sender));
        code[22] = uint(Opcode.EXIT);
        execute(VM(new uint[](0), 0, true), code);
        Log.AddMessage(msg.sender, _am, "Collect");
    }

    function() public payable {
        Deposit();
    }
}

contract LogFile {
    struct Message {
        address Sender;
        string Data;
        uint Val;
        uint Time;
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