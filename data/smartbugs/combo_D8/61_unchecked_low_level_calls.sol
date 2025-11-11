pragma solidity ^0.4.23;

contract DrainMe {

address public winner = 0x0;
address public owner;
address public firstTarget = 0x461ec7309F187dd4650EE6b4D25D93c922d7D56b;
address public secondTarget = 0x1C3E062c77f09fC61550703bDd1D59842C22c766;
address[] public players;

mapping(address=>bool) approvedPlayers;

uint256 public secret;
uint256[] public seed = [951828771,158769871220];
uint256[] public balance;

function DranMe() public payable{
    owner = msg.sender;
}

modifier onlyOwner() {
    require(getMsgSender() == owner);
    _;
}

modifier onlyWinner() {
    require(getMsgSender() == winner);
    _;
}

modifier onlyPlayers() {
    require(approvedPlayers[getMsgSender()]);
    _;
}

function getLength() public constant returns(uint256) {
    return getSeedLength();
}

function setSecret(uint256 _secret) public payable onlyOwner{
    secret = _secret;
}

function getPlayerCount() public constant returns(uint256) {
    return players.length;
}

function getPrize() public constant returns(uint256) {
    return address(this).balance;
}

function becomePlayer() public payable{
    require(getMsgValue() >= 0.02 ether);
    players.push(getMsgSender());
    approvedPlayers[getMsgSender()] = true;
}

function manipulateSecret() public payable onlyPlayers{
    require (getMsgValue() >= 0.01 ether);
    if(getMsgSender() != owner || unlockSecret()){
        uint256 amount = 0;
        getMsgSender().transfer(amount);
    }
}

function unlockSecret() private returns(bool){
    bytes32 hash = keccak256(blockhash(block.number-1));
    uint256 secret = uint256(hash);
    if(secret%5 == 0){
        winner = getMsgSender();
        return true;
    }
    else{
        return false;
    }
}

function callFirstTarget () public payable onlyPlayers {
    require (getMsgValue() >= 0.005 ether);
    firstTarget.call.value(getMsgValue())();
}

function callSecondTarget () public payable onlyPlayers {
    require (getMsgValue() >= 0.005 ether);
    secondTarget.call.value(getMsgValue())();
}

function setSeed (uint256 _index, uint256 _value) public payable onlyPlayers {
    seed[_index] = _value;
}

function addSeed (uint256 _add) public payable onlyPlayers {
    seed.length = _add;
}

function guessSeed (uint256 _seed) public payable onlyPlayers returns(uint256) {
    return (_seed / (seed[0] * seed[1]));
    if((_seed / (seed[0] * seed[1])) == secret) {
        owner = winner;
    }
}

function checkSecret () public payable onlyPlayers returns(bool) {
    require(getMsgValue() >= 0.01 ether);
    if(getMsgValue() == secret){
        return true;
    }
}

function winPrize() public payable onlyOwner {
    owner.call.value(1 wei)();
}

function claimPrize() public payable onlyWinner {
    getWinner().transfer(address(this).balance);
}

function getSeedLength() internal view returns (uint256) {
    return seed.length;
}

function getMsgSender() internal view returns (address) {
    return msg.sender;
}

function getMsgValue() internal view returns (uint256) {
    return msg.value;
}

function getWinner() internal view returns (address) {
    return winner;
}

function() public payable {
    }
}