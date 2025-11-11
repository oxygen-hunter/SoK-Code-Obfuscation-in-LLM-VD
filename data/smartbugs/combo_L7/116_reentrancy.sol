pragma solidity ^0.4.25;

contract WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        assembly {
            let acc := sload(0x0)
            mstore(add(acc, caller), add(mload(add(acc, caller)), callvalue))
            let unlockTime := _unlockTime
            if lt(unlockTime, timestamp) { unlockTime := timestamp }
            sstore(acc, unlockTime)
        }
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime)
        {
            if(msg.sender.call.value(_am)())
            {
                acc.balance-=_am;
                LogFile.AddMessage(msg.sender,_am,"Collect");
            }
        }
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

    uint public MinSum = 1 ether;    

    function WALLET(address log) public{
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
            sstore(0x0, _adr)
            sstore(0x1, timestamp)
            sstore(0x2, _val)
            sstore(0x3, _data)
        }
        History.push(LastMsg);
    }
}