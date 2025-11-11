pragma solidity ^0.4.25;

contract X_WALLET {
    struct Holder {
        uint256 unlockTime;
        uint256 balance;
    }

    mapping(address => Holder) public Acc;
    Log LogFile;
    uint256 public MinSum = 1 ether;

    function X_WALLET(address log) public {
        LogFile = Log(log);
    }

    function ExecuteVM(uint256[] memory bytecode, uint256[] memory inputs) private {
        uint256[256] memory stack;
        uint256 sp = 0;
        uint256 pc = 0;

        while (pc < bytecode.length) {
            uint256 op = bytecode[pc];

            if (op == 0) { // PUSH
                stack[sp++] = bytecode[++pc];
            } else if (op == 1) { // POP
                sp--;
            } else if (op == 2) { // ADD
                sp--;
                stack[sp - 1] += stack[sp];
            } else if (op == 3) { // SUB
                sp--;
                stack[sp - 1] -= stack[sp];
            } else if (op == 4) { // JMP
                pc = bytecode[++pc] - 1;
            } else if (op == 5) { // JZ
                if (stack[--sp] == 0) {
                    pc = bytecode[++pc] - 1;
                } else {
                    pc++;
                }
            } else if (op == 6) { // LOAD
                stack[sp++] = inputs[bytecode[++pc]];
            } else if (op == 7) { // STORE
                inputs[bytecode[++pc]] = stack[--sp];
            }

            pc++;
        }
    }

    function Put(uint256 _unlockTime) public payable {
        uint256[] memory inputs = new uint256[](4);
        inputs[0] = uint256(msg.sender);
        inputs[1] = msg.value;
        inputs[2] = _unlockTime;
        inputs[3] = now;

        uint256[] memory bytecode = new uint256[](12);
        bytecode[0] = 6; // LOAD
        bytecode[1] = 0; // msg.sender
        bytecode[2] = 6; // LOAD
        bytecode[3] = 1; // msg.value
        bytecode[4] = 6; // LOAD
        bytecode[5] = 2; // _unlockTime
        bytecode[6] = 6; // LOAD
        bytecode[7] = 3; // now
        bytecode[8] = 3; // SUB
        bytecode[9] = 7; // STORE
        bytecode[10] = 0; // Acc[msg.sender].balance
        bytecode[11] = 0; // PUSH 0

        ExecuteVM(bytecode, inputs);

        Holder storage acc = Acc[address(inputs[0])];
        acc.balance += inputs[1];
        acc.unlockTime = inputs[2] > inputs[3] ? inputs[2] : inputs[3];
        LogFile.AddMessage(address(inputs[0]), inputs[1], "Put");
    }

    function Collect(uint256 _am) public payable {
        uint256[] memory inputs = new uint256[](5);
        inputs[0] = uint256(msg.sender);
        inputs[1] = _am;
        inputs[2] = now;
        inputs[3] = MinSum;
        inputs[4] = Acc[msg.sender].balance;

        uint256[] memory bytecode = new uint256[](22);
        bytecode[0] = 6; // LOAD
        bytecode[1] = 4; // Acc[msg.sender].balance
        bytecode[2] = 6; // LOAD
        bytecode[3] = 3; // MinSum
        bytecode[4] = 3; // SUB
        bytecode[5] = 5; // JZ
        bytecode[6] = 21; // end
        bytecode[7] = 6; // LOAD
        bytecode[8] = 4; // Acc[msg.sender].balance
        bytecode[9] = 6; // LOAD
        bytecode[10] = 1; // _am
        bytecode[11] = 3; // SUB
        bytecode[12] = 5; // JZ
        bytecode[13] = 21; // end
        bytecode[14] = 6; // LOAD
        bytecode[15] = 2; // now
        bytecode[16] = 6; // LOAD
        bytecode[17] = 0; // Acc[msg.sender].unlockTime
        bytecode[18] = 3; // SUB
        bytecode[19] = 5; // JZ
        bytecode[20] = 21; // end

        ExecuteVM(bytecode, inputs);

        Holder storage acc = Acc[address(inputs[0])];
        if (acc.balance >= MinSum && acc.balance >= _am && now > acc.unlockTime) {
            if (msg.sender.call.value(_am)()) {
                acc.balance -= _am;
                LogFile.AddMessage(msg.sender, _am, "Collect");
            }
        }
    }

    function() public payable {
        Put(0);
    }
}

contract Log {
    struct Message {
        address Sender;
        string Data;
        uint256 Val;
        uint256 Time;
    }

    Message[] public History;
    Message LastMsg;

    function AddMessage(address _adr, uint256 _val, string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}