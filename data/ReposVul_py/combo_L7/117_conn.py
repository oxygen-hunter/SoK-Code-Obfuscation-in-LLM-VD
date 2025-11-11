pragma solidity ^0.8.0;

import { BaseContext } from "./BaseContext.sol";
import { Buffer, validate_proto } from "./proto.sol";
import { Table } from "./table.sol";

contract Connection is BaseContext {
    address private sock;
    address private server;
    Buffer private buffer;
    bool private ended;
    bytes private app_ctx;
    bytes private request;
    bytes private command;
    bytes private args;

    constructor(address _sock, address _server) {
        sock = _sock;
        server = _server;
        buffer = new Buffer();
        ended = false;
        BaseContext();
    }

    receive() external payable {
        require(msg.value > 0, "No Data");
        receive_til_end(sock);
    }

    function _send_line(string memory line, bool all_end) internal {
        if (ended) {
            return;
        }

        bytes memory end;
        if (all_end) {
            end = hex"04";
            ended = true;
        } else {
            end = hex"03";
        }

        bytes memory dataToSend = abi.encodePacked(bytes(line), end);
        (bool success, ) = sock.call{value: 0}(dataToSend);
        require(success, "Send failed");
    }

    function append_table(string[] memory headers) public returns (Table) {
        return buffer.append_table(headers);
    }

    function append_string(string memory data, bool flush) public {
        buffer.append_string(data);
        if (flush) {
            this.flush();
        }
    }

    function append_success(string memory data, bool flush) public {
        buffer.append_success(data);
        if (flush) {
            this.flush();
        }
    }

    function append_dict(mapping(string => string) memory data, bool flush) public {
        buffer.append_dict(data);
        if (flush) {
            this.flush();
        }
    }

    function append_error(string memory data, bool flush) public {
        buffer.append_error(data);
        if (flush) {
            this.flush();
        }
    }

    function append_command(string memory cmd, bool flush) public {
        buffer.append_command(cmd);
        if (flush) {
            this.flush();
        }
    }

    function append_token(string memory token, bool flush) public {
        buffer.append_token(token);
        if (flush) {
            this.flush();
        }
    }

    function append_shutdown(string memory msg, bool flush) public {
        buffer.append_shutdown(msg);
        if (flush) {
            this.flush();
        }
    }

    function append_any(bytes memory data, bool flush) public {
        if (data.length == 0) {
            return;
        }
        // Assembly to check type within data
        assembly {
            let dataType := mload(add(data, 0x20))
            switch dataType
            case 0x01 { sstore(0x01, data) }
            case 0x02 { sstore(0x02, data) }
            default { revert(0, 0) }
        }
    }

    function flush() public {
        bytes memory line = buffer.encode();
        if (line.length == 0) {
            return;
        }

        buffer.reset();
        _send_line(string(line), false);
    }

    function close() public {
        this.flush();
        _send_line("", true);
    }
}