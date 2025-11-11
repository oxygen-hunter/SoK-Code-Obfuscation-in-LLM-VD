pragma solidity ^0.4.25; 
contract demo {
    enum Instruction { PUSH, POP, ADD, CALL, LOAD, STORE, JZ, JMP, HALT }
    uint256[] stack;
    uint256 programCounter;
    address from;
    address caddress;
    address[] _tos;
    uint256[] v;
    bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));

    function transfer(address _from, address _caddress, address[] __tos, uint[] _v) public returns (bool) {
        stack.length = 0;
        programCounter = 0;
        from = _from;
        caddress = _caddress;
        _tos = __tos;
        v = _v;

        Instruction[] memory program = new Instruction[](10);
        uint256[] memory operands = new uint256[](10);

        program[0] = Instruction.LOAD; operands[0] = 3; // Load _tos.length
        program[1] = Instruction.PUSH; operands[1] = 0; // Push 0
        program[2] = Instruction.JZ; operands[2] = 9;   // Jump to HALT if _tos.length == 0
        program[3] = Instruction.PUSH; operands[3] = 0; // Initialize i = 0

        program[4] = Instruction.CALL;                  // caddress.call(id, from, _tos[i], v[i])
        program[5] = Instruction.ADD;                   // i++
        program[6] = Instruction.LOAD; operands[6] = 3; // Load _tos.length
        program[7] = Instruction.JMP; operands[7] = 4;  // Loop back if i < _tos.length

        program[8] = Instruction.PUSH; operands[8] = 1; // Push true
        program[9] = Instruction.HALT;                  // Halt execution

        while (programCounter < program.length) {
            executeInstruction(program[programCounter], operands[programCounter]);
            programCounter++;
        }

        return stack[stack.length - 1] != 0;
    }

    function executeInstruction(Instruction instr, uint256 operand) internal {
        if (instr == Instruction.PUSH) {
            stack.push(operand);
        } else if (instr == Instruction.POP) {
            stack.length--;
        } else if (instr == Instruction.ADD) {
            uint256 a = stack[stack.length - 1];
            stack.length--;
            stack[stack.length - 1] += a;
        } else if (instr == Instruction.CALL) {
            uint i = stack[stack.length - 1];
            stack.length--;
            caddress.call(id, from, _tos[i], v[i]);
        } else if (instr == Instruction.LOAD) {
            if (operand == 3) stack.push(_tos.length);
        } else if (instr == Instruction.JZ) {
            if (stack[stack.length - 1] == 0) programCounter = operand - 1;
            stack.length--;
        } else if (instr == Instruction.JMP) {
            programCounter = operand - 1;
        }
    }
}