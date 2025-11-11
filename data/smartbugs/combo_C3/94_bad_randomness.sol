pragma solidity ^0.4.0;

contract Lottery {
    event GetBet(uint betAmount, uint blockNumber, bool won);

    struct Bet {
        uint betAmount;
        uint blockNumber;
        bool won;
    }

    address private organizer;
    Bet[] private bets;

    function Lottery() {
        organizer = msg.sender;
    }

    function() {
        throw;
    }

    function makeBet() {
        uint[] memory stack = new uint[](256);
        uint sp = 0;
        uint pc = 0;
        uint[] memory program = new uint[](10);
        
        program[0] = 1; // PUSH
        program[1] = msg.value;
        program[2] = 1; // PUSH
        program[3] = block.number;
        program[4] = 2; // MOD
        program[5] = 0; // PUSH
        program[6] = 2;
        program[7] = 3; // EQ
        program[8] = 4; // JZ
        program[9] = 14; // addr to jump

        while (pc < program.length) {
            uint opcode = program[pc];
            pc++;
            if (opcode == 0) { // PUSH
                stack[sp] = program[pc];
                sp++;
                pc++;
            } else if (opcode == 1) { // PUSH
                stack[sp] = program[pc];
                sp++;
                pc++;
            } else if (opcode == 2) { // MOD
                sp--;
                stack[sp-1] = stack[sp-1] % stack[sp];
            } else if (opcode == 3) { // EQ
                sp--;
                stack[sp-1] = (stack[sp-1] == stack[sp]) ? 1 : 0;
            } else if (opcode == 4) { // JZ
                if (stack[sp-1] == 0) {
                    pc = program[pc];
                } else {
                    pc++;
                }
                sp--;
            }
        }

        bool won = (stack[0] == 1);
        bets.push(Bet(msg.value, block.number, won));

        if (won) {
            if (!msg.sender.send(msg.value)) {
                throw;
            }
        }
    }

    function getBets() {
        if (msg.sender != organizer) { throw; }

        for (uint i = 0; i < bets.length; i++) {
            GetBet(bets[i].betAmount, bets[i].blockNumber, bets[i].won);
        }
    }

    function destroy() {
        if (msg.sender != organizer) { throw; }

        suicide(organizer);
    }
}