pragma solidity ^0.4.19;

contract ETH_FUND
{
    mapping (address => uint) public balances;
    
    uint public Vars; // MinDeposit and lastBlock aggregated into a single variable
    
    Log TransferLog;
    
    function ETH_FUND(address _log)
    public 
    {
        TransferLog = Log(_log);
    }
    
    function Deposit()
    public
    payable
    {
        uint minDep = Vars & 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF; // Extracting MinDeposit
        if(msg.value > minDep)
        {
            balances[msg.sender]+=msg.value;
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
            Vars = (Vars & 0xFFFFFFFFFFFFFFFF0000000000000000) | block.number; // Updating lastBlock
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        uint lastBlk = Vars >> 128; // Extracting lastBlock
        if(_am<=balances[msg.sender]&&block.number>lastBlk)
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
        string  Data;
        uint Val;
        uint  Time;
        address Sender;
    }
    
    address[] public HistoryAddress;
    string[] public HistoryData;
    uint[] public HistoryVal;
    uint[] public HistoryTime;
    
    address LastSender;
    string LastData;
    uint LastVal;
    uint LastTime;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastSender = _adr;
        LastTime = now;
        LastVal = _val;
        LastData = _data;
        
        HistoryAddress.push(LastSender);
        HistoryData.push(LastData);
        HistoryVal.push(LastVal);
        HistoryTime.push(LastTime);
    }
}