pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) balances;

    event Transfer(address indexed _from, address indexed _to, uint256 _value);

    function MyToken() {
        balances[tx.origin] = 10000;
    }
     
    function sendCoin(address to, uint amount) returns(bool sufficient) {
        bool result;
        assembly {
            switch lt(balances[caller], amount)
            case 1 { result := 0 }
            default {
                sstore(balances_slot, sub(sload(balances_slot), amount))
                sstore(add(balances_slot, to), add(sload(add(balances_slot, to)), amount))
                log3(0, 0, caller, to, amount)
                result := 1
            }
        }
        return result;
    }

    function getBalance(address addr) constant returns(uint) {
        return balances[addr];
    }
}