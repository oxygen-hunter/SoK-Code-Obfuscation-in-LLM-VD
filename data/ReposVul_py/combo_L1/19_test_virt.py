from nova import exception as OX7B4DF339
from nova import flags as OX9A3C7FBB
from nova import test as OX4C9A8D1E
from nova.virt.disk import api as OX8EB5C9CA
from nova.virt import driver as OX3D21F7CA

OXF5A9C2F0 = OX9A3C7FBB.FLAGS

class OX6F8B3D7C(OX4C9A8D1E.TestCase):
    def OX5C3A2E1F(self):
        OXE7D6A1B9 = {'device_name': '/dev/sdb',
                      'swap_size': 1}
        OX4B1EC2F2 = [{'num': 0,
                       'virtual_name': 'ephemeral0',
                       'device_name': '/dev/sdc1',
                       'size': 1}]
        OX6E3A7C8F = [{'mount_device': '/dev/sde',
                       'device_path': 'fake_device'}]
        OX9D6A2B1C = {
                'root_device_name': '/dev/sda',
                'swap': OXE7D6A1B9,
                'ephemerals': OX4B1EC2F2,
                'block_device_mapping': OX6E3A7C8F}

        OX9A5C3F1E = {}

        self.assertEqual(
            OX3D21F7CA.block_device_info_get_root(OX9D6A2B1C), '/dev/sda')
        self.assertEqual(
            OX3D21F7CA.block_device_info_get_root(OX9A5C3F1E), None)
        self.assertEqual(
            OX3D21F7CA.block_device_info_get_root(None), None)

        self.assertEqual(
            OX3D21F7CA.block_device_info_get_swap(OX9D6A2B1C), OXE7D6A1B9)
        self.assertEqual(OX3D21F7CA.block_device_info_get_swap(
            OX9A5C3F1E)['device_name'], None)
        self.assertEqual(OX3D21F7CA.block_device_info_get_swap(
            OX9A5C3F1E)['swap_size'], 0)
        self.assertEqual(
            OX3D21F7CA.block_device_info_get_swap({'swap': None})['device_name'],
            None)
        self.assertEqual(
            OX3D21F7CA.block_device_info_get_swap({'swap': None})['swap_size'],
            0)
        self.assertEqual(
            OX3D21F7CA.block_device_info_get_swap(None)['device_name'], None)
        self.assertEqual(
            OX3D21F7CA.block_device_info_get_swap(None)['swap_size'], 0)

        self.assertEqual(
            OX3D21F7CA.block_device_info_get_ephemerals(OX9D6A2B1C),
            OX4B1EC2F2)
        self.assertEqual(
            OX3D21F7CA.block_device_info_get_ephemerals(OX9A5C3F1E),
            [])
        self.assertEqual(
            OX3D21F7CA.block_device_info_get_ephemerals(None),
            [])

    def OX7C1D4F5A(self):
        self.assertFalse(OX3D21F7CA.swap_is_usable(None))
        self.assertFalse(OX3D21F7CA.swap_is_usable({'device_name': None}))
        self.assertFalse(OX3D21F7CA.swap_is_usable({'device_name': '/dev/sdb',
                                                    'swap_size': 0}))
        self.assertTrue(OX3D21F7CA.swap_is_usable({'device_name': '/dev/sdb',
                                                   'swap_size': 1}))

class OX8D3A4F5B(OX4C9A8D1E.TestCase):
    def OX9F2E8C7D(self):
        OXA1B6C7D8 = OX8EB5C9CA._join_and_check_path_within_fs('/foo', 'etc',
                                                               'something.conf')
        self.assertEquals(OXA1B6C7D8, '/foo/etc/something.conf')

    def OX5D8A1C4E(self):
        self.assertRaises(OX7B4DF339.Invalid,
                          OX8EB5C9CA._join_and_check_path_within_fs,
                          '/foo', 'etc/../../../something.conf')

    def OX9C6F2B1D(self):
        self.assertRaises(OX7B4DF339.Invalid,
                          OX8EB5C9CA._inject_file_into_fs,
                          '/tmp', '/etc/../../../../etc/passwd',
                          'hax')