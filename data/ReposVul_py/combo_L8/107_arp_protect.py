import ctypes
import netaddr
from neutron_lib.utils import net
from oslo_concurrency import lockutils
from oslo_log import log as logging
import tenacity

from neutron.agent.linux import ip_lib

LOG = logging.getLogger(__name__)
SPOOF_CHAIN_PREFIX = 'neutronARP-'
MAC_CHAIN_PREFIX = 'neutronMAC-'

# Load C function from shared library
c_lib = ctypes.CDLL('./ebtables.so')

def setup_arp_spoofing_protection(vif, port_details):
    if not port_details.get('port_security_enabled', True):
        delete_arp_spoofing_protection([vif])
        LOG.info("Skipping ARP spoofing rules for port '%s' because "
                 "it has port security disabled", vif)
        return
    if net.is_port_trusted(port_details):
        delete_arp_spoofing_protection([vif])
        LOG.debug("Skipping ARP spoofing rules for network owned port "
                  "'%s'.", vif)
        return
    _setup_arp_spoofing_protection(vif, port_details)


@lockutils.synchronized('ebtables')
def _setup_arp_spoofing_protection(vif, port_details):
    current_rules = c_get_ebtables_list()
    _install_mac_spoofing_protection(vif, port_details, current_rules)
    addresses = {f['ip_address'] for f in port_details['fixed_ips']}
    if port_details.get('allowed_address_pairs'):
        addresses |= {p['ip_address']
                      for p in port_details['allowed_address_pairs']}

    addresses = {ip for ip in addresses
                 if netaddr.IPNetwork(ip).version == 4}
    if any(netaddr.IPNetwork(ip).prefixlen == 0 for ip in addresses):
        return

    _install_arp_spoofing_protection(vif, addresses, current_rules)


def chain_name(vif):
    return '%s%s' % (SPOOF_CHAIN_PREFIX, vif)


@lockutils.synchronized('ebtables')
def delete_arp_spoofing_protection(vifs):
    current_rules = c_get_ebtables_list()
    _delete_arp_spoofing_protection(vifs, current_rules, table='nat',
                                    chain='PREROUTING')


def _delete_arp_spoofing_protection(vifs, current_rules, table, chain):
    jumps = [vif for vif in vifs if vif_jump_present(vif, current_rules)]
    for vif in jumps:
        c_delete_ebtables(chain, vif, chain_name(vif))
    for vif in vifs:
        chain_delete(chain_name(vif), table, current_rules)
    _delete_mac_spoofing_protection(vifs, current_rules, table=table,
                                    chain=chain)


@lockutils.synchronized('ebtables')
def delete_unreferenced_arp_protection(current_vifs):
    current_rules = c_get_ebtables_list()
    to_delete = []
    for line in current_rules:
        if line.startswith('Bridge chain: %s' % SPOOF_CHAIN_PREFIX):
            devname = line.split(SPOOF_CHAIN_PREFIX, 1)[1].split(',')[0]
            if devname not in current_vifs:
                to_delete.append(devname)
    LOG.info("Clearing orphaned ARP spoofing entries for devices %s",
             to_delete)
    _delete_arp_spoofing_protection(to_delete, current_rules, table='nat',
                                    chain='PREROUTING')


@lockutils.synchronized('ebtables')
def install_arp_spoofing_protection(vif, addresses):
    current_rules = c_get_ebtables_list()
    _install_arp_spoofing_protection(vif, addresses, current_rules)


def _install_arp_spoofing_protection(vif, addresses, current_rules):
    vif_chain = chain_name(vif)
    if not chain_exists(vif_chain, current_rules):
        c_create_ebtables_chain(vif_chain)
    else:
        c_flush_ebtables_chain(vif_chain)
    for addr in sorted(addresses):
        c_add_ebtables_accept_rule(vif_chain, addr)
    if not vif_jump_present(vif, current_rules):
        c_add_ebtables_jump_rule(vif, vif_chain)


def chain_exists(chain, current_rules):
    for rule in current_rules:
        if rule.startswith('Bridge chain: %s' % chain):
            return True
    return False


def chain_delete(chain, table, current_rules):
    if chain_exists(chain, current_rules):
        c_flush_and_delete_ebtables_chain(chain, table)


def vif_jump_present(vif, current_rules):
    searches = (('-i %s' % vif), ('-j %s' % chain_name(vif)), ('-p ARP'))
    for line in current_rules:
        if all(s in line for s in searches):
            return True
    return False


