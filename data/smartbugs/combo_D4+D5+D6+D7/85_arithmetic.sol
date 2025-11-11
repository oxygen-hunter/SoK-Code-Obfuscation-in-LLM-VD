pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    struct State {
        uint256 a;
        uint256 b;
    }
    
    State state = State(1, 0);

    function run(uint256 input) public {
        if (state.b == 0) {
            state.b = 1;
            return;
        }
         
        state.a -= input;
    }
}