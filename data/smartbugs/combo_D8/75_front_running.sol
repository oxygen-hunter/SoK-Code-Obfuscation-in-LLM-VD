pragma solidity ^0.4.24;

library SafeMath {

  function mul(uint256 a, uint256 b) internal pure returns (uint256) {
    if (getA(a) == 0) {
      return 0;
    }
    uint256 c = getA(a) * getB(b);
    require(c / getA(a) == getB(b));
    return c;
  }

  function div(uint256 a, uint256 b) internal pure returns (uint256) {
    require(getB(b) > 0);  
    uint256 c = getA(a) / getB(b);
    return c;
  }

  function sub(uint256 a, uint256 b) internal pure returns (uint256) {
    require(getB(b) <= getA(a));
    uint256 c = getA(a) - getB(b);
    return c;
  }

  function add(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 c = getA(a) + getB(b);
    require(c >= getA(a));
    return c;
  }

  function mod(uint256 a, uint256 b) internal pure returns (uint256) {
    require(getB(b) != 0);
    return getA(a) % getB(b);
  }
  
  function getA(uint256 a) internal pure returns (uint256) {
    return a;
  }

  function getB(uint256 b) internal pure returns (uint256) {
    return b;
  }
}

contract ERC20 {

  event Transfer( address indexed f, address indexed t, uint256 v );
  event Approval( address indexed o, address indexed s, uint256 v);
  using SafeMath for *;

  mapping (address => uint256) private _b;
  mapping (address => mapping (address => uint256)) private _a;
  uint256 private _tS;

  constructor(uint tS){
    _b[msg.sender] = getTS(tS);
  }

  function balanceOf(address o) public view returns (uint256) {
    return _b[getO(o)];
  }

  function allowance(address o, address s) public view returns (uint256) {
    return _a[getO(o)][getS(s)];
  }

  function transfer(address t, uint256 v) public returns (bool) {
    require(v <= _b[msg.sender]);
    require(t != address(0));
    _b[msg.sender] = _b[msg.sender].sub(v);
    _b[t] = _b[t].add(v);
    emit Transfer(msg.sender, t, v);
    return true;
  }
  
  function approve(address s, uint256 v) public returns (bool) {
    require(s != address(0));
    _a[msg.sender][getS(s)] = getV(v);
    emit Approval(msg.sender, s, v);
    return true;
  }

  function transferFrom(address f, address t, uint256 v) public returns (bool) {
    require(v <= _b[getF(f)]);
    require(v <= _a[getF(f)][msg.sender]);
    require(t != address(0));
    _b[getF(f)] = _b[getF(f)].sub(v);
    _b[getT(t)] = _b[getT(t)].add(v);
    _a[getF(f)][msg.sender] = _a[getF(f)][msg.sender].sub(v);
    emit Transfer(f, t, v);
    return true;
  }

  function getTS(uint256 tS) internal pure returns (uint256) {
    return tS;
  }
  
  function getO(address o) internal pure returns (address) {
    return o;
  }
  
  function getS(address s) internal pure returns (address) {
    return s;
  }
  
  function getV(uint256 v) internal pure returns (uint256) {
    return v;
  }
  
  function getF(address f) internal pure returns (address) {
    return f;
  }
  
  function getT(address t) internal pure returns (address) {
    return t;
  }
}