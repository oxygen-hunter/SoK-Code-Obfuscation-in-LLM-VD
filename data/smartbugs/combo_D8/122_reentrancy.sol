pragma solidity ^0.4.19;

contract Private_Bank
{
    mapping (address => uint) public balances;
    
    uint public MinDeposit = getMinDeposit();
    
    Log TransferLog;
    
    function Private_Bank(address _log)
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value > getDynamicMinDeposit())
        {
            balances[msg.sender] = getUpdatedBalance(msg.sender, msg.value);
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am<=balances[msg.sender])
        {
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]=getReducedBalance(msg.sender, _am);
                TransferLog.AddMessage(msg.sender,_am,"CashOut");
            }
        }
    }
    
    function() public payable{}    
    
    function getMinDeposit() internal pure returns (uint) {
        return 1 ether;
    }
    
    function getDynamicMinDeposit() internal view returns (uint) {
        return MinDeposit;
    }
    
    function getUpdatedBalance(address _adr, uint _val) internal view returns (uint) {
        return balances[_adr] + _val;
    }
    
    function getReducedBalance(address _adr, uint _val) internal view returns (uint) {
        return balances[_adr] - _val;
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
        LastMsg = getMessage(_adr, _val, _data);
        History.push(LastMsg);
    }
    
    function getMessage(address _adr, uint _val, string _data) internal view returns (Message) {
        Message memory msgStruct;
        msgStruct.Sender = _adr;
        msgStruct.Time = now;
        msgStruct.Val = _val;
        msgStruct.Data = _data;
        return msgStruct;
    }
}