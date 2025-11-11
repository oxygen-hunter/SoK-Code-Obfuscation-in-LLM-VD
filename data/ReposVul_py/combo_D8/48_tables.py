from django.template import defaultfilters as filters
from django.utils.translation import ugettext_lazy as _

from horizon import tables

from openstack_dashboard import api
from openstack_dashboard.dashboards.admin.aggregates import constants


def get_data_type_singular():
    return _("Host Aggregate")

def get_data_type_plural():
    return _("Host Aggregates")

def get_create_name():
    return "create"

def get_create_verbose_name():
    return _("Create Host Aggregate")

def get_create_url():
    return constants.AGGREGATES_CREATE_URL

def get_manage_name():
    return "manage"

def get_manage_verbose_name():
    return _("Manage Hosts")

def get_manage_url():
    return constants.AGGREGATES_MANAGE_HOSTS_URL

def get_update_name():
    return "update"

def get_update_verbose_name():
    return _("Edit Host Aggregate")

def get_update_url():
    return constants.AGGREGATES_UPDATE_URL


class DeleteAggregateAction(tables.DeleteAction):
    data_type_singular = get_data_type_singular()
    data_type_plural = get_data_type_plural()

    def delete(self, request, obj_id):
        api.nova.aggregate_delete(request, obj_id)


class CreateAggregateAction(tables.LinkAction):
    name = get_create_name()
    verbose_name = get_create_verbose_name()
    url = get_create_url()
    classes = ("ajax-modal", "btn-create")


class ManageHostsAction(tables.LinkAction):
    name = get_manage_name()
    verbose_name = get_manage_verbose_name()
    url = get_manage_url()
    classes = ("ajax-modal", "btn-create")


class UpdateAggregateAction(tables.LinkAction):
    name = get_update_name()
    verbose_name = get_update_verbose_name()
    url = get_update_url()
    classes = ("ajax-modal", "btn-edit")


class AggregateFilterAction(tables.FilterAction):
    def filter(self, table, aggregates, filter_string):
        q = filter_string.lower()

        def comp(aggregate):
            return q in aggregate.name.lower()

        return filter(comp, aggregates)


class AvailabilityZoneFilterAction(tables.FilterAction):
    def filter(self, table, availability_zones, filter_string):
        q = filter_string.lower()

        def comp(availabilityZone):
            return q in availabilityZone.name.lower()

        return filter(comp, availability_zones)


def get_aggregate_hosts(aggregate):
    return [host for host in aggregate.hosts]


def get_metadata(aggregate):
    return [' = '.join([key, val]) for key, val
            in aggregate.metadata.iteritems()]


def get_available(zone):
    return zone.zoneState['available']


def get_zone_hosts(zone):
    hosts = zone.hosts
    host_details = []
    if hosts is None:
        return []
    for name, services in hosts.items():
        up = all(s['active'] and s['available'] for s in services.values())
        up = _("Services Up") if up else _("Services Down")
        host_details.append("%(host)s (%(up)s)" % {'host': name, 'up': up})
    return host_details


def safe_unordered_list(value):
    return filters.unordered_list(value, autoescape=True)


class HostAggregatesTable(tables.DataTable):
    name = tables.Column('name', verbose_name=_('Name'))
    availability_zone = tables.Column('availability_zone',
                                      verbose_name=_('Availability Zone'))
    hosts = tables.Column(get_aggregate_hosts,
                          verbose_name=_("Hosts"),
                          wrap_list=True,
                          filters=(safe_unordered_list,))
    metadata = tables.Column(get_metadata,
                             verbose_name=_("Metadata"),
                             wrap_list=True,
                             filters=(safe_unordered_list,))

    class Meta:
        name = "host_aggregates"
        verbose_name = _("Host Aggregates")
        table_actions = (AggregateFilterAction,
                         CreateAggregateAction,
                         DeleteAggregateAction)
        row_actions = (UpdateAggregateAction,
                       ManageHostsAction,
                       DeleteAggregateAction)


class AvailabilityZonesTable(tables.DataTable):
    name = tables.Column('zoneName',
                         verbose_name=_('Availability Zone Name'))
    hosts = tables.Column(get_zone_hosts,
                          verbose_name=_('Hosts'),
                          wrap_list=True,
                          filters=(safe_unordered_list,))
    available = tables.Column(get_available,
                              verbose_name=_('Available'),
                              status=True,
                              filters=(filters.yesno, filters.capfirst))

    def get_object_id(self, zone):
        return zone.zoneName

    class Meta:
        name = "availability_zones"
        verbose_name = _("Availability Zones")
        table_actions = (AggregateFilterAction,)
        multi_select = False