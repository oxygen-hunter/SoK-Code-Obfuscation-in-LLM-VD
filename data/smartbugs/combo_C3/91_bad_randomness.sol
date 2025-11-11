pragma solidity ^0.4.24;

contract ObfuscatedPredictTheBlockHashChallenge {

    struct guess {
        uint block;
        bytes32 guess;
    }

    mapping(address => guess) guesses;

    enum Instruction { NOP, PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET }

    struct VM {
        uint[] stack;
        uint pc;
    }

    constructor() public payable {
        require(msg.value == 1 ether);
        execute(new uint256[](0), new uint8[](0));
    }

    function lockInGuess(bytes32 hash) public payable {
        uint8[] memory bytecode = new uint8[](12);
        bytecode[0] = uint8(Instruction.LOAD);
        bytecode[1] = uint8(Instruction.PUSH);
        bytecode[2] = uint8(Instruction.PUSH);
        bytecode[3] = uint8(Instruction.JZ);
        bytecode[4] = uint8(Instruction.LOAD);
        bytecode[5] = uint8(Instruction.PUSH);
        bytecode[6] = uint8(Instruction.JZ);
        bytecode[7] = uint8(Instruction.STORE);
        bytecode[8] = uint8(Instruction.ADD);
        bytecode[9] = uint8(Instruction.RET);
        bytecode[10] = uint8(Instruction.RET);
        bytecode[11] = uint8(Instruction.RET);

        uint256[] memory constants = new uint256[](4);
        constants[0] = uint256(guesses[msg.sender].block);
        constants[1] = 0;
        constants[2] = uint256(msg.value);
        constants[3] = 1 ether;

        execute(constants, bytecode);
        guesses[msg.sender].guess = hash;
        guesses[msg.sender].block = block.number + 1;
    }

    function settle() public {
        uint8[] memory bytecode = new uint8[](10);
        bytecode[0] = uint8(Instruction.LOAD);
        bytecode[1] = uint8(Instruction.PUSH);
        bytecode[2] = uint8(Instruction.JZ);
        bytecode[3] = uint8(Instruction.CALL);
        bytecode[4] = uint8(Instruction.STORE);
        bytecode[5] = uint8(Instruction.LOAD);
        bytecode[6] = uint8(Instruction.PUSH);
        bytecode[7] = uint8(Instruction.JZ);
        bytecode[8] = uint8(Instruction.RET);
        bytecode[9] = uint8(Instruction.RET);

        uint256[] memory constants = new uint256[](3);
        constants[0] = block.number;
        constants[1] = guesses[msg.sender].block;
        constants[2] = 2 ether;

        execute(constants, bytecode);
    }

    function execute(uint256[] memory constants, uint8[] memory bytecode) internal {
        VM memory vm;
        vm.stack = new uint[](256);
        vm.pc = 0;

        while (vm.pc < bytecode.length) {
            Instruction instr = Instruction(bytecode[vm.pc]);

            if (instr == Instruction.PUSH) {
                vm.pc++;
                vm.stack.push(constants[bytecode[vm.pc]]);
            } else if (instr == Instruction.POP) {
                vm.stack.pop();
            } else if (instr == Instruction.ADD) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instr == Instruction.SUB) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (instr == Instruction.JMP) {
                vm.pc = vm.stack.pop();
                continue;
            } else if (instr == Instruction.JZ) {
                uint target = vm.stack.pop();
                if (vm.stack.pop() == 0) {
                    vm.pc = target;
                    continue;
                }
            } else if (instr == Instruction.LOAD) {
                vm.stack.push(uint256(guesses[msg.sender].block));
            } else if (instr == Instruction.STORE) {
                guesses[msg.sender].block = vm.stack.pop();
            } else if (instr == Instruction.CALL) {
                bytes32 answer = blockhash(uint256(guesses[msg.sender].block));
                if (guesses[msg.sender].guess == answer) {
                    msg.sender.transfer(2 ether);
                }
            } else if (instr == Instruction.RET) {
                return;
            }
            vm.pc++;
        }
    }
}