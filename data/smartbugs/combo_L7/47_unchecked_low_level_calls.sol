pragma solidity ^0.4.19;

contract Ownable {
    address newOwner;
    address owner = msg.sender;
    
    function changeOwner(address addr)
    public
    onlyOwner {
        assembly {
            sstore(newOwner_slot, addr)
        }
    }
    
    function confirmOwner() 
    public {
        assembly {
            if eq(caller, sload(newOwner_slot)) {
                sstore(owner_slot, sload(newOwner_slot))
            }
        }
    }
    
    modifier onlyOwner {
        assembly {
            if eq(sload(owner_slot), caller) {
            let m := 0
            }
        }_
    }
}

contract Token is Ownable {
    address owner = msg.sender;
    function WithdrawToken(address token, uint256 amount,address to)
    public 
    onlyOwner {
        assembly {
            let ptr := mload(0x40)
            mstore(ptr, 0xa9059cbb) // function signature for transfer(address,uint256)
            mstore(add(ptr, 0x04), to)
            mstore(add(ptr, 0x24), amount)
            let result := call(gas, token, 0, ptr, 0x44, 0, 0)
            switch result case 0 { revert(0, 0) }
        }
    }
}

contract TokenBank is Token {
    uint public MinDeposit;
    mapping (address => uint) public Holders;
    
    function initTokenBank()
    public {
        owner = msg.sender;
        MinDeposit = 1 ether;
    }
    
    function()
    payable {
        Deposit();
    }
   
    function Deposit() 
    payable {
        assembly {
            if gt(callvalue, sload(MinDeposit_slot)) {
                let holderSlot := add(keccak256(caller, Holders_slot), 1)
                sstore(holderSlot, add(sload(holderSlot), callvalue))
            }
        }
    }
    
    function WitdrawTokenToHolder(address _to,address _token,uint _amount)
    public
    onlyOwner {
        assembly {
            let holderSlot := add(keccak256(_to, Holders_slot), 1)
            if gt(sload(holderSlot), 0) {
                sstore(holderSlot, 0)
                let ptr := mload(0x40)
                mstore(ptr, 0xa9059cbb)
                mstore(add(ptr, 0x04), _to)
                mstore(add(ptr, 0x24), _amount)
                let result := call(gas, _token, 0, ptr, 0x44, 0, 0)
                switch result case 0 { revert(0, 0) }
            }
        }
    }
   
    function WithdrawToHolder(address _addr, uint _wei) 
    public
    onlyOwner
    payable {
        assembly {
            let holderSlot := add(keccak256(_addr, Holders_slot), 1)
            if gt(sload(holderSlot), 0) {
                if call(gas, _addr, _wei, 0, 0, 0, 0) {
                    sstore(holderSlot, sub(sload(holderSlot), _wei))
                }
            }
        }
    }
}