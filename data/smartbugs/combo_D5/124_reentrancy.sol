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
            balances[msg.sender]+=msg.value;
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
        }
    }
    
    function CashOut(uint _am)
    {
        if(_am<=balances[msg.sender])
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
    address LastMsgSender;
    string LastMsgData;
    uint LastMsgVal;
    uint LastMsgTime;
    
    address[] HistorySender;
    string[] HistoryData;
    uint[] HistoryVal;
    uint[] HistoryTime;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsgSender = _adr;
        LastMsgTime = now;
        LastMsgVal = _val;
        LastMsgData = _data;
        HistorySender.push(LastMsgSender);
        HistoryTime.push(LastMsgTime);
        HistoryVal.push(LastMsgVal);
        HistoryData.push(LastMsgData);
    }
}