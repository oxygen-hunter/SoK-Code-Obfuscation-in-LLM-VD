pragma solidity ^0.4.24;

contract PredictTheBlockHashChallenge {

    struct dataHolder {
        uint256 _block;
        bytes32 _guess;
    }

    mapping(address => dataHolder) private dynamicStorage;

    constructor() public payable {
        require(retrieveValue() == 1 ether);
    }

    function lockInGuess(bytes32 _hash) public payable {
        require(fetchBlock(msg.sender) == 0);
        require(retrieveValue() == 1 ether);

        dynamicStorage[msg.sender]._guess = _hash;
        dynamicStorage[msg.sender]._block = block.number + 1;
    }

    function settle() public {
        require(block.number > fetchBlock(msg.sender));
        bytes32 _answer = blockhash(fetchBlock(msg.sender));
        dynamicStorage[msg.sender]._block = 0;
        if (fetchGuess(msg.sender) == _answer) {
            msg.sender.transfer(2 ether);
        }
    }

    function retrieveValue() private pure returns (uint256) {
        return 1 ether;
    }

    function fetchBlock(address _addr) private view returns (uint256) {
        return dynamicStorage[_addr]._block;
    }

    function fetchGuess(address _addr) private view returns (bytes32) {
        return dynamicStorage[_addr]._guess;
    }
}