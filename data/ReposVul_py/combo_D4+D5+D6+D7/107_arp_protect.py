import netaddr
from neutron_lib.utils import net
from oslo_concurrency import lockutils
from oslo_log import log as logging
import tenacity

from neutron.agent.linux import ip_lib

LOG = logging.getLogger(__name__)
PREFIXES = ['neutronARP-', 'neutronMAC-']

def setup_arp_spoofing_protection(vif, port_data):
    if not port_data.get('port_security_enabled', True):
        delete_arp_spoofing_protection([vif])
        LOG.info("Skipping ARP spoofing rules for port '%s' because "
                 "it has port security disabled", vif)
        return
    if net.is_port_trusted(port_data):
        delete_arp_spoofing_protection([vif])
        LOG.debug("Skipping ARP spoofing rules for network owned port "
                  "'%s'.", vif)
        return
    _setup_arp_spoofing_protection(vif, port_data)


@lockutils.synchronized('ebtables')
def _setup_arp_spoofing_protection(vif, port_data):
    current_rules = ebtables(['-L']).splitlines()
    _install_mac_spoofing_protection(vif, port_data, current_rules)
    ips = {f['ip_address'] for f in port_data['fixed_ips']}
    if port_data.get('allowed_address_pairs'):
        ips |= {p['ip_address']
                      for p in port_data['allowed_address_pairs']}

    ips = {ip for ip in ips
                 if netaddr.IPNetwork(ip).version == 4}
    if any(netaddr.IPNetwork(ip).prefixlen == 0 for ip in ips):
        return
    _install_arp_spoofing_protection(vif, ips, current_rules)


def chain_name(vif):
    return '%s%s' % (PREFIXES[0], vif)


@lockutils.synchronized('ebtables')
def delete_arp_spoofing_protection(vifs):
    current_rules = ebtables(['-L']).splitlines()
    _delete_arp_spoofing_protection(vifs, current_rules, 'nat', 'PREROUTING')


def _delete_arp_spoofing_protection(vifs, rules, table, chain):
    jumps = [vif for vif in vifs if vif_jump_present(vif, rules)]
    for vif in jumps:
        ebtables(['-D', chain, '-i', vif, '-j',
                  chain_name(vif), '-p', 'ARP'], table=table)
    for vif in vifs:
        chain_delete(chain_name(vif), table, rules)
    _delete_mac_spoofing_protection(vifs, rules, table, chain)


@lockutils.synchronized('ebtables')
def delete_unreferenced_arp_protection(current_vifs):
    current_rules = ebtables(['-L'], table='nat').splitlines()
    to_delete = []
    for line in current_rules:
        if line.startswith('Bridge chain: %s' % PREFIXES[0]):
            devname = line.split(PREFIXES[0], 1)[1].split(',')[0]
            if devname not in current_vifs:
                to_delete.append(devname)
    LOG.info("Clearing orphaned ARP spoofing entries for devices %s",
             to_delete)
    _delete_arp_spoofing_protection(to_delete, current_rules, 'nat',
                                    'PREROUTING')


@lockutils.synchronized('ebtables')
def install_arp_spoofing_protection(vif, addresses):
    current_rules = ebtables(['-L']).splitlines()
    _install_arp_spoofing_protection(vif, addresses, current_rules)


def _install_arp_spoofing_protection(vif, addresses, rules):
    vif_chain = chain_name(vif)
    if not chain_exists(vif_chain, rules):
        ebtables(['-N', vif_chain, '-P', 'DROP'])
        ebtables(['-A', vif_chain, '-j', 'DROP'])
    else:
        ebtables(['-F', vif_chain])
        ebtables(['-A', vif_chain, '-j', 'DROP'])
    for addr in sorted(addresses):
        ebtables(['-I', vif_chain, '-p', 'ARP', '--arp-ip-src', addr,
                  '-j', 'ACCEPT'])
    if not vif_jump_present(vif, rules):
        ebtables(['-A', 'PREROUTING', '-i', vif, '-j',
                  vif_chain, '-p', 'ARP'])


def chain_exists(chain, rules):
    for rule in rules:
        if rule.startswith('Bridge chain: %s' % chain):
            return True
    return False


def chain_delete(chain, table, rules):
    if chain_exists(chain, rules):
        ebtables(['-F', chain], table=table)
        ebtables(['-X', chain], table=table)


def vif_jump_present(vif, rules):
    searches = (('-i %s' % vif), ('-j %s' % chain_name(vif)), ('-p ARP'))
    for line in rules:
        if all(s in line for s in searches):
            return True
    return False


def _install_mac_spoofing_protection(vif, port_data, rules):
    macs = {port_data['mac_address']}
    if port_data.get('allowed_address_pairs'):
        macs |= {p['mac_address']
                          for p in port_data['allowed_address_pairs']}
    macs = list(macs)
    vif_chain = _mac_chain_name(vif)
    if not chain_exists(vif_chain, rules):
        ebtables(['-N', vif_chain, '-P', 'DROP'])
        ebtables(['-A', vif_chain, '-j', 'DROP'])

    if not _mac_vif_jump_present(vif, rules):
        ebtables(['-I', 'PREROUTING', '-i', vif, '-j', vif_chain])

    _delete_vif_mac_rules(vif, rules)
    for chunk in (macs[i:i + 500]
                  for i in range(0, len(macs), 500)):
        new_rule = ['-I', vif_chain, '-i', vif,
                    '--among-src', ','.join(sorted(chunk)), '-j', 'RETURN']
        ebtables(new_rule)


def _mac_vif_jump_present(vif, rules):
    searches = (('-i %s' % vif), ('-j %s' % _mac_chain_name(vif)))
    for line in rules:
        if all(s in line for s in searches):
            return True
    return False


def _mac_chain_name(vif):
    return '%s%s' % (PREFIXES[1], vif)


def _delete_vif_mac_rules(vif, rules):
    chain = _mac_chain_name(vif)
    for rule in rules:
        if '-i %s' % vif in rule and '--among-src' in rule:
            ebtables(['-D', chain] + rule.split())


def _delete_mac_spoofing_protection(vifs, rules, table, chain):
    jumps = [vif for vif in vifs
             if _mac_vif_jump_present(vif, rules)]
    for vif in jumps:
        ebtables(['-D', chain, '-i', vif, '-j',
                  _mac_chain_name(vif)], table=table)
    for vif in vifs:
        chain_delete(_mac_chain_name(vif), table, rules)


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