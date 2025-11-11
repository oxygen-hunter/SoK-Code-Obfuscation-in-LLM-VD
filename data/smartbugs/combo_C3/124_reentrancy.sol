pragma solidity ^0.4.19;

contract PrivateBank {
    mapping (address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;

    enum InstructionSet { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL }

    struct VM {
        uint256[] stack;
        uint256 pc;
    }

    function PrivateBank(address _log) {
        TransferLog = Log(_log);
    }

    function executeVM(InstructionSet[] _instructions, uint256[] _operands) internal {
        VM memory vm;
        vm.stack = new uint256[](_operands.length);

        while (vm.pc < _instructions.length) {
            InstructionSet instr = _instructions[vm.pc];
            uint256 op = _operands[vm.pc];

            if (instr == InstructionSet.PUSH) {
                vm.stack.push(op);
            } else if (instr == InstructionSet.POP) {
                vm.stack.pop();
            } else if (instr == InstructionSet.ADD) {
                uint256 b = vm.stack.pop();
                uint256 a = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instr == InstructionSet.SUB) {
                uint256 b = vm.stack.pop();
                uint256 a = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (instr == InstructionSet.JMP) {
                vm.pc = op;
                continue;
            } else if (instr == InstructionSet.JZ) {
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = op;
                    continue;
                }
            } else if (instr == InstructionSet.LOAD) {
                vm.stack.push(balances[address(op)]);
            } else if (instr == InstructionSet.STORE) {
                balances[address(op)] = vm.stack.pop();
            } else if (instr == InstructionSet.CALL) {
                if (op == 0) {
                    TransferLog.AddMessage(address(vm.stack.pop()), vm.stack.pop(), "Deposit");
                } else if (op == 1) {
                    TransferLog.AddMessage(address(vm.stack.pop()), vm.stack.pop(), "CashOut");
                }
            }
            vm.pc++;
        }
    }

    function Deposit() public payable {
        InstructionSet[] memory instructions = new InstructionSet[](6);
        uint256[] memory operands = new uint256[](6);
        instructions[0] = InstructionSet.PUSH; operands[0] = msg.value;
        instructions[1] = InstructionSet.PUSH; operands[1] = MinDeposit;
        instructions[2] = InstructionSet.SUB;
        instructions[3] = InstructionSet.JZ; operands[3] = 5;
        instructions[4] = InstructionSet.RETURN;
        instructions[5] = InstructionSet.LOAD; operands[5] = uint256(msg.sender);
        instructions[6] = InstructionSet.ADD;
        instructions[7] = InstructionSet.STORE; operands[7] = uint256(msg.sender);
        instructions[8] = InstructionSet.PUSH; operands[8] = msg.value;
        instructions[9] = InstructionSet.PUSH; operands[9] = uint256(msg.sender);
        instructions[10] = InstructionSet.CALL; operands[10] = 0;

        executeVM(instructions, operands);
    }

    function CashOut(uint _am) public {
        InstructionSet[] memory instructions = new InstructionSet[](8);
        uint256[] memory operands = new uint256[](8);
        instructions[0] = InstructionSet.LOAD; operands[0] = uint256(msg.sender);
        instructions[1] = InstructionSet.PUSH; operands[1] = _am;
        instructions[2] = InstructionSet.SUB;
        instructions[3] = InstructionSet.JZ; operands[3] = 7;
        instructions[4] = InstructionSet.RETURN;
        instructions[5] = InstructionSet.PUSH; operands[5] = _am;
        instructions[6] = InstructionSet.CALL; operands[6] = 1;
        instructions[7] = InstructionSet.LOAD; operands[7] = uint256(msg.sender);
        instructions[8] = InstructionSet.SUB;
        instructions[9] = InstructionSet.STORE; operands[9] = uint256(msg.sender);
        instructions[10] = InstructionSet.PUSH; operands[10] = _am;
        instructions[11] = InstructionSet.PUSH; operands[11] = uint256(msg.sender);
        instructions[12] = InstructionSet.CALL; operands[12] = 1;

        executeVM(instructions, operands);
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