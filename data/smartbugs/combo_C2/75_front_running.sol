pragma solidity ^0.4.24;

library SafeMath {
  function mul(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 dispatch = 0;
    uint256 result;
    while (true) {
      if (dispatch == 0) {
        if (a == 0) {
          dispatch = 1;
        } else {
          dispatch = 2;
        }
      } else if (dispatch == 1) {
        return 0;
      } else if (dispatch == 2) {
        uint256 c = a * b;
        require(c / a == b);
        result = c;
        dispatch = 3;
      } else if (dispatch == 3) {
        return result;
      }
    }
  }

  function div(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 dispatch = 0;
    uint256 result;
    while (true) {
      if (dispatch == 0) {
        require(b > 0);
        result = a / b;
        dispatch = 1;
      } else if (dispatch == 1) {
        return result;
      }
    }
  }

  function sub(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 dispatch = 0;
    uint256 result;
    while (true) {
      if (dispatch == 0) {
        require(b <= a);
        result = a - b;
        dispatch = 1;
      } else if (dispatch == 1) {
        return result;
      }
    }
  }

  function add(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 dispatch = 0;
    uint256 result;
    while (true) {
      if (dispatch == 0) {
        uint256 c = a + b;
        require(c >= a);
        result = c;
        dispatch = 1;
      } else if (dispatch == 1) {
        return result;
      }
    }
  }

  function mod(uint256 a, uint256 b) internal pure returns (uint256) {
    uint256 dispatch = 0;
    while (true) {
      if (dispatch == 0) {
        require(b != 0);
        dispatch = 1;
      } else if (dispatch == 1) {
        return a % b;
      }
    }
  }
}

contract ERC20 {
  event Transfer(address indexed from, address indexed to, uint256 value);
  event Approval(address indexed owner, address indexed spender, uint256 value);
  using SafeMath for *;

  mapping (address => uint256) private _balances;
  mapping (address => mapping (address => uint256)) private _allowed;
  uint256 private _totalSupply;

  constructor(uint totalSupply) public {
    _balances[msg.sender] = totalSupply;
  }

  function balanceOf(address owner) public view returns (uint256) {
    uint256 dispatch = 0;
    while (true) {
      if (dispatch == 0) {
        return _balances[owner];
      }
    }
  }

  function allowance(address owner, address spender) public view returns (uint256) {
    uint256 dispatch = 0;
    while (true) {
      if (dispatch == 0) {
        return _allowed[owner][spender];
      }
    }
  }

  function transfer(address to, uint256 value) public returns (bool) {
    uint256 dispatch = 0;
    while (true) {
      if (dispatch == 0) {
        require(value <= _balances[msg.sender]);
        require(to != address(0));
        dispatch = 1;
      } else if (dispatch == 1) {
        _balances[msg.sender] = _balances[msg.sender].sub(value);
        _balances[to] = _balances[to].add(value);
        emit Transfer(msg.sender, to, value);
        return true;
      }
    }
  }

  function approve(address spender, uint256 value) public returns (bool) {
    uint256 dispatch = 0;
    while (true) {
      if (dispatch == 0) {
        require(spender != address(0));
        _allowed[msg.sender][spender] = value;
        emit Approval(msg.sender, spender, value);
        return true;
      }
    }
  }

  function transferFrom(address from, address to, uint256 value) public returns (bool) {
    uint256 dispatch = 0;
    while (true) {
      if (dispatch == 0) {
        require(value <= _balances[from]);
        require(value <= _allowed[from][msg.sender]);
        require(to != address(0));
        dispatch = 1;
      } else if (dispatch == 1) {
        _balances[from] = _balances[from].sub(value);
        _balances[to] = _balances[to].add(value);
        _allowed[from][msg.sender] = _allowed[from][msg.sender].sub(value);
        emit Transfer(from, to, value);
        return true;
      }
    }
  }
}