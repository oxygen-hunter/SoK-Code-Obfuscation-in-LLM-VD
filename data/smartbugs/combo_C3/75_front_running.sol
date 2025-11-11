pragma solidity ^0.4.24;

library VM {
    enum Opcode { PUSH, POP, ADD, SUB, MUL, DIV, MOD, LOAD, STORE, JMP, JZ, HALT }
    struct State {
        uint256[] stack;
        uint256 pc;
        bool halted;
    }

    function execute(State memory state, bytes memory program, uint256[] memory data) internal pure {
        while (!state.halted && state.pc < program.length) {
            Opcode op = Opcode(uint8(program[state.pc++]));
            if (op == Opcode.PUSH) {
                state.stack.push(data[uint256(program[state.pc++])]);
            } else if (op == Opcode.POP) {
                state.stack.pop();
            } else if (op == Opcode.ADD) {
                uint256 b = state.stack.pop();
                uint256 a = state.stack.pop();
                state.stack.push(a + b);
            } else if (op == Opcode.SUB) {
                uint256 b = state.stack.pop();
                uint256 a = state.stack.pop();
                state.stack.push(a - b);
            } else if (op == Opcode.MUL) {
                uint256 b = state.stack.pop();
                uint256 a = state.stack.pop();
                state.stack.push(a * b);
            } else if (op == Opcode.DIV) {
                uint256 b = state.stack.pop();
                uint256 a = state.stack.pop();
                state.stack.push(a / b);
            } else if (op == Opcode.MOD) {
                uint256 b = state.stack.pop();
                uint256 a = state.stack.pop();
                state.stack.push(a % b);
            } else if (op == Opcode.LOAD) {
                state.stack.push(data[uint256(program[state.pc++])]);
            } else if (op == Opcode.STORE) {
                uint256 value = state.stack.pop();
                data[uint256(program[state.pc++])] = value;
            } else if (op == Opcode.JMP) {
                state.pc = uint256(program[state.pc]);
            } else if (op == Opcode.JZ) {
                uint256 value = state.stack.pop();
                if (value == 0) {
                    state.pc = uint256(program[state.pc]);
                } else {
                    state.pc++;
                }
            } else if (op == Opcode.HALT) {
                state.halted = true;
            }
        }
    }
}

contract ERC20 {
    using VM for VM.State;
    using VM for bytes;

    event Transfer(address indexed from, address indexed to, uint256 value);
    event Approval(address indexed owner, address indexed spender, uint256 value);

    mapping(address => uint256) private _balances;
    mapping(address => mapping(address => uint256)) private _allowed;
    uint256 private _totalSupply;

    constructor(uint totalSupply) public {
        _balances[msg.sender] = totalSupply;
    }

    function balanceOf(address owner) public view returns (uint256) {
        return _balances[owner];
    }

    function allowance(address owner, address spender) public view returns (uint256) {
        return _allowed[owner][spender];
    }

    function transfer(address to, uint256 value) public returns (bool) {
        bytes memory program = hex"0102030405060708090A0B0C0D0E0F";
        uint256[] memory data = new uint256[](3);
        data[0] = _balances[msg.sender];
        data[1] = value;
        data[2] = _balances[to];

        VM.State memory state;
        program.execute(state, data);

        _balances[msg.sender] = data[0];
        _balances[to] = data[2];
        emit Transfer(msg.sender, to, value);
        return true;
    }

    function approve(address spender, uint256 value) public returns (bool) {
        require(spender != address(0));
        _allowed[msg.sender][spender] = value;
        emit Approval(msg.sender, spender, value);
        return true;
    }

    function transferFrom(address from, address to, uint256 value) public returns (bool) {
        bytes memory program = hex"0102030405060708090A0B0C0D0E0F";
        uint256[] memory data = new uint256[](4);
        data[0] = _balances[from];
        data[1] = value;
        data[2] = _allowed[from][msg.sender];
        data[3] = _balances[to];

        VM.State memory state;
        program.execute(state, data);

        _balances[from] = data[0];
        _balances[to] = data[3];
        _allowed[from][msg.sender] = data[2];
        emit Transfer(from, to, value);
        return true;
    }
}