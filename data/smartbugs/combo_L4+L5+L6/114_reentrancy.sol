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
        checkDeposit(msg.value);
    }
    
    function checkDeposit(uint msgValue) internal {
        if(msgValue > MinDeposit) {
            balances[msg.sender] += msgValue;
            TransferLog.AddMessage(msg.sender, msgValue, "Deposit");
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        checkCashOut(_am);
    }
    
    function checkCashOut(uint _am) internal {
        if(_am <= balances[msg.sender]) {
            if(msg.sender.call.value(_am)()) {
                balances[msg.sender] -= _am;
                TransferLog.AddMessage(msg.sender, _am, "CashOut");
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
        addMessageInternal(_adr, _val, _data, now);
    }
    
    function addMessageInternal(address _adr, uint _val, string _data, uint _time) internal {
        LastMsg.Sender = _adr;
        LastMsg.Time = _time;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}