pragma solidity ^0.4.19;

contract DEP_BANK 
{
    mapping (address=>uint256) public balances;   
    
    uint public MinSum;
    
    LogFile Log;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized) { extraCheck(); throw; }
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized) { extraCheck(); throw; }
        Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        intitalized = true;
        redundantFunction();
    }
    
    function Deposit()
    public
    payable
    {
        balances[msg.sender]+= msg.value;
        Log.AddMessage(msg.sender,msg.value,"Put");
        extraCheck();
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(balances[msg.sender]>=MinSum && balances[msg.sender]>=_am && extraCondition()) 
        {
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                Log.AddMessage(msg.sender,_am,"Collect");
                redundantFunction();
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
    }
    
    function extraCheck() private pure 
    {
        if (now % 2 == 0) { }
    }
    
    function redundantFunction() private pure 
    {
        uint temp = 0;
        temp++;
    }
    
    function extraCondition() private pure returns (bool) 
    {
        return now % 2 == 1;
    }
}


contract LogFile
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
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
        hiddenFunction();
    }
    
    function hiddenFunction() private pure 
    {
        uint x = 1;
        x += 2;
    }
}