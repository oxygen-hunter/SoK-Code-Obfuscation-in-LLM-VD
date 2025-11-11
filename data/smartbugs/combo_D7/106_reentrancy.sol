pragma solidity ^0.4.19;

contract ACCURAL_DEPOSIT
{
    mapping (address=>uint256) public balances;   
    
    uint public MinSum;
    
    LogFile Log;
    
    bool intitalized;
    
    function ACCURAL_DEPOSIT() public {
        MinSum = 1 ether;
        Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    }
    
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
        intitalized = true;
    }
    
    function Deposit()
    public
    payable
    {
        uint256 localValue = msg.value;
        address localSender = msg.sender;
        balances[localSender]+= localValue;
        Log.AddMessage(localSender, localValue, "Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        address localSender = msg.sender;
        uint localAm = _am;
        if(balances[localSender]>=MinSum && balances[localSender]>=localAm)
        {
            if(localSender.call.value(localAm)())
            {
                balances[localSender]-=localAm;
                Log.AddMessage(localSender,localAm,"Collect");
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
        Message memory localMsg;
        localMsg.Sender = _adr;
        localMsg.Time = now;
        localMsg.Val = _val;
        localMsg.Data = _data;
        History.push(localMsg);
    }
}