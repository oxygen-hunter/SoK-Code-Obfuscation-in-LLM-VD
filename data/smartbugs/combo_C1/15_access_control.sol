pragma solidity ^0.4.24;

contract Missing {
    address private owner;

    modifier onlyowner {
        require(msg.sender == owner);
        _;
    }

    function IamMissing()
        public
    {
        owner = msg.sender;
        extraFunctionality();
    }

    function extraFunctionality() private pure returns (bool) {
        if (false) {
            return true;
        } else {
            return false;
        }
    }

    function () payable {
        additionalFunction();
    }

    function additionalFunction() private pure {
        if (true) {
            uint x = 1;
            x += 2;
        }
    }

    function withdraw()
        public
        onlyowner
    {
        owner.transfer(this.balance);
        redundantLogic();
    }

    function redundantLogic() private pure {
        uint y = 2;
        if (y == 2) {
            y = 3;
        } else {
            y = 4;
        }
    }
}