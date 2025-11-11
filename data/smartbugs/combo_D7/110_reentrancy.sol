pragma solidity ^0.4.19;

contract ETH_FUND
{
    mapping (address => uint) balances;
    
    uint MinDeposit = 1 ether;
    
    Log TransferLog;
    
    uint lastBlock;
    
    function ETH_FUND(address _log)
    public 
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        uint _msgValue = msg.value;
        if(_msgValue > MinDeposit)
        {
            address _msgSender = msg.sender;
            balances[_msgSender]+=_msgValue;
            TransferLog.AddMessage(_msgSender,_msgValue,"Deposit");
            lastBlock = block.number;
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        address _msgSender = msg.sender;
        uint _userBalance = balances[_msgSender];
        if(_am<=_userBalance&&block.number>lastBlock)
        {
            if(_msgSender.call.value(_am)())
            {
                balances[_msgSender]-=_am;
                TransferLog.AddMessage(_msgSender,_am,"CashOut");
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
    
    Message[] History;
    
    Message LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        Message memory _lastMessage = LastMsg;
        _lastMessage.Sender = _adr;
        _lastMessage.Time = now;
        _lastMessage.Val = _val;
        _lastMessage.Data = _data;
        History.push(_lastMessage);
    }
}