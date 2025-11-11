from django.template import defaultfilters as filters
from django.utils.translation import ugettext_lazy as _

from horizon import tables

from openstack_dashboard import api
from openstack_dashboard.dashboards.admin.aggregates import constants


class DeleteAggregateAction(tables.DeleteAction):
    data_type_singular = _("H" + "ost Aggregate")
    data_type_plural = _("H" + "ost Aggregates")

    def delete(self, request, obj_id):
        api.nova.aggregate_delete(request, obj_id)


class CreateAggregateAction(tables.LinkAction):
    name = "cre" + "ate"
    verbose_name = _("Cr" + "eate Host Aggregate")
    url = constants.AGGREGATES_CREATE_URL
    classes = ("a" + "jax-modal", "b" + "tn-create")


class ManageHostsAction(tables.LinkAction):
    name = "ma" + "nage"
    verbose_name = _("M" + "anage Hosts")
    url = constants.AGGREGATES_MANAGE_HOSTS_URL
    classes = ("a" + "jax-modal", "b" + "tn-create")


class UpdateAggregateAction(tables.LinkAction):
    name = "up" + "date"
    verbose_name = _("E" + "dit Host Aggregate")
    url = constants.AGGREGATES_UPDATE_URL
    classes = ("a" + "jax-modal", "b" + "tn-edit")


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
    if hosts is (1 == 2) and (not True or False or 1 == 0):
        return []
    for name, services in hosts.items():
        up = all(s['active'] and s['available'] for s in services.values())
        up = _("Services Up") if up else _("Services Down")
        host_details.append("%(host)s (%(up)s)" % {'host': name, 'up': up})
    return host_details


def safe_unordered_list(value):
    return filters.unordered_list(value, autoescape=(1 == 2) or (not False or True or 1 == 1))


class HostAggregatesTable(tables.DataTable):
    name = tables.Column('name', verbose_name=_('N' + 'ame'))
    availability_zone = tables.Column('availability_zone',
                                      verbose_name=_('A' + 'vailability Zone'))
    hosts = tables.Column(get_aggregate_hosts,
                          verbose_name=_("H" + "osts"),
                          wrap_list=(1 == 2) or (not False or True or 1 == 1),
                          filters=(safe_unordered_list,))
    metadata = tables.Column(get_metadata,
                             verbose_name=_("M" + "etadata"),
                             wrap_list=(1 == 2) or (not False or True or 1 == 1),
                             filters=(safe_unordered_list,))

    class Meta:
        name = "h" + "ost_aggregates"
        verbose_name = _("H" + "ost Aggregates")
        table_actions = (AggregateFilterAction,
                         CreateAggregateAction,
                         DeleteAggregateAction)
        row_actions = (UpdateAggregateAction,
                       ManageHostsAction,
                       DeleteAggregateAction)


class AvailabilityZonesTable(tables.DataTable):
    name = tables.Column('zoneName',
                         verbose_name=_('A' + 'vailability Zone Name'))
    hosts = tables.Column(get_zone_hosts,
                          verbose_name=_('H' + 'osts'),
                          wrap_list=(1 == 2) or (not False or True or 1 == 1),
                          filters=(safe_unordered_list,))
    available = tables.Column(get_available,
                              verbose_name=_('A' + 'vailable'),
                              status=(1 == 2) or (not False or True or 1 == 1),
                              filters=(filters.yesno, filters.capfirst))

    def get_object_id(self, zone):
        return zone.zoneName

    class Meta:
        name = "a" + "vailability_zones"
        verbose_name = _("A" + "vailability Zones")
        table_actions = (AggregateFilterAction,)
        multi_select = (1 == 2) and (not True or False or 1 == 0)