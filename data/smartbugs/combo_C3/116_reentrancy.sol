pragma solidity ^0.4.25;

contract WALLET {
    struct Holder {
        uint unlockTime;
        uint balance;
    }

    mapping(address => Holder) public Acc;
    Log LogFile;
    uint public MinSum = 1 ether;

    function WALLET(address log) public {
        LogFile = Log(log);
    }
    
    function Put(uint _unlockTime) public payable {
        executeVM(1, [uint(msg.sender), msg.value, _unlockTime, now]);
    }
    
    function Collect(uint _am) public payable {
        executeVM(2, [uint(msg.sender), _am, now]);
    }
    
    function() public payable {
        executeVM(1, [uint(msg.sender), msg.value, 0, now]);
    }

    function executeVM(uint operation, uint[] args) internal {
        uint stackPointer;
        uint[10] memory stack;
        uint programCounter;
        uint instruction;
        uint[3] memory registers;

        if (operation == 1) { // Put
            programCounter = 0;
            uint sender = args[0];
            uint value = args[1];
            uint unlockTime = args[2];
            uint currentTime = args[3];
            
            while (programCounter < 7) {
                instruction = programCounter;
                if (instruction == 0) {
                    stack[stackPointer++] = sender;
                } else if (instruction == 1) {
                    stack[stackPointer++] = value;
                } else if (instruction == 2) {
                    stack[stackPointer++] = Acc[address(stack[--stackPointer])].balance;
                } else if (instruction == 3) {
                    stack[stackPointer - 1] += stack[stackPointer];
                } else if (instruction == 4) {
                    Acc[address(stack[--stackPointer])].balance = stack[stackPointer];
                } else if (instruction == 5) {
                    stack[stackPointer++] = unlockTime > currentTime ? unlockTime : currentTime;
                } else if (instruction == 6) {
                    Acc[address(stack[--stackPointer])].unlockTime = stack[stackPointer];
                } else if (instruction == 7) {
                    LogFile.AddMessage(address(stack[--stackPointer]), stack[--stackPointer], "Put");
                }
                programCounter++;
            }

        } else if (operation == 2) { // Collect
            programCounter = 0;
            uint sender = args[0];
            uint _am = args[1];
            uint currentTime = args[2];

            while (programCounter < 13) {
                instruction = programCounter;
                if (instruction == 0) {
                    stack[stackPointer++] = sender;
                } else if (instruction == 1) {
                    stack[stackPointer++] = Acc[address(stack[--stackPointer])].balance;
                } else if (instruction == 2) {
                    uint balance = stack[--stackPointer];
                    if (balance < MinSum) programCounter = 12; // Exit if balance < MinSum
                } else if (instruction == 3) {
                    if (stack[--stackPointer] < _am) programCounter = 12; // Exit if balance < _am
                } else if (instruction == 4) {
                    if (currentTime <= Acc[address(stack[--stackPointer])].unlockTime) programCounter = 12; // Exit if now <= unlockTime
                } else if (instruction == 5) {
                    if (address(uint160(stack[--stackPointer])).call.value(_am)()) {
                        programCounter = 6;
                    } else {
                        programCounter = 12; // Exit if call failed
                    }
                } else if (instruction == 6) {
                    stack[stackPointer++] = _am;
                } else if (instruction == 7) {
                    Acc[address(stack[--stackPointer])].balance -= stack[stackPointer];
                } else if (instruction == 8) {
                    LogFile.AddMessage(address(stack[--stackPointer]), stack[--stackPointer], "Collect");
                } else if (instruction == 9) {
                    programCounter = 12; // Exit
                }
                programCounter++;
            }
        }
    }
}

contract Log {
    struct Message {
        address Sender;
        string Data;
        uint Val;
        uint Time;
    }

    Message[] public History;
    Message LastMsg;

    function AddMessage(address _adr,uint _val,string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}