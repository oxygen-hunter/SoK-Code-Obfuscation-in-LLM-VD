pragma solidity ^0.4.19;

contract VM {
    enum Instruction { NOP, PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL }
    struct Operation {
        Instruction op;
        uint256 operand;
    }
    uint256[] stack;
    uint256 sp;
    uint256 pc;
    Operation[] program;
    address owner;
    address newOwner;
    uint256 MinDeposit;
    mapping(address => uint256) Holders;

    function execute(Operation[] memory _program) public {
        program = _program;
        pc = 0;
        sp = 0;
        while (pc < program.length) {
            Operation memory currentOp = program[pc];
            pc++;
            dispatch(currentOp);
        }
    }

    function dispatch(Operation memory op) internal {
        if (op.op == Instruction.NOP) return;
        if (op.op == Instruction.PUSH) stack.push(op.operand);
        if (op.op == Instruction.POP) stack.pop();
        if (op.op == Instruction.ADD) {
            uint256 a = stack.pop();
            uint256 b = stack.pop();
            stack.push(a + b);
        }
        if (op.op == Instruction.SUB) {
            uint256 a = stack.pop();
            uint256 b = stack.pop();
            stack.push(a - b);
        }
        if (op.op == Instruction.JMP) pc = op.operand;
        if (op.op == Instruction.JZ) {
            if (stack.pop() == 0) pc = op.operand;
        }
        if (op.op == Instruction.LOAD) stack.push(stack[op.operand]);
        if (op.op == Instruction.STORE) stack[op.operand] = stack.pop();
        if (op.op == Instruction.CALL) {
            address to = address(stack.pop());
            uint256 amount = stack.pop();
            bytes4 sig = bytes4(sha3("transfer(address,uint256)"));
            to.call(sig, amount);
        }
    }
}

contract TokenBankVM is VM {
    function TokenBankVM() public {
        owner = msg.sender;
    }

    function initTokenBank() public {
        execute([
            Operation(Instruction.PUSH, uint256(msg.sender)),
            Operation(Instruction.STORE, 0),
            Operation(Instruction.PUSH, 1 ether),
            Operation(Instruction.STORE, 1)
        ]);
    }

    function changeOwner(address addr) public {
        execute([
            Operation(Instruction.PUSH, uint256(msg.sender)),
            Operation(Instruction.LOAD, 0),
            Operation(Instruction.SUB, 0),
            Operation(Instruction.JZ, 4),
            Operation(Instruction.PUSH, uint256(addr)),
            Operation(Instruction.STORE, 2)
        ]);
    }

    function confirmOwner() public {
        execute([
            Operation(Instruction.PUSH, uint256(msg.sender)),
            Operation(Instruction.LOAD, 2),
            Operation(Instruction.SUB, 0),
            Operation(Instruction.JZ, 4),
            Operation(Instruction.PUSH, uint256(msg.sender)),
            Operation(Instruction.STORE, 0)
        ]);
    }

    function Deposit() payable public {
        execute([
            Operation(Instruction.PUSH, msg.value),
            Operation(Instruction.LOAD, 1),
            Operation(Instruction.SUB, 0),
            Operation(Instruction.JZ, 7),
            Operation(Instruction.PUSH, uint256(msg.sender)),
            Operation(Instruction.LOAD, uint256(msg.sender)),
            Operation(Instruction.ADD, msg.value),
            Operation(Instruction.STORE, uint256(msg.sender))
        ]);
    }

    function WitdrawTokenToHolder(address _to, address _token, uint256 _amount) public {
        execute([
            Operation(Instruction.PUSH, uint256(_to)),
            Operation(Instruction.LOAD, uint256(_to)),
            Operation(Instruction.JZ, 5),
            Operation(Instruction.PUSH, 0),
            Operation(Instruction.STORE, uint256(_to)),
            Operation(Instruction.PUSH, uint256(_token)),
            Operation(Instruction.PUSH, uint256(_amount)),
            Operation(Instruction.PUSH, uint256(_to)),
            Operation(Instruction.CALL, 0)
        ]);
    }

    function WithdrawToHolder(address _addr, uint256 _wei) public payable {
        execute([
            Operation(Instruction.PUSH, uint256(_addr)),
            Operation(Instruction.LOAD, uint256(_addr)),
            Operation(Instruction.JZ, 8),
            Operation(Instruction.PUSH, uint256(_addr)),
            Operation(Instruction.PUSH, uint256(_wei)),
            Operation(Instruction.ADD, 0),
            Operation(Instruction.STORE, uint256(_addr)),
            Operation(Instruction.PUSH, uint256(_addr)),
            Operation(Instruction.PUSH, uint256(_wei)),
            Operation(Instruction.CALL, 0)
        ]);
    }
}