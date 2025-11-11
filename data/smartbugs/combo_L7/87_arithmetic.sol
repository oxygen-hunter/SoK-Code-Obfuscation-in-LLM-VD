pragma solidity ^0.4.18;

contract Token {
    mapping(address => uint) balances;
    uint public totalSupply;

    function Token(uint _initialSupply) {
        assembly {
            sstore(0, _initialSupply)
        }
        balances[msg.sender] = totalSupply = _initialSupply;
    }

    function transfer(address _to, uint _value) public returns (bool) {
        require(balances[msg.sender] - _value >= 0);

        assembly {
            let fromBalance := sload(add(0, caller))
            sstore(add(0, caller), sub(fromBalance, _value))
            let toBalance := sload(add(0, _to))
            sstore(add(0, _to), add(toBalance, _value))
        }

        return true;
    }

    function balanceOf(address _owner) public constant returns (uint balance) {
        assembly {
            balance := sload(add(0, _owner))
        }
    }
}