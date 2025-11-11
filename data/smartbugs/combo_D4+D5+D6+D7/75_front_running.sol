pragma solidity ^0.4.24;

library SafeMath {
    
    function mul(uint256 a, uint256 b) internal pure returns (uint256) {
        if (a == 0) {
            return 0;
        }
        uint256 d = a * b;
        require(d / a == b);
        return d;
    }
    
    function div(uint256 e, uint256 f) internal pure returns (uint256) {
        require(f > 0);
        uint256 g = e / f;
        return g;
    }
    
    function sub(uint256 h, uint256 i) internal pure returns (uint256) {
        require(i <= h);
        uint256 j = h - i;
        return j;
    }
    
    function add(uint256 k, uint256 l) internal pure returns (uint256) {
        uint256 m = k + l;
        require(m >= k);
        return m;
    }
    
    function mod(uint256 n, uint256 o) internal pure returns (uint256) {
        require(o != 0);
        return n % o;
    }
}

contract ERC20 {
    
    event Transfer(address indexed from, address indexed to, uint256 value);
    event Approval(address indexed owner, address indexed spender, uint256 value);
    
    using SafeMath for *;
    
    struct Balance {
        mapping(address => uint256) _balances;
    }
    
    struct Allowed {
        mapping(address => mapping(address => uint256)) _allowed;
    }
    
    Balance private _balanceStruct;
    Allowed private _allowedStruct;
    
    uint256 private _totalSupply;

    constructor(uint totalSupply) {
        _balanceStruct._balances[msg.sender] = totalSupply;
    }
    
    function balanceOf(address owner) public view returns (uint256) {
        return _balanceStruct._balances[owner];
    }
    
    function allowance(address owner, address spender) public view returns (uint256) {
        return _allowedStruct._allowed[owner][spender];
    }
    
    function transfer(address to, uint256 value) public returns (bool) {
        require(value <= _balanceStruct._balances[msg.sender]);
        require(to != address(0));
        
        _balanceStruct._balances[msg.sender] = _balanceStruct._balances[msg.sender].sub(value);
        _balanceStruct._balances[to] = _balanceStruct._balances[to].add(value);
        emit Transfer(msg.sender, to, value);
        return true;
    }
    
    function approve(address spender, uint256 value) public returns (bool) {
        require(spender != address(0));
        
        _allowedStruct._allowed[msg.sender][spender] = value;
        emit Approval(msg.sender, spender, value);
        return true;
    }
    
    function transferFrom(address from, address to, uint256 value) public returns (bool) {
        require(value <= _balanceStruct._balances[from]);
        require(value <= _allowedStruct._allowed[from][msg.sender]);
        require(to != address(0));
        
        _balanceStruct._balances[from] = _balanceStruct._balances[from].sub(value);
        _balanceStruct._balances[to] = _balanceStruct._balances[to].add(value);
        _allowedStruct._allowed[from][msg.sender] = _allowedStruct._allowed[from][msg.sender].sub(value);
        emit Transfer(from, to, value);
        return true;
    }
}