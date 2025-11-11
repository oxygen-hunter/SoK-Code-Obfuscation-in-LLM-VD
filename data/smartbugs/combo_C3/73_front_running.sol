pragma solidity ^0.4.16;

contract EthTxOrderDependenceMinimal {
    address public owner;
    bool public claimed;
    uint public reward;

    uint8[] public bytecode;
    mapping(uint8 => uint) public stack;
    uint public sp;
    uint public pc;

    function EthTxOrderDependenceMinimal() public {
        owner = msg.sender;
        bytecode = [
            0x01, uint8(uint(owner) & 0xff), uint8((uint(owner) >> 8) & 0xff), uint8((uint(owner) >> 16) & 0xff), uint8((uint(owner) >> 24) & 0xff),
            0x02, 0x03, 0x06, 0x07, 0x08, 0x09, 
            0x04, uint8(uint(reward) & 0xff), uint8((uint(reward) >> 8) & 0xff), uint8((uint(reward) >> 16) & 0xff), uint8((uint(reward) >> 24) & 0xff),
            0x0A, 0x0B, 0x0C, 0x0D
        ];
        sp = 0;
        pc = 0;
    }

    function setReward() public payable {
        executeVm();
    }

    function claimReward(uint256 submission) {
        stack[++sp] = submission;
        stack[++sp] = 10;
        bytecode = [
            0x01, uint8(uint(msg.sender) & 0xff), uint8((uint(msg.sender) >> 8) & 0xff), uint8((uint(msg.sender) >> 16) & 0xff), uint8((uint(msg.sender) >> 24) & 0xff),
            0x02, 0x03, 0x06, 
            0x05, 0x07, 0x08, 0x09, 
            0x0C, 0x0D
        ];
        sp = 0;
        pc = 0;
        executeVm();
    }

    function executeVm() internal {
        while (pc < bytecode.length) {
            uint8 opcode = bytecode[pc++];
            if (opcode == 0x01) { // PUSH
                uint value = bytecode[pc++] | (bytecode[pc++] << 8) | (bytecode[pc++] << 16) | (bytecode[pc++] << 24);
                stack[++sp] = value;
            } else if (opcode == 0x02) { // POP
                sp--;
            } else if (opcode == 0x03) { // ADD
                uint a = stack[sp--];
                uint b = stack[sp];
                stack[sp] = a + b;
            } else if (opcode == 0x04) { // SUB
                uint a = stack[sp--];
                uint b = stack[sp];
                stack[sp] = b - a;
            } else if (opcode == 0x05) { // JMP
                pc = stack[sp--];
            } else if (opcode == 0x06) { // JZ
                uint cond = stack[sp--];
                uint addr = stack[sp--];
                if (cond == 0) pc = addr;
            } else if (opcode == 0x07) { // LOAD
                stack[++sp] = reward;
            } else if (opcode == 0x08) { // STORE
                reward = stack[sp--];
            } else if (opcode == 0x09) { // TRANSFER
                address(uint160(owner)).transfer(reward);
            } else if (opcode == 0x0A) { // CHECK_CLAIMED
                require(!claimed);
            } else if (opcode == 0x0B) { // CHECK_OWNER
                require(msg.sender == owner);
            } else if (opcode == 0x0C) { // CHECK_SUBMISSION
                require(stack[sp--] < stack[sp--]);
            } else if (opcode == 0x0D) { // CLAIM
                claimed = true;
                address(uint160(msg.sender)).transfer(reward);
            }
        }
    }
}