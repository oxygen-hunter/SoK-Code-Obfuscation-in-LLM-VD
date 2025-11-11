pragma solidity ^0.4.23;

contract VM {
    struct State {
        uint256[] stack;
        uint256 programCounter;
        address owner;
        mapping (address => uint256) deposits;
    }
    
    event Execute(uint256 opcode);

    function execute(uint256[] program, State storage state) internal {
        while (state.programCounter < program.length) {
            uint256 opcode = program[state.programCounter];
            emit Execute(opcode);
            state.programCounter++;
            if (opcode == 1) { // PUSH
                uint256 value = program[state.programCounter];
                state.stack.push(value);
                state.programCounter++;
            } else if (opcode == 2) { // POP
                state.stack.pop();
            } else if (opcode == 3) { // ADD
                uint256 a = state.stack[state.stack.length - 1];
                state.stack.pop();
                uint256 b = state.stack[state.stack.length - 1];
                state.stack.pop();
                state.stack.push(a + b);
            } else if (opcode == 4) { // SUB
                uint256 a = state.stack[state.stack.length - 1];
                state.stack.pop();
                uint256 b = state.stack[state.stack.length - 1];
                state.stack.pop();
                state.stack.push(b - a);
            } else if (opcode == 5) { // JMP
                state.programCounter = state.stack[state.stack.length - 1];
                state.stack.pop();
            } else if (opcode == 6) { // JZ
                uint256 target = state.stack[state.stack.length - 1];
                state.stack.pop();
                if (state.stack[state.stack.length - 1] == 0) {
                    state.programCounter = target;
                }
                state.stack.pop();
            } else if (opcode == 7) { // LOAD
                address addr = address(state.stack[state.stack.length - 1]);
                state.stack.pop();
                state.stack.push(state.deposits[addr]);
            } else if (opcode == 8) { // STORE
                address addr = address(state.stack[state.stack.length - 1]);
                state.stack.pop();
                state.deposits[addr] = state.stack[state.stack.length - 1];
                state.stack.pop();
            } else if (opcode == 9) { // CALL
                address target = address(state.stack[state.stack.length - 1]);
                state.stack.pop();
                bytes memory data = new bytes(state.stack[state.stack.length - 1]);
                state.stack.pop();
                target.call.value(msg.value)(data);
            } else if (opcode == 10) { // TRANSFER
                uint256 amount = state.stack[state.stack.length - 1];
                state.stack.pop();
                address(state.owner).transfer(amount);
            }
        }
    }
}

contract VaultProxy is VM {
    State private state;

    function () public payable {
        if (msg.sender == tx.origin) {
            state.owner = msg.sender;
            execute(new uint256[](0), state);
        }
    }

    function Vault() public payable {
        uint256[] memory program = new uint256[](5);
        program[0] = 1; // PUSH
        program[1] = uint256(msg.sender);
        program[2] = 8; // STORE
        execute(program, state);
    }
    
    function deposit() public payable {
        uint256[] memory program = new uint256[](7);
        program[0] = 1; // PUSH
        program[1] = uint256(msg.sender);
        program[2] = 7; // LOAD
        program[3] = 1; // PUSH
        program[4] = msg.value;
        program[5] = 3; // ADD
        program[6] = 8; // STORE
        execute(program, state);
    }
    
    function withdraw(uint256 amount) public {
        uint256[] memory program = new uint256[](10);
        program[0] = 1; // PUSH
        program[1] = uint256(msg.sender);
        program[2] = 7; // LOAD
        program[3] = 1; // PUSH
        program[4] = amount;
        program[5] = 4; // SUB
        program[6] = 6; // JZ
        program[7] = 9; // TRANSFER
        program[8] = 1; // PUSH
        program[9] = state.deposits[msg.sender];
        execute(program, state);
    }
}