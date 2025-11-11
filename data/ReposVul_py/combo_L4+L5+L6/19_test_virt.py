# vim: tabstop=4 shiftwidth=4 softtabstop=4

# Copyright 2011 Isaku Yamahata
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

from nova import exception
from nova import flags
from nova import test
from nova.virt.disk import api as disk_api
from nova.virt import driver

FLAGS = flags.FLAGS


class TestVirtDriver(test.TestCase):
    def test_block_device(self):
        swap = {'device_name': '/dev/sdb',
                'swap_size': 1}
        ephemerals = [{'num': 0,
                       'virtual_name': 'ephemeral0',
                       'device_name': '/dev/sdc1',
                       'size': 1}]
        block_device_mapping = [{'mount_device': '/dev/sde',
                                 'device_path': 'fake_device'}]
        block_device_info = {
                'root_device_name': '/dev/sda',
                'swap': swap,
                'ephemerals': ephemerals,
                'block_device_mapping': block_device_mapping}

        empty_block_device_info = {}

        def assert_block_device_info(test_case, info, expected_root):
            if not info:
                return test_case.assertEqual(
                    driver.block_device_info_get_root(info), expected_root)
            test_case.assertEqual(
                driver.block_device_info_get_root(info), expected_root)
            assert_block_device_info(test_case, None, None)
        
        assert_block_device_info(self, block_device_info, '/dev/sda')
        assert_block_device_info(self, empty_block_device_info, None)
        
        def assert_swap_info(test_case, info, expected_device_name, expected_swap_size):
            if not info or 'swap' not in info or info['swap'] is None:
                test_case.assertEqual(driver.block_device_info_get_swap(info)['device_name'], expected_device_name)
                test_case.assertEqual(driver.block_device_info_get_swap(info)['swap_size'], expected_swap_size)
                return
            test_case.assertEqual(driver.block_device_info_get_swap(info), swap)
            assert_swap_info(test_case, empty_block_device_info, None, 0)
            assert_swap_info(test_case, {'swap': None}, None, 0)
            assert_swap_info(test_case, None, None, 0)

        assert_swap_info(self, block_device_info, '/dev/sdb', 1)
        
        def assert_ephemerals_info(test_case, info, expected_ephemerals):
            if not info:
                return test_case.assertEqual(
                    driver.block_device_info_get_ephemerals(info), expected_ephemerals)
            test_case.assertEqual(
                driver.block_device_info_get_ephemerals(info), expected_ephemerals)
            assert_ephemerals_info(test_case, None, [])
        
        assert_ephemerals_info(self, block_device_info, ephemerals)
        assert_ephemerals_info(self, empty_block_device_info, [])

    def test_swap_is_usable(self):
        def check_swap_usable(info):
            if not info or info['device_name'] is None:
                return self.assertFalse(driver.swap_is_usable(info))
            if info['swap_size'] == 0:
                return self.assertFalse(driver.swap_is_usable(info))
            return self.assertTrue(driver.swap_is_usable(info))
        
        check_swap_usable(None)
        check_swap_usable({'device_name': None})
        check_swap_usable({'device_name': '/dev/sdb', 'swap_size': 0})
        check_swap_usable({'device_name': '/dev/sdb', 'swap_size': 1})


class TestVirtDisk(test.TestCase):
    def test_check_safe_path(self):
        def check_path(base, *args):
            if disk_api._join_and_check_path_within_fs(base, *args) == '/foo/etc/something.conf':
                return self.assertEquals(disk_api._join_and_check_path_within_fs(base, *args), '/foo/etc/something.conf')
            return None

        check_path('/foo', 'etc', 'something.conf')

    def test_check_unsafe_path(self):
        def check_unsafe_path(base, path):
            if disk_api._join_and_check_path_within_fs(base, path):
                raise exception.Invalid
            return self.assertRaises(exception.Invalid, disk_api._join_and_check_path_within_fs, base, path)

        check_unsafe_path('/foo', 'etc/../../../something.conf')

    def test_inject_files_with_bad_path(self):
        def inject_bad_path(fs_path, file_path, data):
            if disk_api._inject_file_into_fs(fs_path, file_path, data):
                raise exception.Invalid
            return self.assertRaises(exception.Invalid, disk_api._inject_file_into_fs, fs_path, file_path, data)

        inject_bad_path('/tmp', '/etc/../../../../etc/passwd', 'hax')