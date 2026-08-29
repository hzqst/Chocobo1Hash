#include "../src/crc_64_xz.h"

#include "catch2/single_include/catch2/catch.hpp"

#include <array>
#include <cstring>
#include <functional>


TEST_CASE("crc-64-xz")  // NOLINT
{
	using Hash = Chocobo1::CRC_64_XZ;

	REQUIRE(Hash() == Hash());
	REQUIRE(Hash().addData("123").finalize() != Hash().finalize());

	REQUIRE("0000000000000000" == Hash().finalize().toString());

	const char check[] = "123456789";
	REQUIRE("995dc9bbdf1939fa" == Hash().addData(check, strlen(check)).finalize().toString());

	const char sentence[] = "The quick brown fox jumps over the lazy dog";
	REQUIRE("5b5eb8c2e54aa1c4" == Hash().addData(sentence, strlen(sentence)).finalize().toString());

	Hash chunked;
	chunked.addData(sentence, 10);
	chunked.addData(sentence + 10, strlen(sentence) - 10);
	REQUIRE("5b5eb8c2e54aa1c4" == chunked.finalize().toString());

	const std::array<unsigned char, 8> expected = {0x99, 0x5d, 0xc9, 0xbb, 0xdf, 0x19, 0x39, 0xfa};
	const auto digest = Hash().addData(check, strlen(check)).finalize();
	REQUIRE(expected == digest.toArray());
	REQUIRE(UINT64_C(0x995dc9bbdf1939fa) == static_cast<uint64_t>(digest));

	Hash resetHash;
	resetHash.addData(check, strlen(check)).finalize();
	resetHash.reset();
	REQUIRE(Hash().finalize() == resetHash.finalize());

	REQUIRE(0 == std::hash<Hash> {}(Hash().finalize()));
}
