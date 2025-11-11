pragma solidity ^0.4.19;

contract MONEY_BOX   
{
    mapping (address => uint) public Acc_unlockTime;
    mapping (address => uint) public Acc_balance;
    
    uint public MinSum;
    
    Log LogFile;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized)throw;
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized)throw;
        LogFile = Log(_log);
    }
    
    function Initialized()
    public
    {
        intitalized = true;
    }
    
    function Put(uint _lockTime)
    public
    payable
    {
        Acc_balance[msg.sender] += msg.value;
        if(now+_lockTime > Acc_unlockTime[msg.sender]) Acc_unlockTime[msg.sender] = now+_lockTime;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if( Acc_balance[msg.sender] >= MinSum && Acc_balance[msg.sender] >= _am && now > Acc_unlockTime[msg.sender])
        {
            if(msg.sender.call.value(_am)())
            {
                Acc_balance[msg.sender] -= _am;
                LogFile.AddMessage(msg.sender,_am,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Put(0);
    }
}

contract Log 
{
    address[] public History_Sender;
    string[] public History_Data;
    uint[] public History_Val;
    uint[] public History_Time;
    
    address LastMsg_Sender;
    string  LastMsg_Data;
    uint LastMsg_Val;
    uint  LastMsg_Time;
    
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