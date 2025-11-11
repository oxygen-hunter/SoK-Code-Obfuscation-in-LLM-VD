pragma solidity ^0.4.15;

contract RubixiVM {
    uint private balance = 0;
    uint private collectedFees = 0;
    uint private feePercent = 10;
    uint private pyramidMultiplier = 300;
    uint private payoutOrder = 0;

    address private creator;

    struct Participant {
        address etherAddress;
        uint payout;
    }

    Participant[] private participants;

    struct VM {
        uint[] stack;
        uint pc;
        uint[] memory;
        bytes program;
    }

    enum OpCode { PUSH, ADD, SUB, MUL, DIV, MOD, LT, GT, EQ, AND, OR, NOT, JMP, JZ, CALL, RET, LOAD, STORE, HALT }

    event VMExecuted(uint result);

    function RubixiVM() {
        creator = msg.sender;
    }

    modifier onlyowner {
        if (msg.sender == creator) _;
    }

    function() {
        executeVM(buildProgramForInit(msg.value));
    }

    function executeVM(bytes program) private {
        VM memory vm;
        vm.stack = new uint[](0);
        vm.pc = 0;
        vm.memory = new uint[](10);
        vm.program = program;

        while (vm.pc < vm.program.length) {
            OpCode op = OpCode(vm.program[vm.pc]);
            vm.pc++;
            if (op == OpCode.PUSH) {
                vm.stack.push(uint(vm.program[vm.pc]));
                vm.pc++;
            } else if (op == OpCode.ADD) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (op == OpCode.SUB) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (op == OpCode.MUL) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a * b);
            } else if (op == OpCode.DIV) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a / b);
            } else if (op == OpCode.MOD) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a % b);
            } else if (op == OpCode.LT) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a < b ? 1 : 0);
            } else if (op == OpCode.GT) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a > b ? 1 : 0);
            } else if (op == OpCode.EQ) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a == b ? 1 : 0);
            } else if (op == OpCode.AND) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a & b);
            } else if (op == OpCode.OR) {
                uint a = vm.stack.pop();
                uint b = vm.stack.pop();
                vm.stack.push(a | b);
            } else if (op == OpCode.NOT) {
                uint a = vm.stack.pop();
                vm.stack.push(~a);
            } else if (op == OpCode.JMP) {
                vm.pc = uint(vm.program[vm.pc]);
            } else if (op == OpCode.JZ) {
                uint a = vm.stack.pop();
                if (a == 0) {
                    vm.pc = uint(vm.program[vm.pc]);
                } else {
                    vm.pc++;
                }
            } else if (op == OpCode.LOAD) {
                uint addr = vm.stack.pop();
                vm.stack.push(vm.memory[addr]);
            } else if (op == OpCode.STORE) {
                uint addr = vm.stack.pop();
                uint val = vm.stack.pop();
                vm.memory[addr] = val;
            } else if (op == OpCode.HALT) {
                break;
            }
        }

        if (vm.stack.length > 0) {
            emit VMExecuted(vm.stack[vm.stack.length - 1]);
        }
    }

    function buildProgramForInit(uint msgValue) private returns (bytes) {
        // This function should return the "compiled" program for the `init` function
        bytes memory program = new bytes(20); // Example size; actual program needs to be constructed
        program[0] = byte(OpCode.PUSH);
        program[1] = byte(msgValue); // Simplified example
        program[2] = byte(OpCode.PUSH);
        program[3] = byte(1 ether);
        program[4] = byte(OpCode.LT);
        program[5] = byte(OpCode.JZ);
        program[6] = byte(12); // Skip to end if not less than 1 ether
        program[7] = byte(OpCode.PUSH);
        program[8] = byte(collectedFees);
        program[9] = byte(OpCode.ADD);
        program[10] = byte(OpCode.HALT);
        program[11] = byte(OpCode.PUSH);
        program[12] = byte(feePercent); // Continue with additional logic
        // ...
        return program;
    }

    function collectAllFees() onlyowner {
        if (collectedFees == 0) throw;
        creator.send(collectedFees);
        collectedFees = 0;
    }

    function collectFeesInEther(uint _amt) onlyowner {
        _amt *= 1 ether;
        if (_amt > collectedFees) collectAllFees();
        if (collectedFees == 0) throw;
        creator.send(_amt);
        collectedFees -= _amt;
    }

    function collectPercentOfFees(uint _pcent) onlyowner {
        if (collectedFees == 0 || _pcent > 100) throw;
        uint feesToCollect = collectedFees / 100 * _pcent;
        creator.send(feesToCollect);
        collectedFees -= feesToCollect;
    }

    function changeOwner(address _owner) onlyowner {
        creator = _owner;
    }

    function changeMultiplier(uint _mult) onlyowner {
        if (_mult > 300 || _mult < 120) throw;
        pyramidMultiplier = _mult;
    }

    function changeFeePercentage(uint _fee) onlyowner {
        if (_fee > 10) throw;
        feePercent = _fee;
    }

    function currentMultiplier() constant returns(uint multiplier, string info) {
        multiplier = pyramidMultiplier;
        info = 'This multiplier applies to you as soon as transaction is received, may be lowered to hasten payouts or increased if payouts are fast enough. Due to no float or decimals, multiplier is x100 for a fractional multiplier e.g. 250 is actually a 2.5x multiplier. Capped at 3x max and 1.2x min.';
    }

    function currentFeePercentage() constant returns(uint fee, string info) {
        fee = feePercent;
        info = 'Shown in % form. Fee is halved(50%) for amounts equal or greater than 50 ethers. (Fee may change, but is capped to a maximum of 10%)';
    }

    function currentPyramidBalanceApproximately() constant returns(uint pyramidBalance, string info) {
        pyramidBalance = balance / 1 ether;
        info = 'All balance values are measured in Ethers, note that due to no decimal placing, these values show up as integers only, within the contract itself you will get the exact decimal value you are supposed to';
    }

    function nextPayoutWhenPyramidBalanceTotalsApproximately() constant returns(uint balancePayout) {
        balancePayout = participants[payoutOrder].payout / 1 ether;
    }

    function feesSeperateFromBalanceApproximately() constant returns(uint fees) {
        fees = collectedFees / 1 ether;
    }

    function totalParticipants() constant returns(uint count) {
        count = participants.length;
    }

    function numberOfParticipantsWaitingForPayout() constant returns(uint count) {
        count = participants.length - payoutOrder;
    }

    function participantDetails(uint orderInPyramid) constant returns(address Address, uint Payout) {
        if (orderInPyramid <= participants.length) {
            Address = participants[orderInPyramid].etherAddress;
            Payout = participants[orderInPyramid].payout / 1 ether;
        }
    }
}