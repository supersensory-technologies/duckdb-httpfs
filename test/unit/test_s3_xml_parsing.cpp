#include "catch.hpp"
#include "s3fs.hpp"

using namespace duckdb;

// ---------------------------------------------------------------------------
// ParseCommonPrefix
// ---------------------------------------------------------------------------

TEST_CASE("ParseCommonPrefix - standard AWS XML (no xmlns on inner elements)", "[s3][xml]") {
	string response = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <Name>my-bucket</Name>
  <Prefix>data/</Prefix>
  <CommonPrefixes>
    <Prefix>data/2024/</Prefix>
  </CommonPrefixes>
</ListBucketResult>)";

	auto result = AWSListObjectV2::ParseCommonPrefix(response);
	REQUIRE(result.size() == 1);
	REQUIRE(result[0] == "data/2024/");
}

TEST_CASE("ParseCommonPrefix - Tigris-style XML with xmlns on inner Prefix", "[s3][xml][tigris]") {
	string response = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <Name>my-bucket</Name>
  <Prefix>data/</Prefix>
  <CommonPrefixes>
    <Prefix xmlns="http://s3.amazonaws.com/doc/2006-03-01/">data/2024/</Prefix>
  </CommonPrefixes>
</ListBucketResult>)";

	auto result = AWSListObjectV2::ParseCommonPrefix(response);
	REQUIRE(result.size() == 1);
	REQUIRE(result[0] == "data/2024/");
}

TEST_CASE("ParseCommonPrefix - multiple CommonPrefixes entries", "[s3][xml]") {
	string response = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <Name>my-bucket</Name>
  <Prefix>data/</Prefix>
  <CommonPrefixes>
    <Prefix>data/2023/</Prefix>
  </CommonPrefixes>
  <CommonPrefixes>
    <Prefix>data/2024/</Prefix>
  </CommonPrefixes>
  <CommonPrefixes>
    <Prefix>data/2025/</Prefix>
  </CommonPrefixes>
</ListBucketResult>)";

	auto result = AWSListObjectV2::ParseCommonPrefix(response);
	REQUIRE(result.size() == 3);
	REQUIRE(result[0] == "data/2023/");
	REQUIRE(result[1] == "data/2024/");
	REQUIRE(result[2] == "data/2025/");
}

TEST_CASE("ParseCommonPrefix - multiple entries with xmlns on both CommonPrefixes and Prefix", "[s3][xml][tigris]") {
	string response = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <Name>my-bucket</Name>
  <CommonPrefixes xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
    <Prefix xmlns="http://s3.amazonaws.com/doc/2006-03-01/">alpha/</Prefix>
  </CommonPrefixes>
  <CommonPrefixes xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
    <Prefix xmlns="http://s3.amazonaws.com/doc/2006-03-01/">beta/</Prefix>
  </CommonPrefixes>
</ListBucketResult>)";

	auto result = AWSListObjectV2::ParseCommonPrefix(response);
	REQUIRE(result.size() == 2);
	REQUIRE(result[0] == "alpha/");
	REQUIRE(result[1] == "beta/");
}

TEST_CASE("ParseCommonPrefix - empty response (no CommonPrefixes)", "[s3][xml]") {
	string response = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <Name>my-bucket</Name>
  <Prefix>data/</Prefix>
  <IsTruncated>false</IsTruncated>
</ListBucketResult>)";

	auto result = AWSListObjectV2::ParseCommonPrefix(response);
	REQUIRE(result.empty());
}

// ---------------------------------------------------------------------------
// ParseContinuationToken
// ---------------------------------------------------------------------------

TEST_CASE("ParseContinuationToken - standard AWS XML", "[s3][xml]") {
	string response = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <Name>my-bucket</Name>
  <IsTruncated>true</IsTruncated>
  <NextContinuationToken>abc123token</NextContinuationToken>
</ListBucketResult>)";

	auto result = AWSListObjectV2::ParseContinuationToken(response);
	REQUIRE(result == "abc123token");
}

TEST_CASE("ParseContinuationToken - Tigris-style with xmlns", "[s3][xml][tigris]") {
	string response = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <Name>my-bucket</Name>
  <IsTruncated>true</IsTruncated>
  <NextContinuationToken xmlns="http://s3.amazonaws.com/doc/2006-03-01/">abc123token</NextContinuationToken>
</ListBucketResult>)";

	auto result = AWSListObjectV2::ParseContinuationToken(response);
	REQUIRE(result == "abc123token");
}

TEST_CASE("ParseContinuationToken - not present", "[s3][xml]") {
	string response = R"(<?xml version="1.0" encoding="UTF-8"?>
<ListBucketResult xmlns="http://s3.amazonaws.com/doc/2006-03-01/">
  <Name>my-bucket</Name>
  <IsTruncated>false</IsTruncated>
</ListBucketResult>)";

	auto result = AWSListObjectV2::ParseContinuationToken(response);
	REQUIRE(result.empty());
}
