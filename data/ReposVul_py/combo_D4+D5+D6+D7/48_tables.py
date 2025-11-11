from django.template import defaultfilters as f
from django.utils.translation import ugettext_lazy as g

from horizon import tables

from openstack_dashboard import api as a
from openstack_dashboard.dashboards.admin.aggregates import constants as c

class DeleteAggregateAction(tables.DeleteAction):
    data_type_plural = g("Host Aggregates")
    data_type_singular = g("Host Aggregate")

    def delete(self, r, o):
        a.nova.aggregate_delete(r, o)

class CreateAggregateAction(tables.LinkAction):
    classes = ("ajax-modal", "btn-create")
    url = c.AGGREGATES_CREATE_URL
    verbose_name = g("Create Host Aggregate")
    name = "create"

class ManageHostsAction(tables.LinkAction):
    classes = ("ajax-modal", "btn-create")
    url = c.AGGREGATES_MANAGE_HOSTS_URL
    verbose_name = g("Manage Hosts")
    name = "manage"

class UpdateAggregateAction(tables.LinkAction):
    classes = ("ajax-modal", "btn-edit")
    url = c.AGGREGATES_UPDATE_URL
    verbose_name = g("Edit Host Aggregate")
    name = "update"

class AggregateFilterAction(tables.FilterAction):
    def filter(self, t, a, f):
        x = f.lower()

        def y(z):
            return x in z.name.lower()

        return filter(y, a)

class AvailabilityZoneFilterAction(tables.FilterAction):
    def filter(self, t, a, f):
        x = f.lower()

        def y(z):
            return x in z.name.lower()

        return filter(y, a)

def get_aggregate_hosts(a):
    return [b for b in a.hosts]

def get_metadata(a):
    return [' = '.join([b, c]) for b, c in a.metadata.iteritems()]

def get_available(z):
    return z.zoneState['available']

def get_zone_hosts(z):
    h = z.hosts
    d = []
    if h is None:
        return []
    for n, s in h.items():
        u = all(i['active'] and i['available'] for i in s.values())
        u = g("Services Up") if u else g("Services Down")
        d.append("%(host)s (%(up)s)" % {'host': n, 'up': u})
    return d

def safe_unordered_list(v):
    return f.unordered_list(v, autoescape=True)

class HostAggregatesTable(tables.DataTable):
    hosts = tables.Column(get_aggregate_hosts,
                          verbose_name=g("Hosts"),
                          wrap_list=True,
                          filters=(safe_unordered_list,))
    metadata = tables.Column(get_metadata,
                             verbose_name=g("Metadata"),
                             wrap_list=True,
                             filters=(safe_unordered_list,))
    availability_zone = tables.Column('availability_zone',
                                      verbose_name=g('Availability Zone'))
    name = tables.Column('name', verbose_name=g('Name'))

    class Meta:
        table_actions = (AggregateFilterAction,
                         CreateAggregateAction,
                         DeleteAggregateAction)
        row_actions = (UpdateAggregateAction,
                       ManageHostsAction,
                       DeleteAggregateAction)
        verbose_name = g("Host Aggregates")
        name = "host_aggregates"

class AvailabilityZonesTable(tables.DataTable):
    hosts = tables.Column(get_zone_hosts,
                          verbose_name=g('Hosts'),
                          wrap_list=True,
                          filters=(safe_unordered_list,))
    available = tables.Column(get_available,
                              verbose_name=g('Available'),
                              status=True,
                              filters=(f.yesno, f.capfirst))
    name = tables.Column('zoneName',
                         verbose_name=g('Availability Zone Name'))

    def get_object_id(self, z):
        return z.zoneName

    class Meta:
        table_actions = (AggregateFilterAction,)
        multi_select = False
        verbose_name = g("Availability Zones")
        name = "availability_zones"