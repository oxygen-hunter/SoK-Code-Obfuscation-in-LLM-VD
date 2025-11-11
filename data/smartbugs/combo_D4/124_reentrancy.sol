pragma solidity ^0.4.19;

contract PrivateBank
{
    mapping (address => uint) public balances;
    
    struct BankData {
        uint MinDeposit;
        Log TransferLog;
    }
    
    BankData private bd;
    
    function PrivateBank(address _log)
    {
        bd.MinDeposit = 1 ether;
        bd.TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value >= bd.MinDeposit)
        {
            balances[msg.sender] += msg.value;
            bd.TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
        }
    }
    
    function CashOut(uint _am)
    {
        if(_am <= balances[msg.sender])
        {            
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender] -= _am;
                bd.TransferLog.AddMessage(msg.sender, _am, "CashOut");
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
        uint Time;
    }
    
    struct LogData {
        Message[] History;
        Message LastMsg;
    }
    
    LogData private ld;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        ld.LastMsg.Sender = _adr;
        ld.LastMsg.Time = now;
        ld.LastMsg.Val = _val;
        ld.LastMsg.Data = _data;
        ld.History.push(ld.LastMsg);
    }
}