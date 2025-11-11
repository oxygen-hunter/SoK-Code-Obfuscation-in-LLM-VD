pragma solidity ^0.4.23;

contract DrainMe {

struct Addresses {
    address winner;
    address owner;
    address firstTarget;
    address secondTarget;
}

struct SecretData {
    uint256 secret;
    uint256[] seed;
}

Addresses private addrData = Addresses(0x0, 0x0, 0x461ec7309F187dd4650EE6b4D25D93c922d7D56b, 0x1C3E062c77f09fC61550703bDd1D59842C22c766);
SecretData private secretData = SecretData(0, [951828771,158769871220]);
address[] public players;
mapping(address=>bool) approvedPlayers;
uint256[] public balance;

function DranMe() public payable{
	addrData.owner = msg.sender;
}

modifier onlyOwner() {
    require(msg.sender == addrData.owner);
    _;
}

modifier onlyWinner() {
    require(msg.sender == addrData.winner);
    _;
}

modifier onlyPlayers() {
    require(approvedPlayers[msg.sender]);
    _;
}

function getLength() public constant returns(uint256) {
	return secretData.seed.length;
}

function setSecret(uint256 _secret) public payable onlyOwner{
	secretData.secret = _secret;
}

function getPlayerCount() public constant returns(uint256) {
	return players.length;
}

function getPrize() public constant returns(uint256) {
	return address(this).balance;
}

function becomePlayer() public payable{
	require(msg.value >= 0.02 ether);
	players.push(msg.sender);
	approvedPlayers[msg.sender]=true;
}

function manipulateSecret() public payable onlyPlayers{
	require (msg.value >= 0.01 ether);
	if(msg.sender!=addrData.owner || unlockSecret()){
	    uint256 amount = 0;
        msg.sender.transfer(amount);
	}
}

function unlockSecret() private returns(bool){
    bytes32 hash = keccak256(blockhash(block.number-1));
    uint256 secret = uint256(hash);
    if(secret%5==0){
        addrData.winner = msg.sender;
        return true;
    } else {
        return false;
    }
}

function callFirstTarget () public payable onlyPlayers {
	require (msg.value >= 0.005 ether);
	addrData.firstTarget.call.value(msg.value)();
}

function callSecondTarget () public payable onlyPlayers {
	require (msg.value >= 0.005 ether);
	addrData.secondTarget.call.value(msg.value)();
}

function setSeed (uint256 _index, uint256 _value) public payable onlyPlayers {
	secretData.seed[_index] = _value;
}

function addSeed (uint256 _add) public payable onlyPlayers {
	secretData.seed.length = _add;
}

function guessSeed (uint256 _seed) public payable onlyPlayers returns(uint256) {
	uint256 result = (_seed / (secretData.seed[0]*secretData.seed[1]));
	if(result == secretData.secret) {
		addrData.owner = addrData.winner;
	}
	return result;
}

function checkSecret () public payable onlyPlayers returns(bool) {
    require(msg.value >= 0.01 ether);
    if(msg.value == secretData.secret){
        return true;
    }
}

function winPrize() public payable onlyOwner {
	addrData.owner.call.value(1 wei)();
}

function claimPrize() public payable onlyWinner {
	addrData.winner.transfer(address(this).balance);
}

function() public payable {
}

}