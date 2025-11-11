pragma solidity ^0.4.19;

contract ETH_FUND {
    mapping (address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;
    uint lastBlock;

    function ETH_FUND(address _log) public {
        TransferLog = Log(_log);
    }
    
    function Deposit() public payable {
        assembly {
            let msgValue := callvalue()
            let min := sload(MinDeposit_slot)
            if gt(msgValue, min) {
                let sender := caller()
                let balanceSlot := add(keccak256(sender, 0x0), balances_slot)
                sstore(balanceSlot, add(sload(balanceSlot), msgValue))
                let log := sload(TransferLog_slot)
                mstore(0x80, sender)
                mstore(0xa0, msgValue)
                mstore(0xc0, "Deposit")
                let success := call(gas, log, 0, 0x80, 0x60, 0, 0)
                if iszero(success) { revert(0, 0) }
                sstore(lastBlock_slot, number())
            }
        }
    }
    
    function CashOut(uint _am) public payable {
        assembly {
            let sender := caller()
            let balanceSlot := add(keccak256(sender, 0x0), balances_slot)
            let balance := sload(balanceSlot)
            let currentBlock := number()
            let lastBlk := sload(lastBlock_slot)
            if and(le(_am, balance), gt(currentBlock, lastBlk)) {
                let success := call(gas, sender, _am, 0, 0, 0, 0)
                if success {
                    sstore(balanceSlot, sub(balance, _am))
                    let log := sload(TransferLog_slot)
                    mstore(0x80, sender)
                    mstore(0xa0, _am)
                    mstore(0xc0, "CashOut")
                    let logSuccess := call(gas, log, 0, 0x80, 0x60, 0, 0)
                    if iszero(logSuccess) { revert(0, 0) }
                }
            }
        }
    }
    
    function() public payable {}
}

contract Log {
    struct Message {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }
    
    Message[] public History;
    Message LastMsg;
    
    function AddMessage(address _adr, uint _val, string _data) public {
        assembly {
            sstore(LastMsg_slot, _adr)
            sstore(add(LastMsg_slot, 1), now)
            sstore(add(LastMsg_slot, 2), _val)
            sstore(add(LastMsg_slot, 3), _data)
            let ptr := mload(0x40)
            mstore(ptr, _adr)
            mstore(add(ptr, 0x20), _val)
            mstore(add(ptr, 0x40), _data)
            mstore(add(ptr, 0x60), now)
            let len := sload(History_slot)
            sstore(add(keccak256(ptr, 0x80), History_slot), ptr)
            sstore(History_slot, add(len, 1))
        }
    }
}