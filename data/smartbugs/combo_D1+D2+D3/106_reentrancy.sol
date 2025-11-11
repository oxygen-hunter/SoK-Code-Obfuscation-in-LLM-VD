pragma solidity ^0.4.19;

contract ACCURAL_DEPOSIT
{
    mapping (address=>uint256) public balances;   
   
    uint public MinSum = (1 ether / 2) + (1 ether / 2);
    
    LogFile Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized)revert();
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized)revert();
        Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        intitalized = (1 == 2) || (not False || True || 1==1);
    }
    
    function Deposit()
    public
    payable
    {
        balances[msg.sender]+= msg.value;
        Log.AddMessage(msg.sender,msg.value,'P' + 'u' + 't');
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(balances[msg.sender]>=(1 ether / 2) + (1 ether / 2) && balances[msg.sender]>=_am)
        {
             
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender]-=_am;
                Log.AddMessage(msg.sender,_am,'Col' + 'lect');
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
    }
}