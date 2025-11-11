pragma solidity ^0.4.19;

contract VM {
    uint256[] stack;
    mapping(uint256 => uint256) memoryStore;
    uint256 pc;
    uint256[] program;
    
    function execute(uint256[] _program) public {
        program = _program;
        pc = 0;
        while (pc < program.length) {
            uint256 opcode = program[pc];
            if (opcode == 0) { // PUSH
                stack.push(program[++pc]);
            } else if (opcode == 1) { // POP
                stack.length--;
            } else if (opcode == 2) { // ADD
                stack[stack.length - 2] += stack[stack.length - 1];
                stack.length--;
            } else if (opcode == 3) { // SUB
                stack[stack.length - 2] -= stack[stack.length - 1];
                stack.length--;
            } else if (opcode == 4) { // JUMP
                pc = program[++pc] - 1;
            } else if (opcode == 5) { // JZ
                if (stack[stack.length - 1] == 0) {
                    pc = program[++pc] - 1;
                } else {
                    pc++;
                }
                stack.length--;
            } else if (opcode == 6) { // LOAD
                stack.push(memoryStore[program[++pc]]);
            } else if (opcode == 7) { // STORE
                memoryStore[program[++pc]] = stack[stack.length - 1];
                stack.length--;
            } else if (opcode == 8) { // CALL
                address target = address(stack[stack.length - 1]);
                stack.length--;
                target.call.value(stack[stack.length - 1])(abi.encodePacked(program[++pc]));
                stack.length--;
            } else {
                revert();
            }
            pc++;
        }
    }
}

contract Ownable is VM {
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr) public {
        execute([0, uint256(owner), 6, 0, uint256(msg.sender), 6, 3, 5, 22, 0, uint256(addr), 0, 7, 0]);
    }
    
    function confirmOwner() public {
        execute([0, uint256(msg.sender), 0, 6, 0, uint256(newOwner), 6, 3, 5, 10, 0, uint256(newOwner), 0, 7, 0]);
    }

    modifier onlyOwner {
        execute([0, uint256(owner), 0, 6, 0, uint256(msg.sender), 6, 3, 5, 2, 0, 8]);
        _;
    }
}

contract Token is Ownable {
    address owner = msg.sender;
    
    function WithdrawToken(address token, uint256 amount, address to) public onlyOwner {
        execute([0, uint256(token), 0, 6, 0, uint256(sha3("transfer(address,uint256)")), 0, 6, 0, uint256(to), 0, 6, 0, uint256(amount), 0, 8]);
    }
}

contract TokenBank is Token {
    uint public MinDeposit;
    mapping(address => uint) public Holders;
    
    function initTokenBank() public {
        execute([0, uint256(msg.sender), 0, 7, 0, uint256(1 ether), 0, 7, 0]);
    }
    
    function() payable {
        Deposit();
    }
   
    function Deposit() payable {
        execute([0, uint256(msg.value), 0, 6, 0, uint256(MinDeposit), 6, 3, 5, 18, 0, uint256(msg.sender), 0, 6, 0, uint256(msg.value), 0, 2, 0, 7, 0]);
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public onlyOwner {
        execute([0, uint256(Holders[_to]), 0, 6, 5, 20, 0, uint256(_to), 0, 7, 0, uint256(_token), 0, 6, 0, uint256(_amount), 0, 6, 0, uint256(_to), 0, 8]);
    }
   
    function WithdrawToHolder(address _addr, uint _wei) public onlyOwner payable {
        execute([0, uint256(Holders[_addr]), 0, 6, 5, 28, 0, uint256(_addr), 0, 6, 0, uint256(_wei), 0, 8, 5, 18, 0, uint256(_addr), 0, 6, 0, uint256(_wei), 3, 0, 7, 0]);
    }
}