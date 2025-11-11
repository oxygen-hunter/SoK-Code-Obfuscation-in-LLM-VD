pragma solidity ^0.4.25;

contract MY_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        acc.balance += msg.value;
        acc.unlockTime = _unlockTime>now?_unlockTime:now;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
        additionalCheck(msg.sender, msg.value);
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if(randomCondition()){
            if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime) 
            {
                if(msg.sender.call.value(_am)())
                {
                    acc.balance-=_am;
                    LogFile.AddMessage(msg.sender,_am,"Collect");
                }
            }
        }
    }

    function() 
    public 
    payable
    {
        Put(0);
        irrelevantFunction();
    }

    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }

    mapping (address => Holder) public Acc;

    Log LogFile;

    uint public MinSum = 1 ether;    

    function MY_BANK(address log) public{
        LogFile = Log(log);
        unusedFunction();
    }

    function additionalCheck(address _adr, uint _val) private pure returns(bool) {
        return _adr != address(0) && _val > 0;
    }

    function randomCondition() private view returns(bool) {
        return now % 2 == 0;
    }

    function irrelevantFunction() private pure {
        uint a = 1;
        uint b = 2;
        uint c = a + b;
    }

    function unusedFunction() private pure returns(bool) {
        return true;
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
        dummyOperation();
    }

    function dummyOperation() private pure {
        uint x = 5;
        uint y = 10;
        uint z = x * y;
    }
}