pragma solidity ^0.4.21;

contract ObfuscatedChallenge {
    uint8 private answer;

    function ObfuscatedChallenge() public payable {
        require(msg.value == 1 ether);
        answer = uint8(keccak256(block.blockhash(block.number - 1), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == 0;
    }

    function guess(uint8 n) public payable {
        require(msg.value == 1 ether);
        
        uint8[] memory code = new uint8[](10);
        uint256[] memory stack = new uint256[](2);
        uint8 pc = 0;
        uint8 sp = 0;

        code[0] = 1; // LOAD
        code[1] = 0; // answer
        
        code[2] = 1; // LOAD
        code[3] = 1; // n

        code[4] = 5; // SUB

        code[5] = 6; // JZ
        code[6] = 9; // to pc 9

        code[7] = 7; // JMP
        code[8] = 10; // to end

        code[9] = 8; // TRANSFER

        while (pc < code.length) {
            uint8 instr = code[pc];
            pc++;

            if (instr == 1) { // LOAD
                stack[sp] = instr == 0 ? answer : n;
                sp++;
            } else if (instr == 5) { // SUB
                sp--;
                stack[sp - 1] -= stack[sp];
            } else if (instr == 6) { // JZ
                sp--;
                if (stack[sp] == 0) {
                    pc = code[pc];
                } else {
                    pc++;
                }
            } else if (instr == 7) { // JMP
                pc = code[pc];
            } else if (instr == 8) { // TRANSFER
                msg.sender.transfer(2 ether);
            }
        }
    }
}