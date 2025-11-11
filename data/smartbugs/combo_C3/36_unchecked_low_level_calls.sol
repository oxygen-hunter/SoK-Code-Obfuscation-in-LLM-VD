pragma solidity ^0.4.18;

contract VM {
    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, SENDER, VALUE, EQ, GT, SETGT, SETEQ, HALT }
    uint256[] stack;
    uint256 pc;
    uint256 instrPtr;
    address[] memoryAddresses;
    address owner;
    address newOwner;
    address sender;
    uint256 value;
    uint256 minDeposit;
    mapping (address => uint) public Holders;

    function execute(uint256[] program, address[] addrs, uint256 inputValue) internal {
        stack = new uint256[](256);
        pc = 0;
        instrPtr = 0;
        memoryAddresses = addrs;
        sender = msg.sender;
        value = inputValue;
        while (instrPtr < program.length) {
            OpCode opCode = OpCode(program[instrPtr++]);
            if (opCode == OpCode.PUSH) {
                stack[pc++] = program[instrPtr++];
            } else if (opCode == OpCode.POP) {
                pc--;
            } else if (opCode == OpCode.ADD) {
                stack[pc - 2] += stack[pc - 1];
                pc--;
            } else if (opCode == OpCode.SUB) {
                stack[pc - 2] -= stack[pc - 1];
                pc--;
            } else if (opCode == OpCode.JMP) {
                instrPtr = program[instrPtr];
            } else if (opCode == OpCode.JZ) {
                if (stack[--pc] == 0) {
                    instrPtr = program[instrPtr];
                } else {
                    instrPtr++;
                }
            } else if (opCode == OpCode.LOAD) {
                stack[pc++] = Holders[memoryAddresses[program[instrPtr++]]];
            } else if (opCode == OpCode.STORE) {
                Holders[memoryAddresses[program[instrPtr++]]] = stack[--pc];
            } else if (opCode == OpCode.CALL) {
                address target = memoryAddresses[program[instrPtr++]];
                bytes4 sig = bytes4(program[instrPtr++]);
                uint256 amt = stack[--pc];
                target.call(sig, memoryAddresses[program[instrPtr++]], amt);
            } else if (opCode == OpCode.SENDER) {
                stack[pc++] = uint256(sender);
            } else if (opCode == OpCode.VALUE) {
                stack[pc++] = value;
            } else if (opCode == OpCode.EQ) {
                uint256 a = stack[--pc];
                uint256 b = stack[--pc];
                stack[pc++] = (a == b) ? 1 : 0;
            } else if (opCode == OpCode.GT) {
                uint256 a = stack[--pc];
                uint256 b = stack[--pc];
                stack[pc++] = (a > b) ? 1 : 0;
            } else if (opCode == OpCode.SETGT) {
                memoryAddresses[program[instrPtr++]] = stack[--pc] > 0 ? sender : address(0);
            } else if (opCode == OpCode.SETEQ) {
                memoryAddresses[program[instrPtr++]] = stack[--pc] == 0 ? sender : address(0);
            } else if (opCode == OpCode.HALT) {
                break;
            }
        }
    }
}

contract Ownable is VM {
    function changeOwner(address addr) public {
        uint256[] memory program = new uint256[](9);
        program[0] = uint256(OpCode.SENDER);
        program[1] = uint256(OpCode.PUSH);
        program[2] = uint256(owner);
        program[3] = uint256(OpCode.EQ);
        program[4] = uint256(OpCode.JZ);
        program[5] = 8;
        program[6] = uint256(OpCode.PUSH);
        program[7] = uint256(addr);
        program[8] = uint256(OpCode.SETEQ);
        address[] memory addrs = new address[](1);
        addrs[0] = newOwner;
        execute(program, addrs, 0);
    }

    function confirmOwner() public {
        uint256[] memory program = new uint256[](9);
        program[0] = uint256(OpCode.SENDER);
        program[1] = uint256(OpCode.PUSH);
        program[2] = uint256(newOwner);
        program[3] = uint256(OpCode.EQ);
        program[4] = uint256(OpCode.JZ);
        program[5] = 8;
        program[6] = uint256(OpCode.PUSH);
        program[7] = uint256(newOwner);
        program[8] = uint256(OpCode.SETEQ);
        address[] memory addrs = new address[](1);
        addrs[0] = owner;
        execute(program, addrs, 0);
    }
}

