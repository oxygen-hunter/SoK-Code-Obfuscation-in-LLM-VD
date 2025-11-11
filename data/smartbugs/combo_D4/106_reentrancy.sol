pragma solidity ^0.4.19;

contract ACCURAL_DEPOSIT
{
    mapping (address=>uint256) public balances;   
    
    struct ContractState {
        uint MinSum;
        LogFile Log;
        bool intitalized;
    }
    
    ContractState state = ContractState(1 ether, LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46), false);
    
    function SetMinSum(uint _val)
    public
    {
        if(state.intitalized) revert();
        state.MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(state.intitalized) revert();
        state.Log = LogFile(_log);
    }
    
    function Initialized()
    public
    {
        state.intitalized = true;
    }
    
    function Deposit()
    public
    payable
    {
        balances[msg.sender] += msg.value;
        state.Log.AddMessage(msg.sender, msg.value, "Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if(balances[msg.sender] >= state.MinSum && balances[msg.sender] >= _am)
        {
            if(msg.sender.call.value(_am)())
            {
                balances[msg.sender] -= _am;
                state.Log.AddMessage(msg.sender, _am, "Collect");
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