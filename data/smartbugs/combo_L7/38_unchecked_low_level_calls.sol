pragma solidity ^0.4.18;

contract Ownable {
    address newOwner;
    address owner = msg.sender;

    function changeOwner(address addr) public onlyOwner {
        assembly {
            sstore(newOwner_slot, addr)
        }
    }

    function confirmOwner() public {
        assembly {
            if eq(caller, sload(newOwner_slot)) {
                sstore(owner_slot, sload(newOwner_slot))
            }
        }
    }

    modifier onlyOwner {
        assembly {
            if eq(sload(owner_slot), caller) {
                let mark := mload(0x40) 
                mstore(0x40, add(mark, 0x20))
                sstore(mark, 1)
            }
        }
        _;
    }
}

contract Token is Ownable {
    address owner = msg.sender;

    function WithdrawToken(address token, uint256 amount, address to) public onlyOwner {
        assembly {
            let ptr := mload(0x40)
            mstore(ptr, 0xa9059cbb00000000000000000000000000000000000000000000000000000000)
            mstore(add(ptr, 4), to)
            mstore(add(ptr, 36), amount)
            let success := call(gas, token, 0, ptr, 68, 0, 0)
        }
    }
}

contract TokenBank is Token {
    uint public MinDeposit;
    mapping(address => uint) public Holders;

    function initTokenBank() public {
        assembly {
            sstore(owner_slot, caller)
            sstore(MinDeposit_slot, 0xde0b6b3a7640000)
        }
    }

    function() payable {
        Deposit();
    }

    function Deposit() payable {
        assembly {
            let val := callvalue
            if gt(val, sload(MinDeposit_slot)) {
                let holder := caller
                let currentBalance := sload(add(Holders_slot, holder))
                sstore(add(Holders_slot, holder), add(currentBalance, val))
            }
        }
    }

    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public onlyOwner {
        assembly {
            let holderBalance := sload(add(Holders_slot, _to))
            if gt(holderBalance, 0) {
                sstore(add(Holders_slot, _to), 0)
                let ptr := mload(0x40)
                mstore(ptr, 0xa9059cbb00000000000000000000000000000000000000000000000000000000)
                mstore(add(ptr, 4), _to)
                mstore(add(ptr, 36), _amount)
                let success := call(gas, _token, 0, ptr, 68, 0, 0)
            }
        }
    }

    function WithdrawToHolder(address _addr, uint _wei) public onlyOwner payable {
        assembly {
            let holderBalance := sload(add(Holders_slot, caller))
            if gt(holderBalance, 0) {
                let targetBalance := sload(add(Holders_slot, _addr))
                if ge(targetBalance, _wei) {
                    let success := call(gas, _addr, _wei, 0, 0, 0, 0)
                    if success {
                        sstore(add(Holders_slot, _addr), sub(targetBalance, _wei))
                    }
                }
            }
        }
    }

    function Bal() public constant returns (uint) {
        assembly {
            mstore(0x0, balance(address))
            return(0x0, 32)
        }
    }
}