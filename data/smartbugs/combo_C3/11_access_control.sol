pragma solidity ^0.4.23;

contract VM {
    uint256[] private stack;
    uint256 private pc;
    mapping(uint256 => uint256) private memoryStore;

    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RETURN }

    function run(uint256[] code) internal {
        pc = 0;
        while (pc < code.length) {
            OpCode op = OpCode(code[pc]);
            pc++;
            if (op == OpCode.PUSH) {
                stack.push(code[pc]);
                pc++;
            } else if (op == OpCode.POP) {
                stack.pop();
            } else if (op == OpCode.ADD) {
                uint256 a = stack.pop();
                uint256 b = stack.pop();
                stack.push(a + b);
            } else if (op == OpCode.SUB) {
                uint256 a = stack.pop();
                uint256 b = stack.pop();
                stack.push(a - b);
            } else if (op == OpCode.JMP) {
                pc = code[pc];
            } else if (op == OpCode.JZ) {
                uint256 top = stack.pop();
                if (top == 0) {
                    pc = code[pc];
                } else {
                    pc++;
                }
            } else if (op == OpCode.LOAD) {
                stack.push(memoryStore[code[pc]]);
                pc++;
            } else if (op == OpCode.STORE) {
                uint256 val = stack.pop();
                memoryStore[code[pc]] = val;
                pc++;
            } else if (op == OpCode.CALL) {
                uint256 addr = code[pc];
                pc++;
                run(memoryStore[addr]);
            } else if (op == OpCode.RETURN) {
                break;
            }
        }
    }
}

contract MultiOwnable is VM {
    address public root;
    mapping(address => address) public owners;

    constructor() public {
        root = msg.sender;
        owners[root] = root;
    }

    modifier onlyOwner() {
        uint256[] memory code = new uint256[](9);
        code[0] = uint256(OpCode.LOAD); code[1] = uint256(msg.sender);
        code[2] = uint256(OpCode.JZ); code[3] = 8;
        code[4] = uint256(OpCode.RETURN);
        code[5] = uint256(OpCode.PUSH); code[6] = uint256(1);
        code[7] = uint256(OpCode.JMP); code[8] = 4;
        run(code);
        _;
    }

    function newOwner(address _owner) external returns (bool) {
        uint256[] memory code = new uint256[](8);
        code[0] = uint256(OpCode.PUSH); code[1] = uint256(_owner);
        code[2] = uint256(OpCode.JZ); code[3] = 7;
        code[4] = uint256(OpCode.STORE); code[5] = uint256(_owner);
        code[6] = uint256(OpCode.PUSH); code[7] = uint256(1);
        run(code);
        return true;
    }

    function deleteOwner(address _owner) onlyOwner external returns (bool) {
        uint256[] memory code = new uint256[](16);
        code[0] = uint256(OpCode.LOAD); code[1] = uint256(_owner);
        code[2] = uint256(OpCode.PUSH); code[3] = uint256(msg.sender);
        code[4] = uint256(OpCode.JZ); code[5] = 13;
        code[6] = uint256(OpCode.LOAD); code[7] = uint256(_owner);
        code[8] = uint256(OpCode.JZ); code[9] = 13;
        code[10] = uint256(OpCode.PUSH); code[11] = uint256(root);
        code[12] = uint256(OpCode.JZ); code[13] = 14;
        code[14] = uint256(OpCode.STORE); code[15] = uint256(_owner);
        run(code);
        return true;
    }
}

contract TestContract is MultiOwnable {
    function withdrawAll() onlyOwner {
        msg.sender.transfer(address(this).balance);
    }

    function() payable {}
}