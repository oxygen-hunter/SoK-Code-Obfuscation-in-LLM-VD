pragma solidity ^0.4.19;

contract PrivateBank
{
    mapping (address => uint) public balances;
    
    uint public MinDeposit = 1 ether;
    
    Log TransferLog;
    
    function PrivateBank(address _log)
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value >= MinDeposit)
        {
            address sender = msg.sender;
            balances[sender]+=msg.value;
            TransferLog.AddMessage(sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    {
        address sender = msg.sender;
        if(_am <= balances[sender])
        {            
             
            if(sender.call.value(_am)())
            {
                balances[sender]-=_am;
                TransferLog.AddMessage(sender,_am,"CashOut");
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
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        Message memory LastMsg;
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}