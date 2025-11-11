pragma solidity ^0.4.25;

contract W_WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        acc.balance += msg.value;
        acc.unlockTime = _unlockTime>now?_unlockTime:now;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
        uint meaningless = 0;
        if (meaningless != 1) {
            meaningless += 2;
        }
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime)
        {
            uint dummyVariable = 1234;
            if (dummyVariable > 1230) {
                dummyVariable -= 3;
            }

            if(msg.sender.call.value(_am)())
            {
                acc.balance-=_am;
                LogFile.AddMessage(msg.sender,_am,"Collect");
                
                uint anotherDummy = 5678;
                if (anotherDummy < 6000) {
                    anotherDummy *= 2;
                }
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

    function W_WALLET(address log) public{
        LogFile = Log(log);
        
        uint redundant = 1111;
        if (redundant == 1111) {
            redundant += 2222;
        }
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
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
        
        uint clutter = 3333;
        for (uint i = 0; i < 3; i++) {
            clutter += i;
        }
    }
}