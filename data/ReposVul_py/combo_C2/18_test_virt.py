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

        state = 0
        while True:
            if state == 0:
                self.assertEqual(
                    driver.block_device_info_get_root(block_device_info), '/dev/sda')
                state = 1
            elif state == 1:
                self.assertEqual(
                    driver.block_device_info_get_root(empty_block_device_info), None)
                state = 2
            elif state == 2:
                self.assertEqual(
                    driver.block_device_info_get_root(None), None)
                state = 3
            elif state == 3:
                self.assertEqual(
                    driver.block_device_info_get_swap(block_device_info), swap)
                state = 4
            elif state == 4:
                self.assertEqual(driver.block_device_info_get_swap(
                    empty_block_device_info)['device_name'], None)
                state = 5
            elif state == 5:
                self.assertEqual(driver.block_device_info_get_swap(
                    empty_block_device_info)['swap_size'], 0)
                state = 6
            elif state == 6:
                self.assertEqual(
                    driver.block_device_info_get_swap({'swap': None})['device_name'],
                    None)
                state = 7
            elif state == 7:
                self.assertEqual(
                    driver.block_device_info_get_swap({'swap': None})['swap_size'],
                    0)
                state = 8
            elif state == 8:
                self.assertEqual(
                    driver.block_device_info_get_swap(None)['device_name'], None)
                state = 9
            elif state == 9:
                self.assertEqual(
                    driver.block_device_info_get_swap(None)['swap_size'], 0)
                state = 10
            elif state == 10:
                self.assertEqual(
                    driver.block_device_info_get_ephemerals(block_device_info),
                    ephemerals)
                state = 11
            elif state == 11:
                self.assertEqual(
                    driver.block_device_info_get_ephemerals(empty_block_device_info),
                    [])
                state = 12
            elif state == 12:
                self.assertEqual(
                    driver.block_device_info_get_ephemerals(None),
                    [])
                break

    def test_swap_is_usable(self):
        state = 0
        while True:
            if state == 0:
                self.assertFalse(driver.swap_is_usable(None))
                state = 1
            elif state == 1:
                self.assertFalse(driver.swap_is_usable({'device_name': None}))
                state = 2
            elif state == 2:
                self.assertFalse(driver.swap_is_usable({'device_name': '/dev/sdb',
                                                        'swap_size': 0}))
                state = 3
            elif state == 3:
                self.assertTrue(driver.swap_is_usable({'device_name': '/dev/sdb',
                                                       'swap_size': 1}))
                break


class TestVirtDisk(test.TestCase):
    def test_check_safe_path(self):
        ret = disk_api._join_and_check_path_within_fs('/foo', 'etc',
                                                      'something.conf')
        self.assertEquals(ret, '/foo/etc/something.conf')

    def test_check_unsafe_path(self):
        self.assertRaises(exception.Invalid,
                          disk_api._join_and_check_path_within_fs,
                          '/foo', 'etc/../../../something.conf')

    def test_inject_files_with_bad_path(self):
        self.assertRaises(exception.Invalid,
                          disk_api._inject_file_into_fs,
                          '/tmp', '/etc/../../../../etc/passwd',
                          'hax')