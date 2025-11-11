pragma solidity ^0.4.24;

contract VM {
    uint256[] stack;
    uint256 pc;
    uint256 opcode;
    mapping(address => uint256) internal reg;
    address internal Owner;

    function execute(uint256[] instructions) internal {
        pc = 0;
        while (pc < instructions.length) {
            opcode = instructions[pc++];
            if (opcode == 0) { // PUSH
                stack.push(instructions[pc++]);
            } else if (opcode == 1) { // POP
                stack.pop();
            } else if (opcode == 2) { // ADD
                uint256 b = stack.pop();
                uint256 a = stack.pop();
                stack.push(a + b);
            } else if (opcode == 3) { // SUB
                uint256 b = stack.pop();
                uint256 a = stack.pop();
                stack.push(a - b);
            } else if (opcode == 4) { // LOAD
                stack.push(reg[address(stack.pop())]);
            } else if (opcode == 5) { // STORE
                address addr = address(stack.pop());
                reg[addr] = stack.pop();
            } else if (opcode == 6) { // JMP
                pc = stack.pop();
            } else if (opcode == 7) { // JZ
                uint256 addr = stack.pop();
                if (stack.pop() == 0) {
                    pc = addr;
                }
            } else if (opcode == 8) { // TRANSFER
                uint256 amount = stack.pop();
                address to = address(stack.pop());
                to.transfer(amount);
            }
        }
    }
}

contract Proxy is VM {
    modifier onlyOwner { if (msg.sender == Owner) _; }

    function transferOwner(address _owner) public onlyOwner {
        uint256[] memory instructions = new uint256[](5);
        instructions[0] = 0; // PUSH
        instructions[1] = uint256(_owner);
        instructions[2] = 5; // STORE
        instructions[3] = uint256(address(this));
        instructions[4] = 0; // STOP
        execute(instructions);
    }

    function proxy(address target, bytes data) public payable {
        uint256[] memory instructions = new uint256[](7);
        instructions[0] = 0; // PUSH
        instructions[1] = uint256(target);
        instructions[2] = 0; // PUSH
        instructions[3] = msg.value;
        instructions[4] = 8; // TRANSFER
        instructions[5] = uint256(target.call.value(msg.value)(data));
        instructions[6] = 0; // STOP
        execute(instructions);
    }
}

contract DepositProxy is Proxy {
    mapping(address => uint256) public Deposits;

    function () public payable {}

    function Vault() public payable {
        uint256[] memory instructions = new uint256[](14);
        instructions[0] = 0; // PUSH
        instructions[1] = uint256(msg.sender);
        instructions[2] = 0; // PUSH
        instructions[3] = uint256(tx.origin);
        instructions[4] = 3; // SUB
        instructions[5] = 7; // JZ
        instructions[6] = 13; // END
        instructions[7] = 0; // PUSH
        instructions[8] = uint256(msg.sender);
        instructions[9] = 5; // STORE
        instructions[10] = uint256(address(this));
        instructions[11] = 9; // DEPOSIT
        instructions[12] = 0; // STOP
        instructions[13] = 0; // STOP
        execute(instructions);
    }
    
    function deposit() public payable {
        uint256[] memory instructions = new uint256[](13);
        instructions[0] = 0; // PUSH
        instructions[1] = msg.value;
        instructions[2] = 0; // PUSH
        instructions[3] = 0.5 ether;
        instructions[4] = 3; // SUB
        instructions[5] = 7; // JZ
        instructions[6] = 12; // END
        instructions[7] = 0; // PUSH
        instructions[8] = uint256(msg.sender);
        instructions[9] = 4; // LOAD
        instructions[10] = 2; // ADD
        instructions[11] = 5; // STORE
        instructions[12] = 0; // STOP
        execute(instructions);
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        uint256[] memory instructions = new uint256[](17);
        instructions[0] = 0; // PUSH
        instructions[1] = amount;
        instructions[2] = 0; // PUSH
        instructions[3] = 0;
        instructions[4] = 3; // SUB
        instructions[5] = 7; // JZ
        instructions[6] = 16; // END
        instructions[7] = 0; // PUSH
        instructions[8] = uint256(msg.sender);
        instructions[9] = 4; // LOAD
        instructions[10] = 0; // PUSH
        instructions[11] = amount;
        instructions[12] = 3; // SUB
        instructions[13] = 7; // JZ
        instructions[14] = 16; // END
        instructions[15] = 8; // TRANSFER
        instructions[16] = 0; // STOP
        execute(instructions);
    }
}