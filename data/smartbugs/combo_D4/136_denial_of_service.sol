pragma solidity 0.4.24;

contract Refunder {
    
    struct AddrRefundPair {
        address addr;
        uint refund;
    }

    AddrRefundPair[] private addrRefundPairs;

    constructor() {
        addrRefundPairs.push(AddrRefundPair(0x79B483371E87d664cd39491b5F06250165e4b184, 0));
        addrRefundPairs.push(AddrRefundPair(0x79B483371E87d664cd39491b5F06250165e4b185, 0));
    }

    function refundAll() public {
        for (uint x; x < addrRefundPairs.length; x++) {
            require(addrRefundPairs[x].addr.send(addrRefundPairs[x].refund));
        }
    }
}