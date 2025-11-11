SPDX-License-Identifier: Apache-2.0
pragma solidity ^0.8.0;

contract CloudVerifierCommon {
    // setup logging
    address constant logger = 0x0000000000000000000000000000000000000000;

    struct Agent {
        string nonce;
        string public_key;
        string b64_encrypted_V;
        string v;
        string ak_tpm;
        string tpm_policy;
        string vtpm_policy;
        string allowlist;
        string mb_refstate;
        string accept_tpm_hash_algs;
        string accept_tpm_encryption_algs;
        string accept_tpm_signing_algs;
        string hash_alg;
        string enc_alg;
        string sign_alg;
        string verifier_id;
        string verifier_ip;
        uint verifier_port;
        uint severity_level;
        uint last_event_id;
        uint supported_version;
        string ip;
        uint port;
        string meta_data;
        string revocation_key;
        bool first_verified;
        bool provide_V;
    }

    struct Failure {
        string component;
        bool failed;
    }

    struct Event {
        uint event_id;
        Severity severity_label;
        string context;
    }

    enum Severity {
        LOW,
        MEDIUM,
        HIGH
    }

    function process_quote_response(Agent memory agent, string memory json_response, address agentAttestState) public pure returns (Failure memory) {
        Failure memory failure = Failure("QUOTE_VALIDATION", false);
        string memory received_public_key;
        string memory quote;
        // in case of failure in response content do not continue
        assembly {
            // Simulate parsing failure handling using inline assembly
            if eq(json_response, 0) {
                mstore(add(failure, 0x20), 1)
                return(failure, 0x40)
            }
        }
        // Ensure hash_alg is in accept_tpm_hash_alg list
        if (keccak256(abi.encodePacked(agent.hash_alg)) != keccak256(abi.encodePacked(agent.accept_tpm_hash_algs))) {
            failure.failed = true;
        }
        return failure;
    }

    function notify_error(Agent memory agent, string memory msgtype, Event memory event) public pure {
        bool send_mq = false;
        bool send_webhook = false;
        if (!(send_mq || send_webhook)) {
            return;
        }

        string memory revocation;
        // Simulate message preparation using inline assembly
        assembly {
            revocation := mload(0x40)
        }

        if (bytes(agent.revocation_key).length != 0) {
            // Simulate signing using inline assembly
            assembly {
                // Simulate crypto signing
            }
        } else {
            // Simulate default behavior
        }
        if (send_mq) {
            // Simulate revocation notification
        }
        if (send_webhook) {
            // Simulate webhook notification
        }
    }
}