def _install_mac_spoofing_protection(vif, port_details, current_rules):
    mac_addresses = {port_details['mac_address']}
    if port_details.get('allowed_address_pairs'):
        mac_addresses |= {p['mac_address']
                          for p in port_details['allowed_address_pairs']}
    mac_addresses = list(mac_addresses)
    vif_chain = _mac_chain_name(vif)
    if not chain_exists(vif_chain, current_rules):
        c_create_ebtables_chain(vif_chain)

    if not _mac_vif_jump_present(vif, current_rules):
        c_add_ebtables_jump_rule(vif, vif_chain)

    _delete_vif_mac_rules(vif, current_rules)
    for chunk in (mac_addresses[i:i + 500]
                  for i in range(0, len(mac_addresses), 500)):
        c_add_ebtables_mac_rules(vif_chain, vif, chunk)


def _mac_vif_jump_present(vif, current_rules):
    searches = (('-i %s' % vif), ('-j %s' % _mac_chain_name(vif)))
    for line in current_rules:
        if all(s in line for s in searches):
            return True
    return False


def _mac_chain_name(vif):
    return '%s%s' % (MAC_CHAIN_PREFIX, vif)


def _delete_vif_mac_rules(vif, current_rules):
    chain = _mac_chain_name(vif)
    for rule in current_rules:
        if '-i %s' % vif in rule and '--among-src' in rule:
            c_delete_ebtables_chain_rule(chain, rule)


def _delete_mac_spoofing_protection(vifs, current_rules, table, chain):
    jumps = [vif for vif in vifs
             if _mac_vif_jump_present(vif, current_rules)]
    for vif in jumps:
        c_delete_ebtables_jump_rule(chain, vif, _mac_chain_name(vif))
    for vif in vifs:
        chain_delete(_mac_chain_name(vif), table, current_rules)


NAMESPACE = None


@tenacity.retry(
    wait=tenacity.wait_exponential(multiplier=0.02),
    retry=tenacity.retry_if_exception(lambda e: e.returncode in [255, 4]),
    reraise=True
)
def ebtables(comm, table='nat'):
    execute = ip_lib.IPWrapper(NAMESPACE).netns.execute
    return execute(['ebtables', '-t', table, '--concurrent'] + comm,
                   run_as_root=True)

def c_get_ebtables_list():
    c_lib.get_ebtables_list.restype = ctypes.c_char_p
    return c_lib.get_ebtables_list().decode('utf-8').splitlines()

def c_delete_ebtables(chain, vif, chain_name):
    c_lib.delete_ebtables(ctypes.c_char_p(chain.encode('utf-8')),
                          ctypes.c_char_p(vif.encode('utf-8')),
                          ctypes.c_char_p(chain_name.encode('utf-8')))

def c_create_ebtables_chain(vif_chain):
    c_lib.create_ebtables_chain(ctypes.c_char_p(vif_chain.encode('utf-8')))

def c_flush_ebtables_chain(vif_chain):
    c_lib.flush_ebtables_chain(ctypes.c_char_p(vif_chain.encode('utf-8')))

def c_add_ebtables_accept_rule(vif_chain, addr):
    c_lib.add_ebtables_accept_rule(ctypes.c_char_p(vif_chain.encode('utf-8')),
                                   ctypes.c_char_p(addr.encode('utf-8')))

def c_add_ebtables_jump_rule(vif, vif_chain):
    c_lib.add_ebtables_jump_rule(ctypes.c_char_p(vif.encode('utf-8')),
                                 ctypes.c_char_p(vif_chain.encode('utf-8')))

def c_flush_and_delete_ebtables_chain(chain, table):
    c_lib.flush_and_delete_ebtables_chain(ctypes.c_char_p(chain.encode('utf-8')),
                                          ctypes.c_char_p(table.encode('utf-8')))

def c_add_ebtables_mac_rules(vif_chain, vif, chunk):
    chunk_str = ','.join(chunk)
    c_lib.add_ebtables_mac_rules(ctypes.c_char_p(vif_chain.encode('utf-8')),
                                 ctypes.c_char_p(vif.encode('utf-8')),
                                 ctypes.c_char_p(chunk_str.encode('utf-8')))

def c_delete_ebtables_chain_rule(chain, rule):
    c_lib.delete_ebtables_chain_rule(ctypes.c_char_p(chain.encode('utf-8')),
                                     ctypes.c_char_p(rule.encode('utf-8')))

def c_delete_ebtables_jump_rule(chain, vif, mac_chain_name):
    c_lib.delete_ebtables_jump_rule(ctypes.c_char_p(chain.encode('utf-8')),
                                    ctypes.c_char_p(vif.encode('utf-8')),
                                    ctypes.c_char_p(mac_chain_name.encode('utf-8')))