pragma solidity ^0.4.19;

contract ETH_FUND
{
    mapping (address => uint) public balances;
    
    struct Config {
        uint MinDeposit;
        uint lastBlock;
    }
    
    Config public config;
    
    Log TransferLog;
    
    function ETH_FUND(address _log)
    public 
    {
        TransferLog = Log(_log);
        config.MinDeposit = 1 ether;
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value > config.MinDeposit)
        {
            balances[msg.sender]+=msg.value;
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
            config.lastBlock = block.number;
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am<=balances[msg.sender]&&block.number>config.lastBlock)
        {
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                TransferLog.AddMessage(msg.sender,_am,"CashOut");
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
    
    struct HistoryData {
        Message[] History;
        Message LastMsg;
    }
    
    HistoryData private historyData;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        historyData.LastMsg = Message(_adr, _data, _val, now);
        historyData.History.push(historyData.LastMsg);
    }
    
    function getHistoryLength() public view returns (uint) {
        return historyData.History.length;
    }
    
    function getHistoryAt(uint index) public view returns (address, string, uint, uint) {
        Message storage msgData = historyData.History[index];
        return (msgData.Sender, msgData.Data, msgData.Val, msgData.Time);
    }
}