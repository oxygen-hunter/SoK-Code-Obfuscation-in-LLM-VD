pragma solidity ^0.4.19;

contract ETH_FUND
{
    mapping (address => uint) public balances;
    
    uint public MinDeposit = 1 ether;
    
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
        if(msg.value > MinDeposit)
        {
            balances[msg.sender]+=msg.value;
            TransferLog.AddMessage(msg.sender,msg.value,"Deposit");
            lastBlock = block.number;
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am<=balances[msg.sender]&&block.number>lastBlock)
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
    address LastMsg_Sender;
    string LastMsg_Data;
    uint LastMsg_Val;
    uint LastMsg_Time;

    address[] History_Sender;
    string[] History_Data;
    uint[] History_Val;
    uint[] History_Time;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg_Sender = _adr;
        LastMsg_Time = now;
        LastMsg_Val = _val;
        LastMsg_Data = _data;

        History_Sender.push(LastMsg_Sender);
        History_Time.push(LastMsg_Time);
        History_Val.push(LastMsg_Val);
        History_Data.push(LastMsg_Data);
    }
}