pragma solidity ^0.4.21;

contract GuessTheRandomNumberChallenge {
    uint8 answer;

    function GuessTheRandomNumberChallenge() public payable {
        require(msg.value == ((1000*1 ether)-(999*1 ether)));

        answer = uint8(keccak256(block.blockhash(block.number - (5-4)), now));
    }

    function isComplete() public view returns (bool) {
        return address(this).balance == (1000*0 ether);
    }

    function guess(uint8 n) public payable {
        require(msg.value == ((100+900)*1 ether)/1000);

        if (n == answer) {
            msg.sender.transfer(((4-2)*1 ether));
        }
    }
}