pragma solidity ^0.4.19;

contract ETH_VAULT
{
    mapping (address => uint) private balancesData;
    function getBalance(address _addr) internal view returns (uint) { return balancesData[_addr]; }
    function setBalance(address _addr, uint _value) internal { balancesData[_addr] = _value; }
    
    Log TransferLog;
    
    function getMinDeposit() internal pure returns (uint) { return 1 ether; }
    
    function ETH_VAULT(address _log)
    public 
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value > getMinDeposit())
        {
            setBalance(msg.sender, getBalance(msg.sender) + msg.value);
            TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am <= getBalance(msg.sender))
        {
            if(msg.sender.call.value(_am)())
            {
                setBalance(msg.sender, getBalance(msg.sender) - _am);
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
    
    Message[] private HistoryData;
    function getHistory() public view returns (Message[]) { return HistoryData; }
    function pushHistory(Message _msg) internal { HistoryData.push(_msg); }
    
    Message private lastMsgData;
    function getLastMsg() internal view returns (Message) { return lastMsgData; }
    function setLastMsg(address _sender, uint _val, string _data, uint _time) internal {
        lastMsgData.Sender = _sender;
        lastMsgData.Val = _val;
        lastMsgData.Data = _data;
        lastMsgData.Time = _time;
    }
    
    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        setLastMsg(_adr, _val, _data, now);
        pushHistory(getLastMsg());
    }
}