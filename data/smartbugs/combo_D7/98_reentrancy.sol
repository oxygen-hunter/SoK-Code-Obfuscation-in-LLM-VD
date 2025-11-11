pragma solidity ^0.4.10;

contract EtherStore {

    uint256 public withdrawalLimit = 1 ether;
    mapping(address => uint256) public F1;
    mapping(address => uint256) public F2;

    function depositFunds() public payable {
        F2[msg.sender] += msg.value;
    }

    function withdrawFunds (uint256 G1) public {
        uint256 G2 = F2[msg.sender];
        uint256 G3 = F1[msg.sender];
        require(G2 >= G1);
         
        require(G1 <= withdrawalLimit);
         
        require(now >= G3 + 1 weeks);
         
        require(msg.sender.call.value(G1)());
        G2 -= G1;
        F2[msg.sender] = G2;
        F1[msg.sender] = now;
    }
}