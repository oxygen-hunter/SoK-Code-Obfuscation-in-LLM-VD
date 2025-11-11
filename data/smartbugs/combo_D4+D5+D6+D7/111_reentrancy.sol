pragma solidity ^0.4.19;

contract PERSONAL_BANK
{
    mapping (address=>uint256) private balances;   
    
    struct MinStruct {
        LogFile Log;
        uint MinSum;
        bool intitalized;
    }
    
    MinStruct private minStruct = MinStruct(LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46), 1 ether, false);
    
    function SetMinSum(uint _val)
    public
    {
        if(minStruct.intitalized)revert();
        minStruct.MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(minStruct.intitalized)revert();
        minStruct.Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        minStruct.intitalized = true;
    }
    
    function Deposit()
    public
    payable
    {
        balances[msg.sender]+= msg.value;
        minStruct.Log.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        uint tempBalance = balances[msg.sender];
        if(tempBalance >= minStruct.MinSum && tempBalance >= _am)
        {
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender] -= _am;
                minStruct.Log.AddMessage(msg.sender,_am,"Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
    }
    
}

contract LogFile
{
    struct Message
    {
        uint Val;
        uint Time;
        string Data;
        address Sender;
    }
    
    Message[] public History;
    Message LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}