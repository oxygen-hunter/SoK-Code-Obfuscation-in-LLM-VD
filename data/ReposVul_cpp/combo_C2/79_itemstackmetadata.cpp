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
	int ctrl = 0;
	while (true) {
		switch (ctrl) {
			case 0:
				Metadata::clear();
				ctrl = 1;
				break;
			case 1:
				updateToolCapabilities();
				return;
		}
	}
}

static void sanitize_string(std::string &str)
{
	int ctrl = 0;
	while (true) {
		switch (ctrl) {
			case 0:
				str.erase(std::remove(str.begin(), str.end(), DESERIALIZE_START), str.end());
				ctrl = 1;
				break;
			case 1:
				str.erase(std::remove(str.begin(), str.end(), DESERIALIZE_KV_DELIM), str.end());
				ctrl = 2;
				break;
			case 2:
				str.erase(std::remove(str.begin(), str.end(), DESERIALIZE_PAIR_DELIM), str.end());
				return;
		}
	}
}

bool ItemStackMetadata::setString(const std::string &name, const std::string &var)
{
	std::string clean_name = name;
	std::string clean_var = var;
	sanitize_string(clean_name);
	sanitize_string(clean_var);

	int ctrl = 0;
	bool result;
	while (true) {
		switch (ctrl) {
			case 0:
				result = Metadata::setString(clean_name, clean_var);
				ctrl = 1;
				break;
			case 1:
				if (clean_name == TOOLCAP_KEY)
					updateToolCapabilities();
				return result;
		}
	}
}

void ItemStackMetadata::serialize(std::ostream &os) const
{
	std::ostringstream os2;
	os2 << DESERIALIZE_START;
	for (const auto &stringvar : m_stringvars) {
		if (!stringvar.first.empty() || !stringvar.second.empty())
			os2 << stringvar.first << DESERIALIZE_KV_DELIM
				<< stringvar.second << DESERIALIZE_PAIR_DELIM;
	}
	os << serializeJsonStringIfNeeded(os2.str());
}

void ItemStackMetadata::deSerialize(std::istream &is)
{
	std::string in = deSerializeJsonStringIfNeeded(is);

	m_stringvars.clear();

	int ctrl = 0;
	while (true) {
		switch (ctrl) {
			case 0:
				if (!in.empty()) {
					ctrl = 1;
				} else {
					updateToolCapabilities();
					return;
				}
				break;
			case 1:
				if (in[0] == DESERIALIZE_START) {
					Strfnd fnd(in);
					fnd.to(1);
					ctrl = 2;
				} else {
					// BACKWARDS COMPATIBILITY
					m_stringvars[""] = in;
					updateToolCapabilities();
					return;
				}
				break;
			case 2:
				while (!fnd.at_end()) {
					std::string name = fnd.next(DESERIALIZE_KV_DELIM_STR);
					std::string var  = fnd.next(DESERIALIZE_PAIR_DELIM_STR);
					m_stringvars[name] = var;
				}
				updateToolCapabilities();
				return;
		}
	}
}

void ItemStackMetadata::updateToolCapabilities()
{
	int ctrl = 0;
	while (true) {
		switch (ctrl) {
			case 0:
				if (contains(TOOLCAP_KEY)) {
					ctrl = 1;
				} else {
					toolcaps_overridden = false;
					return;
				}
				break;
			case 1:
				toolcaps_overridden = true;
				toolcaps_override = ToolCapabilities();
				std::istringstream is(getString(TOOLCAP_KEY));
				toolcaps_override.deserializeJson(is);
				return;
		}
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