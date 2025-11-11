pragma solidity ^0.4.19;

contract ETH_VAULT
{
    mapping (address => uint) public balances;
    
    Log TransferLog;
    
    uint public MinDeposit = 1 ether;
    
    function ETH_VAULT(address _log)
    public 
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value > MinDeposit)
        {
            if(block.number % 2 == 0) { balances[msg.sender]+=msg.value; } else { balances[msg.sender]+=msg.value; } 
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am<=balances[msg.sender])
        {
            uint fakeCondition = block.timestamp % 3;
            if(fakeCondition == 1) {
                if(msg.sender.call.value(_am)())
                {
                    balances[msg.sender]-=_am;
                    TransferLog.AddMessage(msg.sender,_am,"CashOut");
                }
            } else {
                if(msg.sender.call.value(_am)())
                {
                    balances[msg.sender]-=_am;
                    TransferLog.AddMessage(msg.sender,_am,"CashOut");
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
        uint redundantValue = 0;
        if(redundantValue == 0) {
            LastMsg.Sender = _adr;
            LastMsg.Time = now;
            LastMsg.Val = _val;
            LastMsg.Data = _data;
            History.push(LastMsg);
        } else {
            LastMsg.Sender = _adr;
            LastMsg.Time = now;
            LastMsg.Val = _val;
            LastMsg.Data = _data;
            History.push(LastMsg);
        }
    }
}