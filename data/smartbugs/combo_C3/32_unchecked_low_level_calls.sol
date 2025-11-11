pragma solidity ^0.4.0;

contract LottoVM {

    uint constant public blocksPerRound = 6800;
    uint constant public ticketPrice = 100000000000000000;
    uint constant public blockReward = 5000000000000000000;

    struct Round {
        address[] buyers;
        uint pot;
        uint ticketsCount;
        mapping(uint=>bool) isCashed;
        mapping(address=>uint) ticketsCountByBuyer;
    }
    mapping(uint => Round) rounds;

    enum Instruction {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
    }

    struct VMState {
        uint[] stack;
        uint pc;
    }

    function execute(uint[] instructions) internal returns (uint) {
        VMState memory state;
        state.stack = new uint[](256);
        state.pc = 0;
        uint instruction;
        while (state.pc < instructions.length) {
            instruction = instructions[state.pc++];
            if (instruction == uint(Instruction.PUSH)) {
                state.stack.push(instructions[state.pc++]);
            } else if (instruction == uint(Instruction.POP)) {
                state.stack.pop();
            } else if (instruction == uint(Instruction.ADD)) {
                uint b = state.stack.pop();
                uint a = state.stack.pop();
                state.stack.push(a + b);
            } else if (instruction == uint(Instruction.SUB)) {
                uint b = state.stack.pop();
                uint a = state.stack.pop();
                state.stack.push(a - b);
            } else if (instruction == uint(Instruction.JMP)) {
                state.pc = state.stack.pop();
            } else if (instruction == uint(Instruction.JZ)) {
                uint target = state.stack.pop();
                if (state.stack.pop() == 0) {
                    state.pc = target;
                }
            } else if (instruction == uint(Instruction.LOAD)) {
                uint roundIndex = state.stack.pop();
                uint subpotIndex = state.stack.pop();
                state.stack.push(rounds[roundIndex].isCashed[subpotIndex] ? 1 : 0);
            } else if (instruction == uint(Instruction.STORE)) {
                uint roundIndex = state.stack.pop();
                uint subpotIndex = state.stack.pop();
                rounds[roundIndex].isCashed[subpotIndex] = (state.stack.pop() == 1);
            } else if (instruction == uint(Instruction.CALL)) {
                uint funcId = state.stack.pop();
                if (funcId == 1) { // getDecisionBlockNumber
                    uint roundIndex = state.stack.pop();
                    uint subpotIndex = state.stack.pop();
                    state.stack.push(((roundIndex + 1) * blocksPerRound) + subpotIndex);
                } else if (funcId == 2) { // getHashOfBlock
                    uint blockIndex = state.stack.pop();
                    state.stack.push(uint(block.blockhash(blockIndex)));
                } else if (funcId == 3) { // getRoundIndex
                    state.stack.push(block.number / blocksPerRound);
                }
            } else if (instruction == uint(Instruction.RET)) {
                break;
            }
        }
        return state.stack.length > 0 ? state.stack[state.stack.length - 1] : 0;
    }

    function getBlocksPerRound() constant returns(uint) {
        uint[] memory instructions = new uint[](3);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = blocksPerRound;
        instructions[2] = uint(Instruction.RET);
        return execute(instructions);
    }

    function getTicketPrice() constant returns(uint) {
        uint[] memory instructions = new uint[](3);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = ticketPrice;
        instructions[2] = uint(Instruction.RET);
        return execute(instructions);
    }

    function getRoundIndex() constant returns (uint) {
        uint[] memory instructions = new uint[](2);
        instructions[0] = uint(Instruction.CALL);
        instructions[1] = 3; // getRoundIndex
        return execute(instructions);
    }

    function getIsCashed(uint roundIndex, uint subpotIndex) constant returns (bool) {
        uint[] memory instructions = new uint[](5);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = roundIndex;
        instructions[2] = uint(Instruction.PUSH);
        instructions[3] = subpotIndex;
        instructions[4] = uint(Instruction.LOAD);
        return execute(instructions) == 1;
    }

    function calculateWinner(uint roundIndex, uint subpotIndex) constant returns(address) {
        uint decisionBlockNumber;
        uint decisionBlockHash;
        uint winningTicketIndex;
        uint ticketIndex = 0;
        uint[] memory instructions = new uint[](10);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = roundIndex;
        instructions[2] = uint(Instruction.PUSH);
        instructions[3] = subpotIndex;
        instructions[4] = uint(Instruction.CALL);
        instructions[5] = 1; // getDecisionBlockNumber
        instructions[6] = uint(Instruction.RET);

        decisionBlockNumber = execute(instructions);
        if (decisionBlockNumber > block.number) return;

        instructions = new uint[](4);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = decisionBlockNumber;
        instructions[2] = uint(Instruction.CALL);
        instructions[3] = 2; // getHashOfBlock

        decisionBlockHash = execute(instructions);
        winningTicketIndex = decisionBlockHash % rounds[roundIndex].ticketsCount;

        for (uint buyerIndex = 0; buyerIndex < rounds[roundIndex].buyers.length; buyerIndex++) {
            address buyer = rounds[roundIndex].buyers[buyerIndex];
            ticketIndex += rounds[roundIndex].ticketsCountByBuyer[buyer];
            if (ticketIndex > winningTicketIndex) {
                return buyer;
            }
        }
    }

    function cash(uint roundIndex, uint subpotIndex) {
        uint subpotsCount;
        uint decisionBlockNumber;
        uint subpot;
        address winner;

        uint[] memory instructions = new uint[](8);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = roundIndex;
        instructions[2] = uint(Instruction.CALL);
        instructions[3] = 1; // getSubpotsCount
        instructions[4] = uint(Instruction.RET);
        subpotsCount = execute(instructions);

        if (subpotIndex >= subpotsCount) return;

        instructions = new uint[](10);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = roundIndex;
        instructions[2] = uint(Instruction.PUSH);
        instructions[3] = subpotIndex;
        instructions[4] = uint(Instruction.CALL);
        instructions[5] = 1; // getDecisionBlockNumber
        instructions[6] = uint(Instruction.RET);
        decisionBlockNumber = execute(instructions);

        if (decisionBlockNumber > block.number) return;

        instructions = new uint[](6);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = roundIndex;
        instructions[2] = uint(Instruction.PUSH);
        instructions[3] = subpotIndex;
        instructions[4] = uint(Instruction.LOAD);
        instructions[5] = uint(Instruction.RET);
        if (execute(instructions) == 1) return;

        winner = calculateWinner(roundIndex, subpotIndex);

        instructions = new uint[](8);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = roundIndex;
        instructions[2] = uint(Instruction.CALL);
        instructions[3] = 1; // getSubpot
        instructions[4] = uint(Instruction.RET);
        subpot = execute(instructions);

        winner.send(subpot);

        instructions = new uint[](6);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = 1;
        instructions[2] = uint(Instruction.PUSH);
        instructions[3] = subpotIndex;
        instructions[4] = uint(Instruction.STORE);
        instructions[5] = uint(Instruction.RET);
        execute(instructions);
    }

    function() payable {
        uint roundIndex;
        uint value;
        uint ticketsCount;

        uint[] memory instructions = new uint[](2);
        instructions[0] = uint(Instruction.CALL);
        instructions[1] = 3; // getRoundIndex
        roundIndex = execute(instructions);

        value = msg.value - (msg.value % ticketPrice);
        if (value == 0) return;

        if (value < msg.value) {
            msg.sender.send(msg.value - value);
        }

        ticketsCount = value / ticketPrice;
        rounds[roundIndex].ticketsCount += ticketsCount;

        if (rounds[roundIndex].ticketsCountByBuyer[msg.sender] == 0) {
            uint buyersLength = rounds[roundIndex].buyers.length++;
            rounds[roundIndex].buyers[buyersLength] = msg.sender;
        }

        rounds[roundIndex].ticketsCountByBuyer[msg.sender] += ticketsCount;
        rounds[roundIndex].ticketsCount += ticketsCount;
        rounds[roundIndex].pot += value;
    }
}