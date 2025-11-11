pragma solidity ^0.4.0;

contract Government {
    uint32 public lastCreditorPayedOut;
    uint public lastTimeOfNewCredit;
    uint public profitFromCrash;
    address[] public creditorAddresses;
    uint[] public creditorAmounts;
    address public corruptElite;
    mapping (address => uint) buddies;
    uint constant TWELVE_HOURS = 43200;
    uint8 public round;

    // Virtual Machine instructions
    uint8 constant PUSH = 0;
    uint8 constant POP = 1;
    uint8 constant ADD = 2;
    uint8 constant SUB = 3;
    uint8 constant JMP = 4;
    uint8 constant JZ = 5;
    uint8 constant LOAD = 6;
    uint8 constant STORE = 7;
    uint8 constant SEND = 8;
    uint8 constant HALT = 255;

    struct VM {
        uint[] stack;
        uint pc;
    }

    function execute(uint[] instructions) internal {
        VM memory vm;
        vm.pc = 0;

        while (vm.pc < instructions.length) {
            uint8 opcode = uint8(instructions[vm.pc]);
            if (opcode == PUSH) {
                vm.stack.push(instructions[++vm.pc]);
            } else if (opcode == POP) {
                vm.stack.length--;
            } else if (opcode == ADD) {
                uint b = vm.stack[vm.stack.length - 1];
                uint a = vm.stack[vm.stack.length - 2];
                vm.stack.length--;
                vm.stack[vm.stack.length - 1] = a + b;
            } else if (opcode == SUB) {
                uint b = vm.stack[vm.stack.length - 1];
                uint a = vm.stack[vm.stack.length - 2];
                vm.stack.length--;
                vm.stack[vm.stack.length - 1] = a - b;
            } else if (opcode == JMP) {
                vm.pc = instructions[++vm.pc] - 1;
            } else if (opcode == JZ) {
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = instructions[++vm.pc] - 1;
                } else {
                    vm.pc++;
                }
            } else if (opcode == LOAD) {
                uint index = instructions[++vm.pc];
                vm.stack.push(vm.stack[index]);
            } else if (opcode == STORE) {
                uint index = instructions[++vm.pc];
                vm.stack[index] = vm.stack[vm.stack.length - 1];
            } else if (opcode == SEND) {
                address(uint160(vm.stack[vm.stack.length - 2])).transfer(vm.stack[vm.stack.length - 1]);
                vm.stack.length -= 2;
            } else if (opcode == HALT) {
                break;
            }
            vm.pc++;
        }
    }

    function Government() {
        profitFromCrash = msg.value;
        corruptElite = msg.sender;
        lastTimeOfNewCredit = block.timestamp;
    }

    function lendGovernmentMoney(address buddy) returns (bool) {
        uint amount = msg.value;
        uint[] memory instructions = new uint[](100);
        uint i = 0;

        instructions[i++] = PUSH;
        instructions[i++] = lastTimeOfNewCredit + TWELVE_HOURS;
        instructions[i++] = PUSH;
        instructions[i++] = block.timestamp;
        instructions[i++] = SUB;
        instructions[i++] = JZ;
        uint jmpPos = i++;
        instructions[i++] = PUSH;
        instructions[i++] = uint(msg.sender);
        instructions[i++] = PUSH;
        instructions[i++] = amount;
        instructions[i++] = SEND;
        instructions[i++] = PUSH;
        instructions[i++] = uint(creditorAddresses[creditorAddresses.length - 1]);
        instructions[i++] = PUSH;
        instructions[i++] = profitFromCrash;
        instructions[i++] = SEND;
        instructions[i++] = PUSH;
        instructions[i++] = uint(corruptElite);
        instructions[i++] = PUSH;
        instructions[i++] = address(this).balance;
        instructions[i++] = SEND;
        instructions[i++] = PUSH;
        instructions[i++] = 0;
        instructions[i++] = STORE;
        instructions[i++] = PUSH;
        instructions[i++] = block.timestamp;
        instructions[i++] = STORE;
        instructions[i++] = PUSH;
        instructions[i++] = 0;
        instructions[i++] = STORE;
        instructions[i++] = PUSH;
        instructions[i++] = 0;
        instructions[i++] = STORE;
        instructions[i++] = PUSH;
        instructions[i++] = uint(creditorAddresses);
        instructions[i++] = PUSH;
        instructions[i++] = uint(creditorAmounts);
        instructions[i++] = PUSH;
        instructions[i++] = round;
        instructions[i++] = PUSH;
        instructions[i++] = 1;
        instructions[i++] = ADD;
        instructions[i++] = STORE;
        instructions[i++] = PUSH;
        instructions[i++] = 0;
        instructions[i++] = HALT;
        instructions[jmpPos] = i;

        execute(instructions);

        if (amount >= 10 ** 18) {
            lastTimeOfNewCredit = block.timestamp;
            creditorAddresses.push(msg.sender);
            creditorAmounts.push(amount * 110 / 100);
            corruptElite.send(amount * 5 / 100);

            if (profitFromCrash < 10000 * 10**18) {
                profitFromCrash += amount * 5 / 100;
            }

            if (buddies[buddy] >= amount) {
                buddy.send(amount * 5 / 100);
            }
            buddies[msg.sender] += amount * 110 / 100;

            if (creditorAmounts[lastCreditorPayedOut] <= address(this).balance - profitFromCrash) {
                creditorAddresses[lastCreditorPayedOut].send(creditorAmounts[lastCreditorPayedOut]);
                buddies[creditorAddresses[lastCreditorPayedOut]] -= creditorAmounts[lastCreditorPayedOut];
                lastCreditorPayedOut += 1;
            }
            return true;
        } else {
            msg.sender.send(amount);
            return false;
        }
    }

    function() {
        lendGovernmentMoney(0);
    }

    function totalDebt() returns (uint debt) {
        for (uint i = lastCreditorPayedOut; i < creditorAmounts.length; i++) {
            debt += creditorAmounts[i];
        }
    }

    function totalPayedOut() returns (uint payout) {
        for (uint i = 0; i < lastCreditorPayedOut; i++) {
            payout += creditorAmounts[i];
        }
    }

    function investInTheSystem() {
        profitFromCrash += msg.value;
    }

    function inheritToNextGeneration(address nextGeneration) {
        if (msg.sender == corruptElite) {
            corruptElite = nextGeneration;
        }
    }

    function getCreditorAddresses() returns (address[]) {
        return creditorAddresses;
    }

    function getCreditorAmounts() returns (uint[]) {
        return creditorAmounts;
    }
}