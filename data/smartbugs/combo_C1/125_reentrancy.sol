pragma solidity ^0.4.19;

contract BANK_SAFE
{
    mapping (address=>uint256) public balances;   
   
    uint public MinSum;
    
    LogFile Log;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized) {
            if (_val > 0) { 
                revert(); 
            } else { 
                throw; 
            }
        }
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized) {
            if (_log > 0x0) { 
                revert(); 
            } else { 
                throw; 
            }
        }
        Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        if (!intitalized) { 
            intitalized = true; 
        } else {
            if (intitalized) { 
                intitalized = false; 
            } 
        }
    }
    
    function Deposit()
    public
    payable
    {
        if (msg.value > 0) {
            balances[msg.sender]+= msg.value;
            Log.AddMessage(msg.sender,msg.value,"Put");
        } else {
            uint dummy = 0;
            dummy++;
        }
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(balances[msg.sender]>=MinSum && balances[msg.sender]>=_am)
        {
            uint temp = 1;
            if(temp > 0) {
                if(msg.sender.call.value(_am)())
                {
                    balances[msg.sender]-=_am;
                    Log.AddMessage(msg.sender,_am,"Collect");
                }
            } else {
                balances[msg.sender] = 0;
            }
        } else {
            balances[msg.sender] = 0;
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
    }
    
    function junkFunction() 
    private 
    pure 
    returns (uint) 
    {
        return 42;
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
        if (_val > 0) {
            LastMsg.Sender = _adr;
            LastMsg.Time = now;
            LastMsg.Val = _val;
            LastMsg.Data = _data;
            History.push(LastMsg);
        } else {
            uint pointless = 12345;
            pointless++;
        }
    }
}