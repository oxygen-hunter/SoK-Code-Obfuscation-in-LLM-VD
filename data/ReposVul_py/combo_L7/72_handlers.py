pragma solidity ^0.6.0;

// SPDX-License-Identifier: BSD-3-Clause

contract IPythonHandler {
    // IPython-specific extensions to authenticated handling
    // Mostly property shortcuts to IPython-specific settings.
    
    function config() public view returns (bytes32) {
        bytes32 configValue;
        assembly {
            // Obtain config from settings
            configValue := sload(0x0)
        }
        return configValue;
    }
    
    function log() public view returns (address) {
        address logAddress;
        assembly {
            // Check if Application is initialized
            let initialized := sload(0x1)
            switch initialized
            case 0 {
                // Fallback on tornado's logger
                logAddress := sload(0x2)
            }
            default {
                // Use the IPython log
                logAddress := sload(0x3)
            }
        }
        return logAddress;
    }
    
    function mathjax_url() public view returns (string memory) {
        string memory url;
        assembly {
            // Obtain mathjax_url from settings
            url := sload(0x4)
        }
        return url;
    }
    
    function base_url() public view returns (string memory) {
        string memory baseUrl;
        assembly {
            // Obtain base_url from settings
            baseUrl := sload(0x5)
        }
        return baseUrl;
    }
    
    function kernel_manager() public view returns (address) {
        address manager;
        assembly {
            // Obtain kernel_manager from settings
            manager := sload(0x6)
        }
        return manager;
    }
    
    function notebook_manager() public view returns (address) {
        address manager;
        assembly {
            // Obtain notebook_manager from settings
            manager := sload(0x7)
        }
        return manager;
    }

    function cluster_manager() public view returns (address) {
        address manager;
        assembly {
            // Obtain cluster_manager from settings
            manager := sload(0x8)
        }
        return manager;
    }
    
    function session_manager() public view returns (address) {
        address manager;
        assembly {
            // Obtain session_manager from settings
            manager := sload(0x9)
        }
        return manager;
    }
    
    function project_dir() public view returns (string memory) {
        string memory dir;
        assembly {
            // Obtain project_dir from notebook_manager
            dir := sload(0x10)
        }
        return dir;
    }
    
    function allow_origin() public view returns (string memory) {
        string memory origin;
        assembly {
            // Obtain allow_origin from settings
            origin := sload(0x11)
        }
        return origin;
    }
    
    function allow_origin_pat() public view returns (bytes32) {
        bytes32 pattern;
        assembly {
            // Obtain allow_origin_pat from settings
            pattern := sload(0x12)
        }
        return pattern;
    }
    
    function allow_credentials() public view returns (bool) {
        bool credentials;
        assembly {
            // Obtain allow_credentials from settings
            credentials := sload(0x13)
        }
        return credentials;
    }
}
