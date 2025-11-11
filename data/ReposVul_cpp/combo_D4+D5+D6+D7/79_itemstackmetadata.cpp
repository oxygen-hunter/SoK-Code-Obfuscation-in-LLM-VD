#include "itemstackmetadata.h"
#include "util/serialize.h"
#include "util/strfnd.h"
#include <algorithm>

#define DESERIALIZE_START '\x01'
#define DESERIALIZE_KV_DELIM '\x02'
#define DESERIALIZE_PAIR_DELIM '\x03'
#define DESERIALIZE_START_STR "\x01"
#define DESERIALIZE_KV_DELIM_STR "\x02"
#define DESERIALIZE_PAIR_DELIM_STR "\x03"

#define TOOLCAP_KEY "tool_capabilities"

void ItemStackMetadata::clear()
{
	Metadata::clear();
	updateToolCapabilities();
}

static void sanitize_string(std::string &G)
{
	G.erase(std::remove(G.begin(), G.end(), DESERIALIZE_START), G.end());
	G.erase(std::remove(G.begin(), G.end(), DESERIALIZE_KV_DELIM), G.end());
	G.erase(std::remove(G.begin(), G.end(), DESERIALIZE_PAIR_DELIM), G.end());
}

bool ItemStackMetadata::setString(const std::string &J, const std::string &K)
{
	std::string z = J;
	std::string y = K;
	sanitize_string(z);
	sanitize_string(y);

	bool T = Metadata::setString(z, y);
	if (z == TOOLCAP_KEY)
		updateToolCapabilities();
	return T;
}

void ItemStackMetadata::serialize(std::ostream &M) const
{
	std::ostringstream N;
	N << DESERIALIZE_START;
	for (const auto &P : m_stringvars) {
		if (!P.first.empty() || !P.second.empty())
			N << P.first << DESERIALIZE_KV_DELIM
				<< P.second << DESERIALIZE_PAIR_DELIM;
	}
	M << serializeJsonStringIfNeeded(N.str());
}

void ItemStackMetadata::deSerialize(std::istream &U)
{
	std::string V = deSerializeJsonStringIfNeeded(U);

	m_stringvars.clear();

	if (!V.empty()) {
		if (V[0] == DESERIALIZE_START) {
			Strfnd fnd(V);
			fnd.to(1);
			while (!fnd.at_end()) {
				std::string A = fnd.next(DESERIALIZE_KV_DELIM_STR);
				std::string B  = fnd.next(DESERIALIZE_PAIR_DELIM_STR);
				m_stringvars[A] = B;
			}
		} else {
			m_stringvars[""] = V;
		}
	}
	updateToolCapabilities();
}

bool toolcaps_overridden;
ToolCapabilities toolcaps_override;

void ItemStackMetadata::updateToolCapabilities()
{
	if (contains(TOOLCAP_KEY)) {
		toolcaps_overridden = true;
		toolcaps_override = ToolCapabilities();
		std::istringstream C(getString(TOOLCAP_KEY));
		toolcaps_override.deserializeJson(C);
	} else {
		toolcaps_overridden = false;
	}
}

void ItemStackMetadata::setToolCapabilities(const ToolCapabilities &D)
{
	std::ostringstream E;
	D.serializeJson(E);
	setString(TOOLCAP_KEY, E.str());
}

void ItemStackMetadata::clearToolCapabilities()
{
	setString(TOOLCAP_KEY, "");
}