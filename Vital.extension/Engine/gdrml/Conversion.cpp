#include "Conversion.h"

#include <cstring>

using namespace godot;

PackedByteArray Conversion::bufferToByteArray(const std::byte* data, size_t size) {
	PackedByteArray result;
	result.resize(static_cast<int64_t>(size));

	if (size > 0) {
		std::memcpy(result.ptrw(), data, size);
	}

	return result;
}

std::vector<std::byte> Conversion::byteArrayToBuffer(const PackedByteArray& byteArray) {
	const auto* ptr =
		reinterpret_cast<const std::byte*>(byteArray.ptr());

	return std::vector<std::byte>(
		ptr,
		ptr + static_cast<size_t>(byteArray.size())
	);
}

String Conversion::stringToGodot(std::string_view string) {
	return String(string.data());
}

std::string Conversion::stringToStd(const String& string) {
	return std::string(string.utf8().get_data());
}
