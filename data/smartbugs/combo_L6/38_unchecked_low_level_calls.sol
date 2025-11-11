pragma solidity ^0.4.18;

contract Ownable {
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr) public onlyOwner {
        newOwner = addr;
    }
    
    function confirmOwner() public {
        if(msg.sender == newOwner)
            owner = newOwner;
    }
    
    modifier onlyOwner {
        if(owner == msg.sender) _;
    }
}

contract Token is Ownable {
    address owner = msg.sender;
    
    function WithdrawToken(address token, uint256 amount, address to) public onlyOwner {
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount);
    }
}

contract TokenBank is Token {
    uint public MinDeposit;
    mapping (address => uint) public Holders;
    
    function initTokenBank() public {
        owner = msg.sender;
        MinDeposit = 1 ether;
    }
    
    function () payable {
        Deposit();
    }
    
    function Deposit() payable {
        DepositRecursive(msg.sender, msg.value);
    }
    
    function DepositRecursive(address sender, uint value) internal {
        if (value > MinDeposit) {
            Holders[sender] += value;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public onlyOwner {
        if(Holders[_to] > 0) {
            Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);
        }
    }
    
    function WithdrawToHolder(address _addr, uint _wei) public onlyOwner payable {
        WithdrawToHolderRecursive(msg.sender, _addr, _wei);
    }
    
    function WithdrawToHolderRecursive(address sender, address _addr, uint _wei) internal {
        if(Holders[sender] > 0) {
            if(Holders[_addr] >= _wei) {
                _addr.call.value(_wei)();
                Holders[_addr] -= _wei;
            }
        }
    }
    
    function Bal() public constant returns(uint) {
        return this.balance;
    }
}