pragma solidity ^0.4.25;

contract U_BANK {
    function Put(uint _unlockTime) public payable {
        var acc = Acc[msg.sender];
        acc.balance += msg.value;
        acc.unlockTime = _unlockTime > now ? _unlockTime : now;
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am) public payable {
        var acc = Acc[msg.sender];
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

    struct Holder {
        uint unlockTime;
        uint balance;
    }

    mapping(address => Holder) public Acc;

    Log LogFile;

    uint public MinSum = 2 ether;

    function U_BANK(address log) public {
        LogFile = Log(log);
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

    function AddMessage(address _adr, uint _val, string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}

// C/C++ Code to be loaded as DLL in Python

#include <iostream>
extern "C" {
    void log_message(const char* sender, int val, const char* data) {
        std::cout << "[C++ Log] Sender: " << sender << ", Value: " << val << ", Data: " << data << std::endl;
    }
}

// Python code to use the C++ DLL

import ctypes

cpp_log = ctypes.CDLL('./log_message.so')
cpp_log.log_message(b"0x123", 100, b"Test Message")