contract Token is Ownable {
    function WithdrawToken(address token, uint256 amount, address to) public {
        uint256[] memory program = new uint256[](9);
        program[0] = uint256(OpCode.SENDER);
        program[1] = uint256(OpCode.PUSH);
        program[2] = uint256(owner);
        program[3] = uint256(OpCode.EQ);
        program[4] = uint256(OpCode.JZ);
        program[5] = 8;
        program[6] = uint256(OpCode.PUSH);
        program[7] = uint256(amount);
        program[8] = uint256(OpCode.CALL);
        address[] memory addrs = new address[](3);
        addrs[0] = token;
        addrs[1] = to;
        addrs[2] = bytes4(sha3("transfer(address,uint256)"));
        execute(program, addrs, 0);
    }
}

contract TokenBank is Token {
    function initTokenBank() public {
        uint256[] memory program = new uint256[](3);
        program[0] = uint256(OpCode.SENDER);
        program[1] = uint256(OpCode.PUSH);
        program[2] = uint256(1 ether);
        address[] memory addrs = new address[](1);
        addrs[0] = owner;
        execute(program, addrs, 0);
    }

    function() payable {
        Deposit();
    }

    function Deposit() payable {
        uint256[] memory program = new uint256[](12);
        program[0] = uint256(OpCode.VALUE);
        program[1] = uint256(OpCode.PUSH);
        program[2] = uint256(minDeposit);
        program[3] = uint256(OpCode.GT);
        program[4] = uint256(OpCode.JZ);
        program[5] = 11;
        program[6] = uint256(OpCode.LOAD);
        program[7] = uint256(OpCode.PUSH);
        program[8] = uint256(msg.value);
        program[9] = uint256(OpCode.ADD);
        program[10] = uint256(OpCode.STORE);
        address[] memory addrs = new address[](1);
        addrs[0] = msg.sender;
        execute(program, addrs, msg.value);
    }

    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public {
        uint256[] memory program = new uint256[](12);
        program[0] = uint256(OpCode.LOAD);
        program[1] = uint256(OpCode.JZ);
        program[2] = 11;
        program[3] = uint256(OpCode.PUSH);
        program[4] = uint256(0);
        program[5] = uint256(OpCode.STORE);
        program[6] = uint256(OpCode.PUSH);
        program[7] = uint256(_amount);
        program[8] = uint256(OpCode.CALL);
        address[] memory addrs = new address[](3);
        addrs[0] = _token;
        addrs[1] = _to;
        addrs[2] = bytes4(sha3("transfer(address,uint256)"));
        execute(program, addrs, 0);
    }

    function WithdrawToHolder(address _addr, uint _wei) public payable {
        uint256[] memory program = new uint256[](18);
        program[0] = uint256(OpCode.LOAD);
        program[1] = uint256(OpCode.JZ);
        program[2] = 17;
        program[3] = uint256(OpCode.LOAD);
        program[4] = uint256(OpCode.PUSH);
        program[5] = uint256(_wei);
        program[6] = uint256(OpCode.GT);
        program[7] = uint256(OpCode.JZ);
        program[8] = 17;
        program[9] = uint256(OpCode.PUSH);
        program[10] = uint256(_wei);
        program[11] = uint256(OpCode.CALL);
        program[12] = uint256(OpCode.LOAD);
        program[13] = uint256(OpCode.SUB);
        program[14] = uint256(OpCode.STORE);
        address[] memory addrs = new address[](2);
        addrs[0] = _addr;
        addrs[1] = msg.sender;
        execute(program, addrs, _wei);
    }
}