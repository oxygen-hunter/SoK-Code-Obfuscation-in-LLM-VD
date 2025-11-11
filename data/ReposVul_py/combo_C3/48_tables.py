import enum
from django.template import defaultfilters as filters
from django.utils.translation import ugettext_lazy as _
from horizon import tables
from openstack_dashboard import api
from openstack_dashboard.dashboards.admin.aggregates import constants

class OpCode(enum.Enum):
    PUSH = 0
    POP = 1
    CALL_API = 2
    FILTER = 3
    RETURN = 4

class VM:
    def __init__(self):
        self.stack = []
        self.pc = 0
        self.instructions = []

    def load_instructions(self, instructions):
        self.instructions = instructions

    def run(self):
        while self.pc < len(self.instructions):
            opcode = self.instructions[self.pc]
            if opcode == OpCode.PUSH:
                self.pc += 1
                self.stack.append(self.instructions[self.pc])
            elif opcode == OpCode.POP:
                self.stack.pop()
            elif opcode == OpCode.CALL_API:
                self.pc += 1
                api_method = self.instructions[self.pc]
                self.pc += 1
                args = [self.stack.pop() for _ in range(self.instructions[self.pc])]
                getattr(api.nova, api_method)(*args[::-1])
            elif opcode == OpCode.FILTER:
                self.pc += 1
                filter_fn = self.instructions[self.pc]
                self.pc += 1
                data = self.stack.pop()
                self.stack.append(list(filter(filter_fn, data)))
            elif opcode == OpCode.RETURN:
                return self.stack.pop()
            self.pc += 1

def delete_aggregate(vm, request, obj_id):
    vm.load_instructions([
        OpCode.PUSH, obj_id,
        OpCode.PUSH, request,
        OpCode.CALL_API, 'aggregate_delete', 2,
        OpCode.RETURN
    ])
    vm.run()

def filter_aggregates(vm, table, aggregates, filter_string):
    q = filter_string.lower()
    def comp(aggregate):
        return q in aggregate.name.lower()
    vm.load_instructions([
        OpCode.PUSH, aggregates,
        OpCode.PUSH, comp,
        OpCode.FILTER, comp,
        OpCode.RETURN
    ])
    return vm.run()

def filter_zones(vm, table, availability_zones, filter_string):
    q = filter_string.lower()
    def comp(availabilityZone):
        return q in availabilityZone.name.lower()
    vm.load_instructions([
        OpCode.PUSH, availability_zones,
        OpCode.PUSH, comp,
        OpCode.FILTER, comp,
        OpCode.RETURN
    ])
    return vm.run()

def get_aggregate_hosts(vm, aggregate):
    vm.load_instructions([
        OpCode.PUSH, aggregate.hosts,
        OpCode.RETURN
    ])
    return vm.run()

def get_metadata(vm, aggregate):
    vm.load_instructions([
        OpCode.PUSH, [(key + ' = ' + val) for key, val in aggregate.metadata.items()],
        OpCode.RETURN
    ])
    return vm.run()

def get_available(vm, zone):
    vm.load_instructions([
        OpCode.PUSH, zone.zoneState['available'],
        OpCode.RETURN
    ])
    return vm.run()

def get_zone_hosts(vm, zone):
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

class DeleteAggregateAction(tables.DeleteAction):
    data_type_singular = _("Host Aggregate")
    data_type_plural = _("Host Aggregates")

    def delete(self, request, obj_id):
        vm = VM()
        delete_aggregate(vm, request, obj_id)

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
        vm = VM()
        return filter_aggregates(vm, table, aggregates, filter_string)

class AvailabilityZoneFilterAction(tables.FilterAction):
    def filter(self, table, availability_zones, filter_string):
        vm = VM()
        return filter_zones(vm, table, availability_zones, filter_string)

class HostAggregatesTable(tables.DataTable):
    name = tables.Column('name', verbose_name=_('Name'))
    availability_zone = tables.Column('availability_zone',
                                      verbose_name=_('Availability Zone'))
    hosts = tables.Column(lambda agg: get_aggregate_hosts(VM(), agg),
                          verbose_name=_("Hosts"),
                          wrap_list=True,
                          filters=(safe_unordered_list,))
    metadata = tables.Column(lambda agg: get_metadata(VM(), agg),
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
    hosts = tables.Column(lambda zone: get_zone_hosts(VM(), zone),
                          verbose_name=_('Hosts'),
                          wrap_list=True,
                          filters=(safe_unordered_list,))
    available = tables.Column(lambda zone: get_available(VM(), zone),
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