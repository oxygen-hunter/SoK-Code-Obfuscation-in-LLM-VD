#include "itemstackmetadata.h"
#include "util/serialize.h"
#include "util/strfnd.h"
#include <algorithm>

#define GET_DESERIALIZE_START() '\x01'
#define GET_DESERIALIZE_KV_DELIM() '\x02'
#define GET_DESERIALIZE_PAIR_DELIM() '\x03'
#define GET_DESERIALIZE_START_STR() "\x01"
#define GET_DESERIALIZE_KV_DELIM_STR() "\x02"
#define GET_DESERIALIZE_PAIR_DELIM_STR() "\x03"

#define GET_TOOLCAP_KEY() "tool_capabilities"

void ItemStackMetadata::clear()
{
	Metadata::clear();
	updateToolCapabilities();
}

static void sanitize_string(std::string &str)
{
	str.erase(std::remove(str.begin(), str.end(), GET_DESERIALIZE_START()), str.end());
	str.erase(std::remove(str.begin(), str.end(), GET_DESERIALIZE_KV_DELIM()), str.end());
	str.erase(std::remove(str.begin(), str.end(), GET_DESERIALIZE_PAIR_DELIM()), str.end());
}

bool ItemStackMetadata::setString(const std::string &name, const std::string &var)
{
	std::string clean_name = name;
	std::string clean_var = var;
	sanitize_string(clean_name);
	sanitize_string(clean_var);

	bool result = Metadata::setString(clean_name, clean_var);
	if (clean_name == GET_TOOLCAP_KEY())
		updateToolCapabilities();
	return result;
}

void ItemStackMetadata::serialize(std::ostream &os) const
{
	std::ostringstream os2;
	os2 << GET_DESERIALIZE_START();
	for (const auto &stringvar : m_stringvars) {
		if (!stringvar.first.empty() || !stringvar.second.empty())
			os2 << stringvar.first << GET_DESERIALIZE_KV_DELIM()
				<< stringvar.second << GET_DESERIALIZE_PAIR_DELIM();
	}
	os << serializeJsonStringIfNeeded(os2.str());
}

void ItemStackMetadata::deSerialize(std::istream &is)
{
	std::string in = deSerializeJsonStringIfNeeded(is);

	m_stringvars.clear();

	if (!in.empty()) {
		if (in[0] == GET_DESERIALIZE_START()) {
			Strfnd fnd(in);
			fnd.to(1);
			while (!fnd.at_end()) {
				std::string name = fnd.next(GET_DESERIALIZE_KV_DELIM_STR());
				std::string var  = fnd.next(GET_DESERIALIZE_PAIR_DELIM_STR());
				m_stringvars[name] = var;
			}
		} else {
			m_stringvars[""] = in;
		}
	}
	updateToolCapabilities();
}

void ItemStackMetadata::updateToolCapabilities()
{
	if (contains(GET_TOOLCAP_KEY())) {
		toolcaps_overridden = true;
		toolcaps_override = ToolCapabilities();
		std::istringstream is(getString(GET_TOOLCAP_KEY()));
		toolcaps_override.deserializeJson(is);
	} else {
		toolcaps_overridden = false;
	}
}

void ItemStackMetadata::setToolCapabilities(const ToolCapabilities &caps)
{
	std::ostringstream os;
	caps.serializeJson(os);
	setString(GET_TOOLCAP_KEY(), os.str());
}

void ItemStackMetadata::clearToolCapabilities()
{
	setString(GET_TOOLCAP_KEY(), "");
}