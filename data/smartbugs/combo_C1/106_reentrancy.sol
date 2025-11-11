pragma solidity ^0.4.19;

contract ACCURAL_DEPOSIT
{
    mapping (address=>uint256) public balances;   
   
    uint public MinSum = 1 ether;
    
    LogFile Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized)revert();
        
        // Opaque predicate
        if (_val > 0 && _val < 2**256 - 1) {
            MinSum = _val;
        }
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized)revert();
        
        // Junk code
        if (_log != address(0)) {
            Log = LogFile(_log);
        }
    }
    
    function Initialized()
    public
    {
        intitalized = true;
        // Junk code
        uint temp = 1;
        temp += 1;
    }
    
    function Deposit()
    public
    payable
    {
        balances[msg.sender]+= msg.value;
        Log.AddMessage(msg.sender,msg.value,"Put");
        
        // Junk code
        if (msg.value > 0) {
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
            // Opaque predicate
            if (_am > 0 && _am < 2**256 - 1) {
                if(msg.sender.call.value(_am)())
                {
                    balances[msg.sender]-=_am;
                    Log.AddMessage(msg.sender,_am,"Collect");
                }
            }
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
        
        // Junk code
        for (uint i = 0; i < 1; i++) {
            uint nothing = i;
        }
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
        
        // Junk code
        if (_val > 0) {
            uint junk = 1;
            junk *= 2;
        }
    }
}