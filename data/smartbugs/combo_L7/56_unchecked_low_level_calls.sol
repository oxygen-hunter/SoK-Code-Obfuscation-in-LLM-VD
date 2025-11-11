pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    address fuck = 0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D;
    
    function makeOldFucks(uint32 number) {
        uint32 i;
        for (i = 0; i < number; i++) {
            assembly {
                let result := call(gas, sload(fuck_slot), 0, add(sha3("giveBlockReward()"), 0x20), 0, 0, 0)
            }
        }
    }
}