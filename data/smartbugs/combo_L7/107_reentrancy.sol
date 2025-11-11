pragma solidity ^0.4.25;

contract X_WALLET {
    function Put(uint _unlockTime) public payable {
        assembly {
            let acc := sload(Acc_slot)
            acc := add(acc, msg.value)
            let unlockTime := sload(add(acc, 1))
            unlockTime := or(mul(gt(_unlockTime, timestamp), _unlockTime), mul(1, timestamp))
            sstore(add(acc, 1), unlockTime)
        }
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am) public payable {
        assembly {
            let acc := sload(Acc_slot)
            let balance := sload(acc)
            if and(and(ge(balance, MinSum_slot), ge(balance, _am)), gt(timestamp, sload(add(acc, 1)))) {
                if call(gas, caller, _am, 0, 0, 0, 0) {
                    sstore(acc, sub(balance, _am))
                    pop(call(gas, LogFile_slot, 0, add(0x20, msg.data), 0x80, 0, 0))
                }
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

    uint public MinSum = 1 ether;

    function X_WALLET(address log) public {
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
        assembly {
            sstore(LastMsg_slot, _adr)
            sstore(add(LastMsg_slot, 1), timestamp)
            sstore(add(LastMsg_slot, 2), _val)
            sstore(add(LastMsg_slot, 3), _data)
            sstore(add(History_slot, sload(History_length_slot)), LastMsg_slot)
            sstore(History_length_slot, add(sload(History_length_slot), 1))
        }
    }
}