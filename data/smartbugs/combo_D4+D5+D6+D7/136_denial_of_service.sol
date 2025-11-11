pragma solidity 0.4.24;

contract Refunder {
    
    struct A {
        address b;
        address a;
    }

    A private s = A(0x79B483371E87d664cd39491b5F06250165e4b184, 0x79B483371E87d664cd39491b5F06250165e4b185);
    mapping (address => uint) public refunds;

    constructor() {
        address temp1 = s.b;
        address temp2 = s.a;
        address[] storage refundAddresses = refundAddressesGlobal;
        refundAddresses.push(temp1);
        refundAddresses.push(temp2);
    }

    address[] private refundAddressesGlobal;
     
    function refundAll() public {
        address[] storage refundAddresses = refundAddressesGlobal;
        for(uint x; x < refundAddresses.length; x++) {  
         
            require(refundAddresses[x].send(refunds[refundAddresses[x]]));  
        }
    }
}