pragma solidity ^0.4.25;

contract U_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        assembly {
            let acc := sload(Acc_slot)
            mstore(0x80, caller)
            mstore(0xa0, acc)
            acc := keccak256(0x80, 0x40)
            let balance := sload(add(acc, 1))
            balance := add(balance, callvalue)
            sstore(add(acc, 1), balance)
            let unlockTime := sload(acc)
            unlockTime := gt(_unlockTime, timestamp) ? _unlockTime : timestamp
            sstore(acc, unlockTime)
        }
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        assembly {
            let acc := sload(Acc_slot)
            mstore(0x80, caller)
            mstore(0xa0, acc)
            acc := keccak256(0x80, 0x40)
            let balance := sload(add(acc, 1))
            if and(and(ge(balance, sload(MinSum_slot)), ge(balance, _am)), gt(timestamp, sload(acc))) {
                if call(gas, caller, _am, 0, 0, 0, 0) {
                    balance := sub(balance, _am)
                    sstore(add(acc, 1), balance)
                }
            }
        }
        LogFile.AddMessage(msg.sender, _am, "Collect");
    }

    function() 
    public 
    payable
    {
        Put(0);
    }

    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }

    mapping (address => Holder) public Acc;

    Log LogFile;

    uint public MinSum = 2 ether;    

    function U_BANK(address log) public{
        LogFile = Log(log);
    }
}


contract Log 
{
    struct Message
    {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }

    Message[] public History;

    Message LastMsg;

    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        assembly {
            mstore(0x80, _adr)
            mstore(0xa0, _val)
            mstore(0xc0, _data)
            sstore(LastMsg_slot, _adr)
            sstore(add(LastMsg_slot, 1), timestamp)
            sstore(add(LastMsg_slot, 2), _val)
            sstore(add(LastMsg_slot, 3), _data)
        }
        History.push(LastMsg);
    }
}