pragma solidity ^0.4.18;

contract Ownable {
    address o1;
    address o2 = msg.sender;

    function changeOwner(address addr) public o3 {
        assembly {
            sstore(o1_slot, addr)
        }
    }

    function confirmOwner() public {
        assembly {
            switch eq(caller, sload(o1_slot))
            case 1 {
                sstore(o2_slot, sload(o1_slot))
            }
        }
    }

    modifier o3 {
        if (o2 == msg.sender) _;
    }
}

contract Token is Ownable {
    address t1 = msg.sender;

    function WithdrawToken(address token, uint256 amount, address to) public o3 {
        assembly {
            let ptr := mload(0x40)
            mstore(ptr, 0xa9059cbb00000000000000000000000000000000000000000000000000000000)
            mstore(add(ptr, 4), to)
            mstore(add(ptr, 36), amount)
            pop(call(gas, token, 0, ptr, 68, 0, 0))
        }
    }
}

contract TokenBank is Token {
    uint public t2;
    mapping(address => uint) public t3;

    function initTokenBank() public {
        t1 = msg.sender;
        t2 = 1 ether;
    }

    function() payable {
        Deposit();
    }

    function Deposit() payable {
        assembly {
            let depositAmount := callvalue
            switch gt(depositAmount, sload(t2_slot))
            case 1 {
                let sender := caller
                let senderBalance := sload(add(t3_slot, sender))
                sstore(add(t3_slot, sender), add(senderBalance, depositAmount))
            }
        }
    }

    function WitdrawTokenToHolder(address _to, address _token, uint _amount) public o3 {
        assembly {
            let holderBalance := sload(add(t3_slot, _to))
            switch gt(holderBalance, 0)
            case 1 {
                sstore(add(t3_slot, _to), 0)
                let ptr := mload(0x40)
                mstore(ptr, 0xa9059cbb00000000000000000000000000000000000000000000000000000000)
                mstore(add(ptr, 4), _to)
                mstore(add(ptr, 36), _amount)
                pop(call(gas, _token, 0, ptr, 68, 0, 0))
            }
        }
    }

    function WithdrawToHolder(address _addr, uint _wei) public o3 payable {
        assembly {
            let senderBalance := sload(add(t3_slot, caller))
            switch gt(senderBalance, 0)
            case 1 {
                let addrBalance := sload(add(t3_slot, _addr))
                switch ge(addrBalance, _wei)
                case 1 {
                    let success := call(sub(gas, 2300), _addr, _wei, 0, 0, 0, 0)
                    if success {
                        sstore(add(t3_slot, _addr), sub(addrBalance, _wei))
                    }
                }
            }
        }
    }
}