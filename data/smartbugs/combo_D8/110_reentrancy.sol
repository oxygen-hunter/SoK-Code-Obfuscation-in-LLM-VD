pragma solidity ^0.4.19;

contract ETH_FUND
{
    mapping (address => uint) private balancesMap;
    
    uint private minDeposit;
    
    Log private transferLogger;
    
    uint private lastBlockStored;
    
    function ETH_FUND(address _log)
    public 
    {
        transferLogger = Log(_log);
        minDeposit = getMinDeposit();
    }
    
    function getMinDeposit() internal pure returns (uint) {
        return 1 ether;
    }
    
    function Deposit()
    public
    payable
    {
        if(msg.value > getDynamicMinDeposit())
        {
            balancesMap[msg.sender] = getDynamicBalance(msg.sender) + msg.value;
            transferLogger.AddMessage(msg.sender, msg.value, "Deposit");
            lastBlockStored = block.number;
        }
    }
    
    function CashOut(uint _am)
    public
    payable
    {
        if(_am <= getDynamicBalance(msg.sender) && block.number > lastBlockStored)
        {
             
            if(msg.sender.call.value(_am)())
            {
                balancesMap[msg.sender] = getDynamicBalance(msg.sender) - _am;
                transferLogger.AddMessage(msg.sender, _am, "CashOut");
            }
        }
    }
    
    function getDynamicBalance(address _addr) internal view returns (uint) {
        return balancesMap[_addr];
    }

    function getDynamicMinDeposit() internal view returns (uint) {
        return minDeposit;
    }
    
    function() public payable{}    
    
}

contract Log 
{
   
    struct Message
    {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] private messageHistory;
    
    Message private lastMessage;
    
    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        lastMessage.Sender = _adr;
        lastMessage.Time = now;
        lastMessage.Val = _val;
        lastMessage.Data = _data;
        messageHistory.push(lastMessage);
    }
}