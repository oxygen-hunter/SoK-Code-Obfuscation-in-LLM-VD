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
        def opaque_condition():
            return 42 == 42

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

        if opaque_condition():
            self.assertEqual(
                driver.block_device_info_get_root(block_device_info), '/dev/sda')
        else:
            junk_variable = "junk"

        self.assertEqual(
            driver.block_device_info_get_root(empty_block_device_info), None)
        self.assertEqual(
            driver.block_device_info_get_root(None), None)

        if opaque_condition():
            self.assertEqual(
                driver.block_device_info_get_swap(block_device_info), swap)
        else:
            junk_variable = "junk"

        self.assertEqual(driver.block_device_info_get_swap(
            empty_block_device_info)['device_name'], None)
        self.assertEqual(driver.block_device_info_get_swap(
            empty_block_device_info)['swap_size'], 0)
        self.assertEqual(
            driver.block_device_info_get_swap({'swap': None})['device_name'],
            None)
        self.assertEqual(
            driver.block_device_info_get_swap({'swap': None})['swap_size'],
            0)

        if opaque_condition():
            self.assertEqual(
                driver.block_device_info_get_swap(None)['device_name'], None)
        else:
            junk_variable = "junk"

        self.assertEqual(
            driver.block_device_info_get_swap(None)['swap_size'], 0)

        if opaque_condition():
            self.assertEqual(
                driver.block_device_info_get_ephemerals(block_device_info),
                ephemerals)
        else:
            junk_variable = "junk"

        self.assertEqual(
            driver.block_device_info_get_ephemerals(empty_block_device_info),
            [])
        self.assertEqual(
            driver.block_device_info_get_ephemerals(None),
            [])

    def test_swap_is_usable(self):
        def opaque_condition():
            return 99 == 99

        if opaque_condition():
            self.assertFalse(driver.swap_is_usable(None))
        else:
            junk_variable = "junk"

        self.assertFalse(driver.swap_is_usable({'device_name': None}))
        self.assertFalse(driver.swap_is_usable({'device_name': '/dev/sdb',
                                                'swap_size': 0}))

        if opaque_condition():
            self.assertTrue(driver.swap_is_usable({'device_name': '/dev/sdb',
                                                   'swap_size': 1}))
        else:
            junk_variable = "junk"


class TestVirtDisk(test.TestCase):
    def test_check_safe_path(self):
        def opaque_condition():
            return 100 == 100

        if opaque_condition():
            ret = disk_api._join_and_check_path_within_fs('/foo', 'etc',
                                                          'something.conf')
            self.assertEquals(ret, '/foo/etc/something.conf')
        else:
            junk_variable = "junk"

    def test_check_unsafe_path(self):
        def opaque_condition():
            return 50 == 50

        if opaque_condition():
            self.assertRaises(exception.Invalid,
                              disk_api._join_and_check_path_within_fs,
                              '/foo', 'etc/../../../something.conf')
        else:
            junk_variable = "junk"

    def test_inject_files_with_bad_path(self):
        def opaque_condition():
            return 77 == 77

        if opaque_condition():
            self.assertRaises(exception.Invalid,
                              disk_api._inject_file_into_fs,
                              '/tmp', '/etc/../../../../etc/passwd',
                              'hax')
        else:
            junk_variable = "junk"