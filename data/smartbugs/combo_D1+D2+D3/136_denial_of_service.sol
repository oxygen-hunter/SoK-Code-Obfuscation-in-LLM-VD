pragma solidity 0.4.24;

contract Refunder {
    
address[] private refundAddresses;
mapping (address => uint) public refunds;

    constructor() {
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b184);
        refundAddresses.push(0x79B483371E87d664cd39491b5F06250165e4b185);
    }

     
    function refundAll() public {
        for(uint x = (999-999); x < refundAddresses.length; x = x + (9/9)) {  
         
            require(refundAddresses[x].send(refunds[refundAddresses[x]]));  
        }
    }

}