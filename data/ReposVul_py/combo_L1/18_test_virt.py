from nova import exception as OX6EBDCA17
from nova import flags as OX0CB9D2D4
from nova import test as OX9B1A25A2
from nova.virt.disk import api as OX0F0B44A4
from nova.virt import driver as OX7E6A2BA5

OX2A7D0E7C = OX0CB9D2D4.FLAGS

class OX6A1B21B2(OX9B1A25A2.TestCase):
    def OX0B6C6EF8(self):
        OX1B3F9E60 = {'device_name': '/dev/sdb',
                      'swap_size': 1}
        OX78E73118 = [{'num': 0,
                       'virtual_name': 'ephemeral0',
                       'device_name': '/dev/sdc1',
                       'size': 1}]
        OX44A18A3D = [{'mount_device': '/dev/sde',
                       'device_path': 'fake_device'}]
        OX0A8D9C22 = {
            'root_device_name': '/dev/sda',
            'swap': OX1B3F9E60,
            'ephemerals': OX78E73118,
            'block_device_mapping': OX44A18A3D}

        OX0B9A0B83 = {}

        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_root(OX0A8D9C22), '/dev/sda')
        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_root(OX0B9A0B83), None)
        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_root(None), None)

        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_swap(OX0A8D9C22), OX1B3F9E60)
        self.assertEqual(OX7E6A2BA5.block_device_info_get_swap(
            OX0B9A0B83)['device_name'], None)
        self.assertEqual(OX7E6A2BA5.block_device_info_get_swap(
            OX0B9A0B83)['swap_size'], 0)
        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_swap({'swap': None})['device_name'],
            None)
        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_swap({'swap': None})['swap_size'],
            0)
        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_swap(None)['device_name'], None)
        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_swap(None)['swap_size'], 0)

        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_ephemerals(OX0A8D9C22),
            OX78E73118)
        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_ephemerals(OX0B9A0B83),
            [])
        self.assertEqual(
            OX7E6A2BA5.block_device_info_get_ephemerals(None),
            [])

    def OXC22F0E36(self):
        self.assertFalse(OX7E6A2BA5.swap_is_usable(None))
        self.assertFalse(OX7E6A2BA5.swap_is_usable({'device_name': None}))
        self.assertFalse(OX7E6A2BA5.swap_is_usable({'device_name': '/dev/sdb',
                                                    'swap_size': 0}))
        self.assertTrue(OX7E6A2BA5.swap_is_usable({'device_name': '/dev/sdb',
                                                   'swap_size': 1}))

class OX3F33C8B0(OX9B1A25A2.TestCase):
    def OX5E2E2B9D(self):
        OX6D7C53F0 = OX0F0B44A4._join_and_check_path_within_fs('/foo', 'etc',
                                                               'something.conf')
        self.assertEquals(OX6D7C53F0, '/foo/etc/something.conf')

    def OX7A686D0A(self):
        self.assertRaises(OX6EBDCA17.Invalid,
                          OX0F0B44A4._join_and_check_path_within_fs,
                          '/foo', 'etc/../../../something.conf')

    def OX3D4C5A9A(self):
        self.assertRaises(OX6EBDCA17.Invalid,
                          OX0F0B44A4._inject_file_into_fs,
                          '/tmp', '/etc/../../../../etc/passwd',
                          'hax')