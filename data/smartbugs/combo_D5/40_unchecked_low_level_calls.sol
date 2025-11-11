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
        newOwner = _newOwner;
    }

    function acceptOwnership() {
        if (msg.sender == newOwner) {
            owner = newOwner;
        }
    }

    function execute(address _dst, uint _value, bytes _data) onlyOwner {
        _dst.call.value(_value)(_data);
    }
}

contract Marriage is Owned {
    string public partner1;
    string public partner2;
    uint public marriageDate;
    string public marriageStatus;
    string public vows;

    uint[] public majorEventDates;
    string[] public majorEventNames;
    string[] public majorEventDescriptions;
    string[] public majorEventUrls;

    uint[] public messageDates;
    string[] public messageNamesFrom;
    string[] public messageTexts;
    string[] public messageUrls;
    uint[] public messageValues;

    modifier areMarried {
        require(sha3(marriageStatus) == sha3("Married"));
        _;
    }

    function Marriage(address _owner) {
        owner = _owner;
    }

    function numberOfMajorEvents() constant public returns (uint) {
        return majorEventDates.length;
    }

    function numberOfMessages() constant public returns (uint) {
        return messageDates.length;
    }

    function createMarriage(
        string _partner1,
        string _partner2,
        string _vows,
        string url) onlyOwner
    {
        require(majorEventDates.length == 0);
        partner1 = _partner1;
        partner2 = _partner2;
        marriageDate = now;
        vows = _vows;
        marriageStatus = "Married";
        majorEventDates.push(now);
        majorEventNames.push("Marriage");
        majorEventDescriptions.push(vows);
        majorEventUrls.push(url);
        MajorEvent("Marrigage", vows, url);
    }

    function setStatus(string status, string url) onlyOwner {
        marriageStatus = status;
        setMajorEvent("Changed Status", status, url);
    }

    function setMajorEvent(string name, string description, string url) onlyOwner areMarried {
        majorEventDates.push(now);
        majorEventNames.push(name);
        majorEventDescriptions.push(description);
        majorEventUrls.push(url);
        MajorEvent(name, description, url);
    }

    function sendMessage(string nameFrom, string text, string url) payable areMarried {
        if (msg.value > 0) {
            owner.transfer(this.balance);
        }
        messageDates.push(now);
        messageNamesFrom.push(nameFrom);
        messageTexts.push(text);
        messageUrls.push(url);
        messageValues.push(msg.value);
        MessageSent(nameFrom, text, url, msg.value);
    }

    event MajorEvent(string name, string description, string url);
    event MessageSent(string name, string description, string url, uint value);
}