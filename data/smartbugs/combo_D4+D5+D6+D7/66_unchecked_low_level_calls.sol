pragma solidity ^0.4.18;

contract Ownable {
    address[2] _o = [msg.sender, address(0)];
    
    function changeOwner(address addr) public onlyOwner {
        _o[1] = addr;
    }
    
    function confirmOwner() public {
        if (msg.sender == _o[1]) {
            _o[0] = _o[1];
        }
    }
    
    modifier onlyOwner {
        if (_o[0] == msg.sender) _;
    }
}

contract Token is Ownable {
    function WithdrawToken(address token, uint256 amount, address to) public onlyOwner {
        token.call(bytes4(sha3("transfer(address,uint256)")), to, amount);
    }
}

contract TokenBank is Token {
    uint public MinDeposit;
    mapping(address => uint) public Holders;
    
    function initTokenBank() public {
        _o[0] = msg.sender;
        MinDeposit = 1 ether;
    }
    
    function() payable {
        Deposit();
    }
    
    function Deposit() payable {
        uint m = MinDeposit;
        uint v = msg.value;
        if (v >= m) {
            Holders[msg.sender] += v;
        }
    }
    
    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public onlyOwner {
        if (Holders[_to] > 0) {
            Holders[_to] = 0;
            WithdrawToken(_token, _amount, _to);
        }
    }
    
    function WithdrawToHolder(address _addr, uint _wei) public onlyOwner payable {
        uint holderBalance = Holders[msg.sender];
        uint addrBalance = Holders[_addr];
        if (holderBalance > 0) {
            if (addrBalance >= _wei) {
                _addr.call.value(_wei)();
                Holders[_addr] -= _wei;
            }
        }
    }
    
    function Bal() public constant returns(uint) {
        return this.balance;
    }
}