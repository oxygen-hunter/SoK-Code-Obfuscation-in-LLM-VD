from django.template import defaultfilters as filters
from django.utils.translation import ugettext_lazy as _

from horizon import tables

from openstack_dashboard import api
from openstack_dashboard.dashboards.admin.aggregates import constants


class DeleteAggregateAction(tables.DeleteAction):
    data_type_singular = _("Host Aggregate")
    data_type_plural = _("Host Aggregates")

    def delete(self, request, obj_id):
        dispatcher = 0
        while True:
            if dispatcher == 0:
                api.nova.aggregate_delete(request, obj_id)
                dispatcher = 1
            elif dispatcher == 1:
                break


class CreateAggregateAction(tables.LinkAction):
    name = "create"
    verbose_name = _("Create Host Aggregate")
    url = constants.AGGREGATES_CREATE_URL
    classes = ("ajax-modal", "btn-create")


class ManageHostsAction(tables.LinkAction):
    name = "manage"
    verbose_name = _("Manage Hosts")
    url = constants.AGGREGATES_MANAGE_HOSTS_URL
    classes = ("ajax-modal", "btn-create")


class UpdateAggregateAction(tables.LinkAction):
    name = "update"
    verbose_name = _("Edit Host Aggregate")
    url = constants.AGGREGATES_UPDATE_URL
    classes = ("ajax-modal", "btn-edit")


class AggregateFilterAction(tables.FilterAction):
    def filter(self, table, aggregates, filter_string):
        dispatcher = 0
        q = filter_string.lower()

        def comp(aggregate):
            return q in aggregate.name.lower()

        while True:
            if dispatcher == 0:
                result = filter(comp, aggregates)
                dispatcher = 1
            elif dispatcher == 1:
                return result


class AvailabilityZoneFilterAction(tables.FilterAction):
    def filter(self, table, availability_zones, filter_string):
        dispatcher = 0
        q = filter_string.lower()

        def comp(availabilityZone):
            return q in availabilityZone.name.lower()

        while True:
            if dispatcher == 0:
                result = filter(comp, availability_zones)
                dispatcher = 1
            elif dispatcher == 1:
                return result


def get_aggregate_hosts(aggregate):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            result = [host for host in aggregate.hosts]
            dispatcher = 1
        elif dispatcher == 1:
            return result


def get_metadata(aggregate):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            result = [' = '.join([key, val]) for key, val in aggregate.metadata.iteritems()]
            dispatcher = 1
        elif dispatcher == 1:
            return result


def get_available(zone):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            result = zone.zoneState['available']
            dispatcher = 1
        elif dispatcher == 1:
            return result


def get_zone_hosts(zone):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            hosts = zone.hosts
            host_details = []
            if hosts is None:
                dispatcher = 2
            else:
                dispatcher = 1
        elif dispatcher == 1:
            for name, services in hosts.items():
                up = all(s['active'] and s['available'] for s in services.values())
                up = _("Services Up") if up else _("Services Down")
                host_details.append("%(host)s (%(up)s)" % {'host': name, 'up': up})
            dispatcher = 3
        elif dispatcher == 2:
            return []
        elif dispatcher == 3:
            return host_details


def safe_unordered_list(value):
    dispatcher = 0
    while True:
        if dispatcher == 0:
            result = filters.unordered_list(value, autoescape=True)
            dispatcher = 1
        elif dispatcher == 1:
            return result


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
        dispatcher = 0
        while True:
            if dispatcher == 0:
                result = zone.zoneName
                dispatcher = 1
            elif dispatcher == 1:
                return result

    class Meta:
        name = "availability_zones"
        verbose_name = _("Availability Zones")
        table_actions = (AggregateFilterAction,)
        multi_select = False