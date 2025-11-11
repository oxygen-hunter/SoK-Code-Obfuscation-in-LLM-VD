pragma solidity ^0.4.0;

contract KingOfTheEtherThroneVM {

    struct Monarch {
        address etherAddress;
        string name;
        uint claimPrice;
        uint coronationTimestamp;
    }

    address wizardAddress;
    uint constant startingClaimPrice = 100 finney;
    uint constant claimPriceAdjustNum = 3;
    uint constant claimPriceAdjustDen = 2;
    uint constant wizardCommissionFractionNum = 1;
    uint constant wizardCommissionFractionDen = 100;
    uint public currentClaimPrice;
    Monarch public currentMonarch;
    Monarch[] public pastMonarchs;

    uint8 private constant PUSH = 0;
    uint8 private constant POP = 1;
    uint8 private constant ADD = 2;
    uint8 private constant SUB = 3;
    uint8 private constant MUL = 4;
    uint8 private constant DIV = 5;
    uint8 private constant JMP = 6;
    uint8 private constant JZ = 7;
    uint8 private constant LOAD = 8;
    uint8 private constant STORE = 9;
    uint8 private constant CALL = 10;

    modifier onlywizard { if (msg.sender == wizardAddress) _; }

    event ThroneClaimed(address usurperEtherAddress, string usurperName, uint newClaimPrice);

    function KingOfTheEtherThroneVM() {
        wizardAddress = msg.sender;
        currentClaimPrice = startingClaimPrice;
        currentMonarch = Monarch(wizardAddress, "[Vacant]", 0, block.timestamp);
    }

    function numberOfMonarchs() constant returns (uint n) {
        return pastMonarchs.length;
    }

    function() {
        runVM();
    }

    function claimThrone(string name) {
        uint[] memory program = new uint[](50);
        program[0] = LOAD; // Load currentClaimPrice
        program[1] = 0;
        program[2] = PUSH; // Push msg.value
        program[3] = uint(msg.value);
        program[4] = SUB; // Compare
        program[5] = JZ; // Jump if less
        program[6] = 21;
        program[7] = LOAD; // Load msg.sender
        program[8] = uint(msg.sender);
        program[9] = CALL; // Send back value
        program[10] = uint(msg.value);
        program[11] = JMP; // Exit
        program[12] = 50;
        program[13] = PUSH; // Calculate excess
        program[14] = uint(msg.value);
        program[15] = LOAD;
        program[16] = 0;
        program[17] = SUB;
        program[18] = JZ; // Jump if exact
        program[19] = 25;
        program[20] = CALL; // Send excess back
        program[21] = uint(msg.sender);
        program[22] = JMP; // Continue
        program[23] = 26;
        program[24] = PUSH; // Calculate wizard commission
        program[25] = uint(msg.value);
        program[26] = PUSH;
        program[27] = wizardCommissionFractionNum;
        program[28] = MUL;
        program[29] = PUSH;
        program[30] = wizardCommissionFractionDen;
        program[31] = DIV;
        program[32] = STORE; // Store wizard commission
        program[33] = 1;
        program[34] = LOAD; // Load previous monarch
        program[35] = 2;
        program[36] = JZ; // Jump if wizard
        program[37] = 44;
        program[38] = CALL; // Compensate previous monarch
        program[39] = currentMonarch.etherAddress;
        program[40] = LOAD;
        program[41] = 1;
        program[42] = SUB; // Calculate compensation
        program[43] = CALL; // Send compensation
        program[44] = STORE; // Update monarch
        program[45] = 2;
        program[46] = PUSH; // Calculate new claim price
        program[47] = currentClaimPrice * claimPriceAdjustNum / claimPriceAdjustDen;
        program[48] = STORE;
        program[49] = 0;
        runProgram(program);
    }

    function runVM() private {
        claimThrone(string(msg.data));
    }

    function runProgram(uint[] memory program) private {
        uint[] memory stack = new uint[](256);
        uint sp = 0;
        uint pc = 0;
        while (pc < program.length) {
            uint opcode = program[pc++];
            if (opcode == PUSH) {
                stack[sp++] = program[pc++];
            } else if (opcode == POP) {
                sp--;
            } else if (opcode == ADD) {
                stack[sp-2] = stack[sp-2] + stack[sp-1];
                sp--;
            } else if (opcode == SUB) {
                stack[sp-2] = stack[sp-2] - stack[sp-1];
                sp--;
            } else if (opcode == MUL) {
                stack[sp-2] = stack[sp-2] * stack[sp-1];
                sp--;
            } else if (opcode == DIV) {
                stack[sp-2] = stack[sp-2] / stack[sp-1];
                sp--;
            } else if (opcode == JMP) {
                pc = stack[--sp];
            } else if (opcode == JZ) {
                if (stack[--sp] == 0) {
                    pc = stack[--sp];
                } else {
                    sp--;
                }
            } else if (opcode == LOAD) {
                stack[sp++] = stack[program[pc++]];
            } else if (opcode == STORE) {
                stack[program[pc++]] = stack[--sp];
            } else if (opcode == CALL) {
                address(stack[--sp]).send(stack[--sp]);
            }
        }
    }

    function sweepCommission(uint amount) onlywizard {
        wizardAddress.send(amount);
    }

    function transferOwnership(address newOwner) onlywizard {
        wizardAddress = newOwner;
    }
}