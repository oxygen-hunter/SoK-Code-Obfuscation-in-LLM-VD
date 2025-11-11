pragma solidity ^0.4.0;

contract ObfuscatedLuckyDoubler {

    address private VM_owner;
    uint private VM_balance = 0;
    uint private VM_fee = 5;
    uint private VM_multiplier = 125;
    uint256 constant private VM_FACTOR = 1157920892373161954235709850086879078532699846656405640394575840079131296399;

    mapping (address => VM_User) private VM_users;
    VM_Entry[] private VM_entries;
    uint[] private VM_unpaidEntries;

    struct VM_User {
        address id;
        uint deposits;
        uint payoutsReceived;
    }

    struct VM_Entry {
        address entryAddress;
        uint deposit;
        uint payout;
        bool paid;
    }

    uint[] private VM_stack;
    uint private VM_pc = 0;

    enum VM_OPCODES {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT
    }

    function ObfuscatedLuckyDoubler() {
        VM_owner = msg.sender;
        execute();
    }

    function execute() private {
        uint[] memory bytecode = new uint[](300);
        uint i = 0;

        bytecode[i++] = uint(VM_OPCODES.PUSH);
        bytecode[i++] = uint(msg.sender);
        bytecode[i++] = uint(VM_OPCODES.CALL);
        bytecode[i++] = uint(4);

        bytecode[i++] = uint(VM_OPCODES.PUSH);
        bytecode[i++] = uint(msg.value);

        bytecode[i++] = uint(VM_OPCODES.PUSH);
        bytecode[i++] = 1 ether;
        bytecode[i++] = uint(VM_OPCODES.SUB);

        bytecode[i++] = uint(VM_OPCODES.JZ);
        bytecode[i++] = 16;

        bytecode[i++] = uint(VM_OPCODES.PUSH);
        bytecode[i++] = uint(msg.sender);
        bytecode[i++] = uint(VM_OPCODES.PUSH);
        bytecode[i++] = 1 ether;
        bytecode[i++] = uint(VM_OPCODES.SUB);
        bytecode[i++] = uint(VM_OPCODES.CALL);
        bytecode[i++] = uint(5);

        bytecode[i++] = uint(VM_OPCODES.CALL);
        bytecode[i++] = uint(6);

        bytecode[i++] = uint(VM_OPCODES.HALT);

        while (VM_pc < bytecode.length) {
            uint opcode = bytecode[VM_pc++];

            if (opcode == uint(VM_OPCODES.PUSH)) {
                VM_stack.push(bytecode[VM_pc++]);
            } else if (opcode == uint(VM_OPCODES.POP)) {
                VM_stack.pop();
            } else if (opcode == uint(VM_OPCODES.ADD)) {
                uint b = VM_stack.pop();
                uint a = VM_stack.pop();
                VM_stack.push(a + b);
            } else if (opcode == uint(VM_OPCODES.SUB)) {
                uint b = VM_stack.pop();
                uint a = VM_stack.pop();
                VM_stack.push(a - b);
            } else if (opcode == uint(VM_OPCODES.JMP)) {
                VM_pc = VM_stack.pop();
            } else if (opcode == uint(VM_OPCODES.JZ)) {
                uint address = VM_stack.pop();
                if (VM_stack.pop() == 0) {
                    VM_pc = address;
                }
            } else if (opcode == uint(VM_OPCODES.LOAD)) {
                // Implement LOAD operation
            } else if (opcode == uint(VM_OPCODES.STORE)) {
                // Implement STORE operation
            } else if (opcode == uint(VM_OPCODES.CALL)) {
                uint funcAddress = VM_stack.pop();
                if (funcAddress == 4) {
                    VM_init();
                } else if (funcAddress == 5) {
                    msg.sender.send(VM_stack.pop());
                } else if (funcAddress == 6) {
                    VM_join();
                }
            } else if (opcode == uint(VM_OPCODES.HALT)) {
                break;
            }
        }
    }

    function VM_init() private {
        if (msg.value < 1 ether) {
            msg.sender.send(msg.value);
            return;
        }
        VM_join();
    }

    function VM_join() private {
        uint dValue = 1 ether;
        if (msg.value > 1 ether) {
            msg.sender.send(msg.value - 1 ether);
            dValue = 1 ether;
        }
        if (VM_users[msg.sender].id == address(0)) {
            VM_users[msg.sender].id = msg.sender;
            VM_users[msg.sender].deposits = 0;
            VM_users[msg.sender].payoutsReceived = 0;
        }
        VM_entries.push(VM_Entry(msg.sender, dValue, (dValue * (VM_multiplier) / 100), false));
        VM_users[msg.sender].deposits++;
        VM_unpaidEntries.push(VM_entries.length - 1);
        VM_balance += (dValue * (100 - VM_fee)) / 100;

        uint index = VM_unpaidEntries.length > 1 ? VM_rand(VM_unpaidEntries.length) : 0;
        VM_Entry storage theEntry = VM_entries[VM_unpaidEntries[index]];

        if (VM_balance > theEntry.payout) {
            uint payout = theEntry.payout;
            theEntry.entryAddress.send(payout);
            theEntry.paid = true;
            VM_users[theEntry.entryAddress].payoutsReceived++;
            VM_balance -= payout;
            if (index < VM_unpaidEntries.length - 1)
                VM_unpaidEntries[index] = VM_unpaidEntries[VM_unpaidEntries.length - 1];
            VM_unpaidEntries.length--;
        }

        uint fees = this.balance - VM_balance;
        if (fees > 0) {
            VM_owner.send(fees);
        }
    }

    function VM_rand(uint max) constant private returns (uint256 result) {
        uint256 factor = VM_FACTOR * 100 / max;
        uint256 lastBlockNumber = block.number - 1;
        uint256 hashVal = uint256(block.blockhash(lastBlockNumber));
        return uint256((uint256(hashVal) / factor)) % max;
    }

    function changeOwner(address newOwner) {
        require(msg.sender == VM_owner);
        VM_owner = newOwner;
    }

    function changeMultiplier(uint multi) {
        require(msg.sender == VM_owner);
        if (multi < 110 || multi > 150) revert();
        VM_multiplier = multi;
    }

    function changeFee(uint newFee) {
        require(msg.sender == VM_owner);
        if (newFee > 5) revert();
        VM_fee = newFee;
    }

    function multiplierFactor() constant returns (uint factor, string info) {
        factor = VM_multiplier;
        info = 'The current multiplier applied to all deposits. Min 110%, max 150%.';
    }

    function currentFee() constant returns (uint feePercentage, string info) {
        feePercentage = VM_fee;
        info = 'The fee percentage applied to all deposits. It can change to speed payouts (max 5%).';
    }

    function totalEntries() constant returns (uint count, string info) {
        count = VM_entries.length;
        info = 'The number of deposits.';
    }

    function userStats(address user) constant returns (uint deposits, uint payouts, string info) {
        if (VM_users[user].id != address(0x0)) {
            deposits = VM_users[user].deposits;
            payouts = VM_users[user].payoutsReceived;
            info = 'Users stats: total deposits, payouts received.';
        }
    }

    function entryDetails(uint index) constant returns (address user, uint payout, bool paid, string info) {
        if (index < VM_entries.length) {
            user = VM_entries[index].entryAddress;
            payout = VM_entries[index].payout / 1 finney;
            paid = VM_entries[index].paid;
            info = 'Entry info: user address, expected payout in Finneys, payout status.';
        }
    }
}