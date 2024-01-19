#include "Json.hpp"

#include <mod/Post.hpp>
#include <mod/lib/IO/IO.hpp>

#include <boost/crc.hpp>

namespace mod::lib::IO {

void writeJsonFile(const std::string &name, const nlohmann::json &j) {
	const std::vector<std::uint8_t> bytes = nlohmann::json::to_cbor(j);
	boost::crc_32_type result;
	result.process_bytes(bytes.data(), bytes.size());
	const std::uint32_t checksum = result.checksum();
	const char version = 1;

	post::FileHandle s(name);
	s.stream.write(reinterpret_cast<const char *>(bytes.data()), bytes.size());
	s.stream.write(reinterpret_cast<const char *>(&checksum), sizeof(checksum));
	s.stream.write(&version, 1);
}

std::optional<nlohmann::json> readJson(const std::vector<std::uint8_t> &data, std::ostream &err) {
	if(data.size() < sizeof(std::uint32_t) + 1) {
		err << "Integrity check failed. Input too short.";
		return {};
	}
	const auto version = data.back();
	if(version != 1) {
		err << "Integrity check failed. Unknown file version, " + std::to_string(version) + ".";
		return {};
	}
	const auto dataSize = data.size() - 1 - sizeof(std::uint32_t);
	const std::uint32_t checksum = *reinterpret_cast<const std::uint32_t *>(data.data() + dataSize);
	boost::crc_32_type result;
	result.process_bytes(data.data(), dataSize);
	if(checksum != result.checksum()) {
		err << "Integrity check failed. Wrong checksum, " << checksum << ". Actual checksum is "
		    << result.checksum() << ".";
		return {};
	}
	try {
		return nlohmann::json::from_cbor(data.data(), data.data() + dataSize);
	} catch(const std::exception &e) {
		err << e.what();
		return {};
	}
}

bool validateJson(const nlohmann::json &j,
                  const nlohmann::json_schema::json_validator &validator,
                  std::ostream &err,
                  const std::string &msg) {
	struct ErrorHandler : nlohmann::json_schema::basic_error_handler {
		ErrorHandler(std::ostream &err, const std::string &msg) : err(err), msg(msg) {}

		void error(const nlohmann::json::json_pointer &pointer,
		           const nlohmann::json &instance,
		           const std::string &message) override {
			nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
			err << msg << "\n\t" << message << " at " << pointer << " in\n\t" << instance << "\n";
		}
	public:
		std::ostream &err;
		const std::string &msg;
	};
	ErrorHandler handler(err, msg);
	validator.validate(j, handler);
	return !handler;
}

} // namespace mod::lib::IO