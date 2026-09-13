#include "product_config.h"

namespace ondewo_client_test {

// Mirrors the #include list of the generated public-api.h, one .proto per pair of headers:
//   sed -n 's|^#include "\(.*\)\.pb\.h"$|\1|p' public-api.h | sed 's|\.grpc$||' | sort -u
//
// The ONDEWO S2T API is a single .proto file. Its imports (google/protobuf/empty.proto,
// struct.proto, timestamp.proto) are well-known types that live inside libprotobuf, so no
// google/* code is generated into api/ and none is listed here.
const std::vector<std::string> kProtoFileNames = {
    "ondewo/s2t/speech-to-text.proto",
};

const std::vector<std::string> kServiceFullNames = {
    "ondewo.s2t.Speech2Text",
};

const std::vector<ExpectedMethod> kExpectedMethods = {
    // The two RPCs the product exists for, one of them bidirectionally streaming ...
    {"ondewo.s2t.Speech2Text", "TranscribeFile"},
    {"ondewo.s2t.Speech2Text", "TranscribeStream"},
    // ... the complete CRUD surface over the pipeline configurations ...
    {"ondewo.s2t.Speech2Text", "GetS2tPipeline"},
    {"ondewo.s2t.Speech2Text", "CreateS2tPipeline"},
    {"ondewo.s2t.Speech2Text", "UpdateS2tPipeline"},
    {"ondewo.s2t.Speech2Text", "DeleteS2tPipeline"},
    {"ondewo.s2t.Speech2Text", "ListS2tPipelines"},
    // ... the listing RPCs ...
    {"ondewo.s2t.Speech2Text", "ListS2tLanguages"},
    {"ondewo.s2t.Speech2Text", "ListS2tDomains"},
    {"ondewo.s2t.Speech2Text", "ListS2tLanguageModels"},
    {"ondewo.s2t.Speech2Text", "ListS2tNormalizationPipelines"},
    // ... the user-language-model lifecycle ...
    {"ondewo.s2t.Speech2Text", "CreateUserLanguageModel"},
    {"ondewo.s2t.Speech2Text", "AddDataToUserLanguageModel"},
    {"ondewo.s2t.Speech2Text", "TrainUserLanguageModel"},
    {"ondewo.s2t.Speech2Text", "DeleteUserLanguageModel"},
    // ... and the RPC whose request type comes from google.protobuf rather than from S2T.
    {"ondewo.s2t.Speech2Text", "GetServiceInfo"},
};

// TranscribeRequestConfig carries a plain string, an enum, a oneof-wrapped string and two
// proto3 `optional` strings - the widest singular-scalar mix in the API.
const std::string kScalarMessageFullName = "ondewo.s2t.TranscribeRequestConfig";

const std::string kEnumFullName = "ondewo.s2t.Decoding";

// ONDEWO S2T API 7.5.0 generates 69 messages (the two map<> entry types excluded), 7 enums
// and 213 singular scalar fields across the file listed above. The floors sit just below
// that; all seven enums are declared at file scope, so 7 is the honest enum floor.
const int kMinimumMessageCount = 65;
const int kMinimumEnumCount = 7;
const int kMinimumScalarFieldCount = 200;

}  // namespace ondewo_client_test
