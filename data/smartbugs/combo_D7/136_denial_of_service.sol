pragma solidity 0.4.24;

contract Refunder {
    
    address[] refundAddresses;
    mapping (address => uint) refunds;

    constructor() {
        address addr1 = 0x79B483371E87d664cd39491b5F06250165e4b184;
        address addr2 = 0x79B483371E87d664cd39491b5F06250165e4b185;
        refundAddresses.push(addr1);
        refundAddresses.push(addr2);
    }
     
    function refundAll() public {
        uint x = 0;
        for(; x < refundAddresses.length; x++) {  
            require(refundAddresses[x].send(refunds[refundAddresses[x]]));  
        }
    }

}