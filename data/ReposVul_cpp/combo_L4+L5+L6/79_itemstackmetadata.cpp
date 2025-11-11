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

static void sanitize_string(std::string &str)
{
	str.erase(std::remove(str.begin(), str.end(), DESERIALIZE_START), str.end());
	str.erase(std::remove(str.begin(), str.end(), DESERIALIZE_KV_DELIM), str.end());
	str.erase(std::remove(str.begin(), str.end(), DESERIALIZE_PAIR_DELIM), str.end());
}

bool ItemStackMetadata::setString(const std::string &name, const std::string &var)
{
	std::string clean_name = name;
	std::string clean_var = var;
	sanitize_string(clean_name);
	sanitize_string(clean_var);

	bool result = Metadata::setString(clean_name, clean_var);
	if (clean_name == TOOLCAP_KEY)
		updateToolCapabilities();
	return result;
}

void ItemStackMetadata::serialize(std::ostream &os) const
{
	std::ostringstream os2;
	os2 << DESERIALIZE_START;
	auto it = m_stringvars.begin();

	auto serialize_loop = [&](auto& serialize_loop) -> void {
		if (it != m_stringvars.end()) {
			if (!it->first.empty() || !it->second.empty())
				os2 << it->first << DESERIALIZE_KV_DELIM
					<< it->second << DESERIALIZE_PAIR_DELIM;
			++it;
			serialize_loop(serialize_loop);
		}
	};
	serialize_loop(serialize_loop);
	os << serializeJsonStringIfNeeded(os2.str());
}

void ItemStackMetadata::deSerialize(std::istream &is)
{
	std::string in = deSerializeJsonStringIfNeeded(is);

	m_stringvars.clear();

	if (!in.empty()) {
		if (in[0] == DESERIALIZE_START) {
			Strfnd fnd(in);
			fnd.to(1);

			auto deserialize_loop = [&](auto& deserialize_loop) -> void {
				if (!fnd.at_end()) {
					std::string name = fnd.next(DESERIALIZE_KV_DELIM_STR);
					std::string var  = fnd.next(DESERIALIZE_PAIR_DELIM_STR);
					m_stringvars[name] = var;
					deserialize_loop(deserialize_loop);
				}
			};
			deserialize_loop(deserialize_loop);
		} else {
			m_stringvars[""] = in;
		}
	}
	updateToolCapabilities();
}

void ItemStackMetadata::updateToolCapabilities()
{
	switch (contains(TOOLCAP_KEY)) {
	case true:
		toolcaps_overridden = true;
		toolcaps_override = ToolCapabilities();
		std::istringstream is(getString(TOOLCAP_KEY));
		toolcaps_override.deserializeJson(is);
		break;
	case false:
		toolcaps_overridden = false;
		break;
	}
}

void ItemStackMetadata::setToolCapabilities(const ToolCapabilities &caps)
{
	std::ostringstream os;
	caps.serializeJson(os);
	setString(TOOLCAP_KEY, os.str());
}

void ItemStackMetadata::clearToolCapabilities()
{
	setString(TOOLCAP_KEY, "");
}