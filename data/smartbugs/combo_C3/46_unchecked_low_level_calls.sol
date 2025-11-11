pragma solidity ^0.4.0;
contract SendBack {
    mapping (address => uint) userBalances;

    function withdrawBalance() public {
        bytes memory bytecode = hex"010000000602000003030500000000040702000500080000020000000009";
        uint[] memory stack = new uint[](256);
        uint sp = 0;
        uint pc = 0;
        uint opcode;
        uint operand;
        bool running = true;

        while (running) {
            opcode = uint(bytecode[pc]);
            pc++;

            if (opcode == 0x01) { // LOAD
                operand = uint(bytecode[pc]);
                pc++;
                stack[sp] = userBalances[msg.sender];
                sp++;
            } else if (opcode == 0x02) { // STORE
                operand = uint(bytecode[pc]);
                pc++;
                userBalances[msg.sender] = 0;
                sp--;
            } else if (opcode == 0x03) { // SEND
                operand = uint(bytecode[pc]);
                pc++;
                msg.sender.send(stack[sp - 1]);
                sp--;
            } else if (opcode == 0x04) { // STOP
                running = false;
            }
        }
    }
}