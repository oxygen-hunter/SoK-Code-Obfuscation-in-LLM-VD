pragma solidity ^0.4.18;

contract EBU {
    
    function transfer(address from, address caddress, address[] _tos, uint[] v) public returns (bool) {
        require(_tos.length > 0);
        
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        
        uint[] memory stack = new uint[](256);
        uint sp = 0;
        uint pc = 0;
        uint[] memory instructions = new uint[](256);
        
        // Compilation of logic into VM instructions
        instructions[0] = 1; // PUSH _tos.length
        instructions[1] = uint(_tos.length);
        instructions[2] = 5; // JZ end
        instructions[3] = 20; // End label position
        instructions[4] = 1; // PUSH id
        instructions[5] = uint(id);
        instructions[6] = 1; // PUSH from
        instructions[7] = uint(from);
        instructions[8] = 1; // PUSH caddress
        instructions[9] = uint(caddress);
        instructions[10] = 1; // PUSH _tos
        instructions[11] = uint(_tos.length);
        instructions[12] = 1; // PUSH v
        instructions[13] = uint(v.length);
        instructions[14] = 7; // CALL caddress.call
        instructions[15] = 0; // No operation required
        instructions[16] = 1; // PUSH 1
        instructions[17] = 1;
        instructions[18] = 3; // ADD sp
        instructions[19] = 4; // JMP start
        instructions[20] = 4; // JMP to end
        
        bool success = false;
        
        while (pc < instructions.length) {
            uint instr = instructions[pc];
            if (instr == 1) { // PUSH
                stack[sp++] = instructions[++pc];
            } else if (instr == 3) { // ADD
                uint a = stack[--sp];
                uint b = stack[--sp];
                stack[sp++] = a + b;
            } else if (instr == 4) { // JMP
                pc = instructions[++pc] - 1;
            } else if (instr == 5) { // JZ
                uint check = stack[--sp];
                if (check == 0) {
                    pc = instructions[++pc] - 1;
                } else {
                    ++pc;
                }
            } else if (instr == 7) { // CALL
                caddress.call(id, from, _tos[stack[--sp]], v[stack[--sp]]);
            }
            pc++;
        }
        
        success = true;
        return success;
    }
}