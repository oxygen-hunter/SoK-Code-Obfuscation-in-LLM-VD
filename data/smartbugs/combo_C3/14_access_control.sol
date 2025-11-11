pragma solidity ^0.4.24;

contract Wallet {
    uint256[] stack;
    uint256 pc;
    address creator;
    mapping(address => uint256) balances;

    constructor() public {
        creator = msg.sender;
        execute(0);
    }

    function deposit() public payable {
        execute(1);
    }

    function withdraw(uint256 amount) public {
        stack.push(amount);
        execute(2);
    }

    function migrateTo(address to) public {
        stack.push(uint256(to));
        execute(3);
    }
    
    function execute(uint256 entryPoint) internal {
        uint256[] memory program = new uint256[](15);
        program[0] = 0;  // Entry point for constructor
        program[1] = 100; // Entry point for deposit
        program[2] = 200; // Entry point for withdraw
        program[3] = 300; // Entry point for migrateTo

        pc = program[entryPoint];
        while (true) {
            uint256 opcode = pc;
            if (opcode == 0) { // STOP
                break;
            } else if (opcode == 100) { // DEPOSIT
                stack.push(uint256(msg.sender));
                uint256 val = msg.value;
                uint256 sender = stack.pop();
                stack.push(balances[address(sender)] + val);
                require(stack[stack.length - 1] > balances[address(sender)]);
                balances[address(sender)] = stack.pop();
                pc = 0;
            } else if (opcode == 200) { // WITHDRAW
                uint256 amount = stack.pop();
                stack.push(uint256(msg.sender));
                require(amount <= balances[address(stack[stack.length - 1])]);
                msg.sender.transfer(amount);
                balances[address(stack[stack.length - 1])] -= amount;
                pc = 0;
            } else if (opcode == 300) { // MIGRATETO
                uint256 to = stack.pop();
                require(creator == msg.sender);
                address(to).transfer(this.balance);
                pc = 0;
            }
        }
    }
}