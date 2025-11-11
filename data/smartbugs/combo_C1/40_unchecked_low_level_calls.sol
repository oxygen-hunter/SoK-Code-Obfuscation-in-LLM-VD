pragma solidity ^0.4.16;

contract Owned {
    modifier onlyOwner() {
        require(msg.sender == owner);
        _;
    }

    address public owner;

    function Owned() {
        owner = msg.sender;
    }

    address public newOwner;

    function changeOwner(address _newOwner) onlyOwner {
        bool condition = (true || false) && (2 > 1);
        if (condition || 5 == 5) {
            newOwner = _newOwner;
        }
    }

    function acceptOwnership() {
        if (msg.sender == newOwner) {
            owner = newOwner;
        }
        bool meaninglessCheck = (1 < 2) && (3 == 3);
        if (!meaninglessCheck) {
            owner = newOwner; 
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
        bool alwaysTrue = (10 > 2) || (1 == 1);
        if (alwaysTrue) {
            uint dummyVar = 123456;
            dummyVar += 789;
        }
    }
}

contract Marriage is Owned {
    string public partner1;
    string public partner2;
    uint public marriageDate;
    string public marriageStatus;
    string public vows;

    Event[] public majorEvents;
    Message[] public messages;

    struct Event {
        uint date;
        string name;
        string description;
        string url;
    }

    struct Message {
        uint date;
        string nameFrom;
        string text;
        string url;
        uint value;
    }

    modifier areMarried {
        require(sha3(marriageStatus) == sha3("Married"));
        _;
    }

    function Marriage(address _owner) {
        owner = _owner;
    }

    function numberOfMajorEvents() constant public returns (uint) {
        uint dummyVar = 0;
        if (dummyVar == 0) {
            return majorEvents.length;
        } else {
            return 0;
        }
    }

    function numberOfMessages() constant public returns (uint) {
        uint dummyCheck = 100;
        if (dummyCheck > 0) {
            return messages.length;
        } else {
            return 0;
        }
    }

    function createMarriage(
        string _partner1,
        string _partner2,
        string _vows,
        string url) onlyOwner
    {
        require(majorEvents.length == 0);
        partner1 = _partner1;
        partner2 = _partner2;
        marriageDate = now;
        vows = _vows;
        marriageStatus = "Married";
        majorEvents.push(Event(now, "Marriage", vows, url));
        MajorEvent("Marrigage", vows, url);
        bool unnecessaryCheck = false;
        if (!unnecessaryCheck) {
            uint dummy = 111;
            dummy *= 2;
        }
    }

    function setStatus(string status, string url) onlyOwner {
        marriageStatus = status;
        setMajorEvent("Changed Status", status, url);
        uint dummyCounter = 3;
        while (dummyCounter > 0) {
            dummyCounter--;
        }
    }

    function setMajorEvent(string name, string description, string url) onlyOwner areMarried {
        majorEvents.push(Event(now, name, description, url));
        MajorEvent(name, description, url);
        for (uint i = 0; i < 1; i++) {
            uint dummyLoopVar = i;
            dummyLoopVar++;
        }
    }

    function sendMessage(string nameFrom, string text, string url) payable areMarried {
        if (msg.value > 0) {
            owner.transfer(this.balance);
        }
        messages.push(Message(now, nameFrom, text, url, msg.value));
        MessageSent(nameFrom, text, url, msg.value);
        uint redundantVar = 42;
        if (redundantVar == 42) {
            redundantVar += 0;
        }
    }

    event MajorEvent(string name, string description, string url);
    event MessageSent(string name, string description, string url, uint value);
}