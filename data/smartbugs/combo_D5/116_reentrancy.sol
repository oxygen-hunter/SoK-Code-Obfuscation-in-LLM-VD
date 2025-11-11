pragma solidity ^0.4.25;

contract WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var accUnlockTime = Acc[msg.sender].unlockTime;
        var accBalance = Acc[msg.sender].balance;
        accBalance += msg.value;
        accUnlockTime = _unlockTime>now?_unlockTime:now;
        Acc[msg.sender].unlockTime = accUnlockTime;
        Acc[msg.sender].balance = accBalance;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var accUnlockTime = Acc[msg.sender].unlockTime;
        var accBalance = Acc[msg.sender].balance;
        if( accBalance>=MinSum && accBalance>=_am && now>accUnlockTime)
        {
            if(msg.sender.call.value(_am)())
            {
                accBalance-=_am;
                Acc[msg.sender].balance = accBalance;
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

    address LastMsgSender;
    string LastMsgData;
    uint LastMsgVal;
    uint LastMsgTime;

    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsgSender = _adr;
        LastMsgTime = now;
        LastMsgVal = _val;
        LastMsgData = _data;
        History.push(Message(LastMsgSender, LastMsgData, LastMsgVal, LastMsgTime));
    }
}