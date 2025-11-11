pragma solidity ^0.4.16;

contract Owned {
    modifier onlyOwner() {
        require(msg.sender == owner);
        _;
    }

    address public owner;
    address public newOwner;

    function Owned() {
        owner = msg.sender;
    }

    function changeOwner(address _newOwner) onlyOwner {
        newOwner = _newOwner;
    }

    function acceptOwnership() {
        if (msg.sender == newOwner) {
            owner = newOwner;
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }
}

contract MarriageVM {
    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, STORE, LOAD, CALL, RETURN }
    struct Instruction { Opcode opcode; uint[] operands; }

    uint[] stack;
    uint[] memory;
    uint pc;
    bool halted;
    address owner;
    string public partner1;
    string public partner2;
    uint public marriageDate;
    string public marriageStatus;
    string public vows;
    Event[] public majorEvents;
    Message[] public messages;

    struct Event { uint date; string name; string description; string url; }
    struct Message { uint date; string nameFrom; string text; string url; uint value; }

    modifier onlyOwner() { require(msg.sender == owner); _; }
    modifier areMarried { require(sha3(marriageStatus) == sha3("Married")); _; }

    function MarriageVM(address _owner) {
        owner = _owner;
    }

    function executeProgram(Instruction[] program) internal {
        while (!halted && pc < program.length) {
            Instruction memory inst = program[pc];
            executeInstruction(inst);
            pc++;
        }
    }

    function executeInstruction(Instruction memory inst) internal {
        if (inst.opcode == Opcode.PUSH) {
            stack.push(inst.operands[0]);
        } else if (inst.opcode == Opcode.POP) {
            stack.pop();
        } else if (inst.opcode == Opcode.ADD) {
            uint a = stack.pop();
            uint b = stack.pop();
            stack.push(a + b);
        } else if (inst.opcode == Opcode.SUB) {
            uint a = stack.pop();
            uint b = stack.pop();
            stack.push(a - b);
        } else if (inst.opcode == Opcode.JMP) {
            pc = inst.operands[0] - 1;
        } else if (inst.opcode == Opcode.JZ) {
            if (stack.pop() == 0) pc = inst.operands[0] - 1;
        } else if (inst.opcode == Opcode.STORE) {
            memory[inst.operands[0]] = stack.pop();
        } else if (inst.opcode == Opcode.LOAD) {
            stack.push(memory[inst.operands[0]]);
        } else if (inst.opcode == Opcode.CALL) {
            owner.call.value(stack.pop())(abi.encodeWithSignature("execute(address,uint256,bytes)", stack.pop()));
        } else if (inst.opcode == Opcode.RETURN) {
            halted = true;
        }
    }

    function createMarriage(string _partner1, string _partner2, string _vows, string url) onlyOwner {
        Instruction[] memory program = new Instruction[](6);
        program[0] = Instruction(Opcode.PUSH, [uint(0)]);
        program[1] = Instruction(Opcode.PUSH, [uint(keccak256(abi.encodePacked(partner1, partner2)))]);
        program[2] = Instruction(Opcode.ADD, new uint[](0));
        program[3] = Instruction(Opcode.STORE, [uint(0)]);
        program[4] = Instruction(Opcode.PUSH, [uint(now)]);
        program[5] = Instruction(Opcode.STORE, [uint(1)]);
        executeProgram(program);

        partner1 = _partner1;
        partner2 = _partner2;
        marriageDate = now;
        vows = _vows;
        marriageStatus = "Married";
        majorEvents.push(Event(now, "Marriage", vows, url));
        MajorEvent("Marriage", vows, url);
    }

    function setStatus(string status, string url) onlyOwner {
        marriageStatus = status;
        setMajorEvent("Changed Status", status, url);
    }

    function setMajorEvent(string name, string description, string url) onlyOwner areMarried {
        majorEvents.push(Event(now, name, description, url));
        MajorEvent(name, description, url);
    }

    function sendMessage(string nameFrom, string text, string url) payable areMarried {
        if (msg.value > 0) {
            owner.transfer(this.balance);
        }
        messages.push(Message(now, nameFrom, text, url, msg.value));
        MessageSent(nameFrom, text, url, msg.value);
    }

    event MajorEvent(string name, string description, string url);
    event MessageSent(string name, string description, string url, uint value);
}