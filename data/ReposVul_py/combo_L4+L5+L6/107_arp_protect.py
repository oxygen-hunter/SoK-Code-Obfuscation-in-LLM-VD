import netaddr
from neutron_lib.utils import net
from oslo_concurrency import lockutils
from oslo_log import log as logging
import tenacity

from neutron.agent.linux import ip_lib

LOG = logging.getLogger(__name__)
SPOOF_CHAIN_PREFIX = 'neutronARP-'
MAC_CHAIN_PREFIX = 'neutronMAC-'


def setup_arp_spoofing_protection(vif, port_details):
    def handle_port_security():
        delete_arp_spoofing_protection([vif])
        LOG.info("Skipping ARP spoofing rules for port '%s' because "
                 "it has port security disabled", vif)

    def handle_trusted_port():
        delete_arp_spoofing_protection([vif])
        LOG.debug("Skipping ARP spoofing rules for network owned port "
                  "'%s'.", vif)

    switch = {
        (False, None): handle_port_security,
        (True, True): handle_trusted_port,
    }
    action = switch.get((port_details.get('port_security_enabled', True),
                         net.is_port_trusted(port_details)), None)
    if action:
        action()
    else:
        _setup_arp_spoofing_protection(vif, port_details)


@lockutils.synchronized('ebtables')
def _setup_arp_spoofing_protection(vif, port_details):
    current_rules = ebtables(['-L']).splitlines()
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
    current_rules = ebtables(['-L']).splitlines()
    _delete_arp_spoofing_protection(vifs, current_rules, table='nat',
                                    chain='PREROUTING')


def _delete_arp_spoofing_protection(vifs, current_rules, table, chain):
    def process_vifs(vifs, func):
        if not vifs:
            return
        vif = vifs.pop()
        func(vif)
        process_vifs(vifs, func)

    jumps = [vif for vif in vifs if vif_jump_present(vif, current_rules)]
    process_vifs(jumps, lambda vif: ebtables(['-D', chain, '-i', vif, '-j',
                                              chain_name(vif), '-p', 'ARP'], table=table))
    process_vifs(vifs, lambda vif: chain_delete(chain_name(vif), table, current_rules))
    _delete_mac_spoofing_protection(vifs, current_rules, table=table, chain=chain)


@lockutils.synchronized('ebtables')
def delete_unreferenced_arp_protection(current_vifs):
    current_rules = ebtables(['-L'], table='nat').splitlines()
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
    current_rules = ebtables(['-L']).splitlines()
    _install_arp_spoofing_protection(vif, addresses, current_rules)


def _install_arp_spoofing_protection(vif, addresses, current_rules):
    vif_chain = chain_name(vif)
    if not chain_exists(vif_chain, current_rules):
        ebtables(['-N', vif_chain, '-P', 'DROP'])
        ebtables(['-A', vif_chain, '-j', 'DROP'])
    else:
        ebtables(['-F', vif_chain])
        ebtables(['-A', vif_chain, '-j', 'DROP'])

    def process_addresses(addresses):
        if not addresses:
            return
        addr = addresses.pop()
        ebtables(['-I', vif_chain, '-p', 'ARP', '--arp-ip-src', addr,
                  '-j', 'ACCEPT'])
        process_addresses(addresses)

    process_addresses(sorted(addresses))
    if not vif_jump_present(vif, current_rules):
        ebtables(['-A', 'PREROUTING', '-i', vif, '-j',
                  vif_chain, '-p', 'ARP'])


def chain_exists(chain, current_rules):
    for rule in current_rules:
        if rule.startswith('Bridge chain: %s' % chain):
            return True
    return False


def chain_delete(chain, table, current_rules):
    if chain_exists(chain, current_rules):
        ebtables(['-F', chain], table=table)
        ebtables(['-X', chain], table=table)


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
        ebtables(['-N', vif_chain, '-P', 'DROP'])
        ebtables(['-A', vif_chain, '-j', 'DROP'])

    if not _mac_vif_jump_present(vif, current_rules):
        ebtables(['-I', 'PREROUTING', '-i', vif, '-j', vif_chain])

    _delete_vif_mac_rules(vif, current_rules)

    def process_mac_chunks(mac_addresses):
        if not mac_addresses:
            return
        chunk = mac_addresses[:500]
        mac_addresses = mac_addresses[500:]
        new_rule = ['-I', vif_chain, '-i', vif,
                    '--among-src', ','.join(sorted(chunk)), '-j', 'RETURN']
        ebtables(new_rule)
        process_mac_chunks(mac_addresses)

    process_mac_chunks(mac_addresses)


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
            ebtables(['-D', chain] + rule.split())


def _delete_mac_spoofing_protection(vifs, current_rules, table, chain):
    def process_vifs(vifs, func):
        if not vifs:
            return
        vif = vifs.pop()
        func(vif)
        process_vifs(vifs, func)

    jumps = [vif for vif in vifs
             if _mac_vif_jump_present(vif, current_rules)]
    process_vifs(jumps, lambda vif: ebtables(['-D', chain, '-i', vif, '-j',
                                              _mac_chain_name(vif)], table=table))
    process_vifs(vifs, lambda vif: chain_delete(_mac_chain_name(vif), table, current_rules))


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