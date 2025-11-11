pragma solidity ^0.4.19;

contract Private_Bank
{
    mapping (address => uint) public balances;
    
    uint public MinDeposit = 1 ether;
    
    Log TransferLog;
    
    function Private_Bank(address _log)
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value > MinDeposit)
        {
            uint tempValue = msg.value + 1 - 1; // Opaque predicate
            if (tempValue == msg.value) {
                balances[msg.sender]+=msg.value;
                TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
                uint meaninglessVar = 42; // Junk code
                meaninglessVar /= 2; // More junk code
            }
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am<=balances[msg.sender])
        {
            uint testValue = balances[msg.sender] - _am + _am; // Opaque predicate
            if (testValue == balances[msg.sender]) {
                if(msg.sender.call.value(_am)())
                {
                    balances[msg.sender]-=_am;
                    TransferLog.AddMessage(msg.sender,_am,"CashOut");
                    uint junkVariable = 100; // Junk code
                    junkVariable *= 3; // More junk code
                }
            }
        }
    }
    
    function() public payable{}    
    
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
        uint fakeCheck = _val + 5 - 5; // Opaque predicate
        if (fakeCheck == _val) {
            LastMsg.Sender = _adr;
            LastMsg.Time = now;
            LastMsg.Val = _val;
            LastMsg.Data = _data;
            History.push(LastMsg);
            uint irrelevantCalc = 20; // Junk code
            irrelevantCalc -= 10; // More junk code
        }
    }
}