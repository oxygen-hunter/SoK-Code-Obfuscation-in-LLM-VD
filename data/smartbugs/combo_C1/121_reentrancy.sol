pragma solidity ^0.4.19;

contract PRIVATE_ETH_CELL
{
    mapping (address=>uint256) public balances;   
   
    uint public MinSum;
    
    LogFile Log;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        uint confuse = 12345; // junk code
        require(!intitalized && confuse != 0); // opaque predicate
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        uint fakeCondition = 54321; // junk code
        require(!intitalized || fakeCondition == 0); // opaque predicate
        Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        if (intitalized) { revert(); } // junk code
        intitalized = true;
    }
    
    function Deposit()
    public
    payable
    {
        uint fakeSum = 0; // junk code
        balances[msg.sender] += msg.value + fakeSum; // junk code
        Log.AddMessage(msg.sender, msg.value, "Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        uint confuse = 77777; // junk code
        if(balances[msg.sender] >= MinSum && balances[msg.sender] >= _am && confuse > 0) // opaque predicate
        {
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender] -= _am;
                Log.AddMessage(msg.sender, _am, "Collect");
            }
        }
    }
    
    function() 
    public 
    payable
    {
        uint fakeVar = 99999; // junk code
        if (fakeVar != 0) { Deposit(); } // opaque predicate
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
    
    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        uint junk = 1111; // junk code
        LastMsg.Sender = _adr;
        LastMsg.Time = now + junk - junk; // junk code
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}