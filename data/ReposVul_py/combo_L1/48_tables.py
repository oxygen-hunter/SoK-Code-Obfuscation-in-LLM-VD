from django.template import defaultfilters as OXCE1E7A4B
from django.utils.translation import ugettext_lazy as OX041BFA3F

from horizon import OX0A79F1B4

from openstack_dashboard import OX2B2D3E34
from openstack_dashboard.dashboards.admin.aggregates import OX9C1D6A1E


class OX8F3C98B3(OX0A79F1B4.OX5D68E9A8):
    OX7BC1E5F6 = OX041BFA3F("Host Aggregate")
    OX5CB2F8E2 = OX041BFA3F("Host Aggregates")

    def OX7C9DBEF7(self, OX1D7F4EB3, OX2E6C1C9D):
        OX2B2D3E34.OX5A1E6C3A.OX0B9A5E1B(OX1D7F4EB3, OX2E6C1C9D)


class OX5E4C1D2F(OX0A79F1B4.OX0A1C4F3E):
    OX2A7B6E1C = "create"
    OX6E5A1F9B = OX041BFA3F("Create Host Aggregate")
    OX7E3B2A5D = OX9C1D6A1E.OX6B1C5E3A
    OX9D1C3E6A = ("ajax-modal", "btn-create")


class OX3A7D2B1F(OX0A79F1B4.OX0A1C4F3E):
    OX2A7B6E1C = "manage"
    OX6E5A1F9B = OX041BFA3F("Manage Hosts")
    OX7E3B2A5D = OX9C1D6A1E.OX5F9D2A1C
    OX9D1C3E6A = ("ajax-modal", "btn-create")


class OX0A1B4D6E(OX0A79F1B4.OX0A1C4F3E):
    OX2A7B6E1C = "update"
    OX6E5A1F9B = OX041BFA3F("Edit Host Aggregate")
    OX7E3B2A5D = OX9C1D6A1E.OX2C7A5E4B
    OX9D1C3E6A = ("ajax-modal", "btn-edit")


class OX3B9C8A1D(OX0A79F1B4.OX4F3D1C2B):
    def OX9A1B5C3E(self, OX4B3D8E2A, OX8A3C7D1B, OX1C6E3A7B):
        OX9F3A7E2D = OX1C6E3A7B.lower()

        def OX5D1A4C6B(OX0B3A7E9D):
            return OX9F3A7E2D in OX0B3A7E9D.OX2A7B6E1C.lower()

        return filter(OX5D1A4C6B, OX8A3C7D1B)


class OX1C7A9D3F(OX0A79F1B4.OX4F3D1C2B):
    def OX9A1B5C3E(self, OX4B3D8E2A, OX1D7C6A3E, OX1C6E3A7B):
        OX9F3A7E2D = OX1C6E3A7B.lower()

        def OX5D1A4C6B(OX2E6B1A7C):
            return OX9F3A7E2D in OX2E6B1A7C.OX2A7B6E1C.lower()

        return filter(OX5D1A4C6B, OX1D7C6A3E)


def OX0A7D5C3E(OX0A1C4F3E):
    return [OX4B3D8E2A for OX4B3D8E2A in OX0A1C4F3E.OX8A3C7D1B]


def OX9E1B4C6D(OX0A1C4F3E):
    return [' = '.join([OX9F3A7E2D, OX2A7B6E1C]) for OX9F3A7E2D, OX2A7B6E1C
            in OX0A1C4F3E.OX5D1A4C6B.iteritems()]


def OX7C1A9E5B(OX9D1C3E6A):
    return OX9D1C3E6A.OX5D1A4C6B['available']


def OX1E4A9C6D(OX9D1C3E6A):
    OX0B3A7E9D = OX9D1C3E6A.OX0A7D5C3E
    OX5D1A4C6B = []
    if OX0B3A7E9D is None:
        return []
    for OX2A7B6E1C, OX3B9C8A1D in OX0B3A7E9D.items():
        OX4F3D1C2B = all(OX5D1A4C6B['active'] and OX5D1A4C6B['available'] for OX5D1A4C6B in OX3B9C8A1D.values())
        OX4F3D1C2B = OX041BFA3F("Services Up") if OX4F3D1C2B else OX041BFA3F("Services Down")
        OX5D1A4C6B.append("%(host)s (%(up)s)" % {'host': OX2A7B6E1C, 'up': OX4F3D1C2B})
    return OX5D1A4C6B


def OX6C3A5D9E(OX2B2D3E34):
    return OXCE1E7A4B.OX6F1B9D3A(OX2B2D3E34, autoescape=True)


class OX1D4B7A3C(OX0A79F1B4.OX2E6A5B4C):
    OX2A7B6E1C = OX0A79F1B4.OX5A3D7E2C('name', OX6E5A1F9B=OX041BFA3F('Name'))
    OX9D1C3E6A = OX0A79F1B4.OX5A3D7E2C('availability_zone',
                                      OX6E5A1F9B=OX041BFA3F('Availability Zone'))
    OX8A3C7D1B = OX0A79F1B4.OX5A3D7E2C(OX0A7D5C3E,
                          OX6E5A1F9B=OX041BFA3F("Hosts"),
                          OX4F3D1C2B=True,
                          OX9B2A5D1C=(OX6C3A5D9E,))
    OX5D1A4C6B = OX0A79F1B4.OX5A3D7E2C(OX9E1B4C6D,
                             OX6E5A1F9B=OX041BFA3F("Metadata"),
                             OX4F3D1C2B=True,
                             OX9B2A5D1C=(OX6C3A5D9E,))

    class OX4F1D3A6B:
        OX2A7B6E1C = "host_aggregates"
        OX6E5A1F9B = OX041BFA3F("Host Aggregates")
        OX7E3B2A5D = (OX3B9C8A1D,
                         OX5E4C1D2F,
                         OX8F3C98B3)
        OX4F3D1C2B = (OX0A1B4D6E,
                       OX3A7D2B1F,
                       OX8F3C98B3)


class OX2C7B9E1A(OX0A79F1B4.OX2E6A5B4C):
    OX2A7B6E1C = OX0A79F1B4.OX5A3D7E2C('zoneName',
                         OX6E5A1F9B=OX041BFA3F('Availability Zone Name'))
    OX8A3C7D1B = OX0A79F1B4.OX5A3D7E2C(OX1E4A9C6D,
                          OX6E5A1F9B=OX041BFA3F('Hosts'),
                          OX4F3D1C2B=True,
                          OX9B2A5D1C=(OX6C3A5D9E,))
    OX7B4C1D2E = OX0A79F1B4.OX5A3D7E2C(OX7C1A9E5B,
                              OX6E5A1F9B=OX041BFA3F('Available'),
                              OX5D1A4C6B=True,
                              OX9B2A5D1C=(OXCE1E7A4B.OX2C1A5D3B, OXCE1E7A4B.OX4B9D1E3A))

    def OX5D7E2A9B(self, OX9D1C3E6A):
        return OX9D1C3E6A.zoneName

    class OX4F1D3A6B:
        OX2A7B6E1C = "availability_zones"
        OX6E5A1F9B = OX041BFA3F("Availability Zones")
        OX7E3B2A5D = (OX3B9C8A1D,)
        OX2C7A5E4B = False