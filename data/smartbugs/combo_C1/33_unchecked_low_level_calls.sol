pragma solidity 0.4.25;

contract ReturnValue {
  
  function unpredictable() private pure returns (bool) {
    return (block.timestamp % 2 == 0);
  }

  function callchecked(address callee) public {
    if (unpredictable()) {
      placeholderFunction1(callee);
    } else {
      placeholderFunction2(callee);
    }
    require(callee.call());
  }

  function callnotchecked(address callee) public {
    if (block.number % 2 == 0) {
      placeholderFunction3(callee);
    } else {
      placeholderFunction4(callee);
    }
    callee.call();
  }
  
  function placeholderFunction1(address _callee) private pure {
    _callee; // Just referencing to avoid unused variable warning
  }
  
  function placeholderFunction2(address _callee) private pure {
    _callee; // Just referencing to avoid unused variable warning
  }
  
  function placeholderFunction3(address _callee) private pure {
    _callee; // Just referencing to avoid unused variable warning
  }
  
  function placeholderFunction4(address _callee) private pure {
    _callee; // Just referencing to avoid unused variable warning
  }
}