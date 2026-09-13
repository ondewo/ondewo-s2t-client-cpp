// Assertions against the concrete C++ types the S2T stubs generate.
//
// This is the per-product half of the suite: it names ondewo::s2t types, so replicating
// the suite to another ONDEWO client means rewriting this file against that product's
// messages and services. Everything generic lives in test_generated_stubs.cc.

#include <chrono>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>

#include "ondewo/s2t/speech-to-text.grpc.pb.h"
#include "ondewo/s2t/speech-to-text.pb.h"

namespace ondewo_client_test {
namespace {

// A channel to a port nothing listens on. gRPC connects lazily, so constructing stubs
// against it touches no network at all; the one test that does issue an RPC gives it a
// short deadline and asserts only that the call comes back as a failure.
std::shared_ptr<grpc::Channel> DeadChannel() {
  return grpc::CreateChannel("127.0.0.1:1", grpc::InsecureChannelCredentials());
}

TEST(TypedApi, MessageSurvivesSerializeAndParse) {
  ondewo::s2t::TranscribeStreamRequest original;
  original.set_audio_chunk("RIFF....WAVE");
  original.set_end_of_stream(true);
  original.set_mute_audio(true);

  ondewo::s2t::TranscribeRequestConfig* config = original.mutable_config();
  config->set_s2t_pipeline_id("pipeline-de-1");
  config->set_decoding(ondewo::s2t::Decoding::BEAM_SEARCH_WITH_LM);
  config->set_language_model_name("lm-de");

  std::string bytes;
  ASSERT_TRUE(original.SerializeToString(&bytes));
  EXPECT_FALSE(bytes.empty());

  ondewo::s2t::TranscribeStreamRequest parsed;
  ASSERT_TRUE(parsed.ParseFromString(bytes));

  EXPECT_EQ(parsed.audio_chunk(), "RIFF....WAVE");
  EXPECT_TRUE(parsed.end_of_stream());
  EXPECT_TRUE(parsed.mute_audio());
  ASSERT_TRUE(parsed.has_config());
  EXPECT_EQ(parsed.config().s2t_pipeline_id(), "pipeline-de-1");
  EXPECT_EQ(parsed.config().decoding(), ondewo::s2t::Decoding::BEAM_SEARCH_WITH_LM);
  // A oneof member reports presence of its own, which is what distinguishes "the caller
  // asked for the default language model" from "the caller said nothing".
  EXPECT_TRUE(parsed.config().has_language_model_name());
  EXPECT_EQ(parsed.config().language_model_name(), "lm-de");
  EXPECT_EQ(parsed.SerializeAsString(), bytes);
}

// `optional string language = 9` has proto3 explicit presence. Set to "" - the type's
// default - it must still reach the wire and still read back as *present*; a generator
// that drops the presence bit makes "" unsendable, which is exactly the class of bug that
// hit the Angular client.
TEST(TypedApi, ExplicitPresenceFieldSurvivesItsZeroValue) {
  ondewo::s2t::TranscribeRequestConfig original;
  EXPECT_FALSE(original.has_language());

  original.set_language("");
  ASSERT_TRUE(original.has_language());

  const std::string bytes = original.SerializeAsString();
  EXPECT_FALSE(bytes.empty()) << "an explicitly present empty string was not written to the wire";

  ondewo::s2t::TranscribeRequestConfig parsed;
  ASSERT_TRUE(parsed.ParseFromString(bytes));
  EXPECT_TRUE(parsed.has_language()) << "presence of an empty value was lost on the wire";
  EXPECT_EQ(parsed.language(), "");

  original.clear_language();
  EXPECT_FALSE(original.has_language());
  EXPECT_TRUE(original.SerializeAsString().empty());
}

// A plain (non-optional) proto3 scalar has the opposite contract: its zero value is the
// default and must NOT be written. Asserting both directions is what proves the two field
// kinds really are generated differently.
TEST(TypedApi, PlainScalarZeroValueStaysOffTheWire) {
  ondewo::s2t::TranscribeRequestConfig config;
  config.set_s2t_pipeline_id("");
  EXPECT_TRUE(config.SerializeAsString().empty());

  config.set_s2t_pipeline_id("pipeline-de-1");
  EXPECT_FALSE(config.SerializeAsString().empty());
}

TEST(TypedApi, EnumZeroValueIsTheUnspecifiedOne) {
  EXPECT_EQ(static_cast<int>(ondewo::s2t::Decoding::DEFAULT), 0);
  EXPECT_EQ(ondewo::s2t::Decoding_Name(ondewo::s2t::Decoding::DEFAULT), "DEFAULT");
  EXPECT_EQ(static_cast<int>(ondewo::s2t::InferenceBackend::INFERENCE_BACKEND_UNKNOWN), 0);

  ondewo::s2t::Decoding parsed = ondewo::s2t::Decoding::BEAM_SEARCH;
  ASSERT_TRUE(ondewo::s2t::Decoding_Parse("DEFAULT", &parsed));
  EXPECT_EQ(parsed, ondewo::s2t::Decoding::DEFAULT);

  // A request defaults to the zero decoding, so the zero value has to be requestable.
  ondewo::s2t::TranscribeRequestConfig config;
  EXPECT_EQ(config.decoding(), ondewo::s2t::Decoding::DEFAULT);
}

TEST(TypedApi, ServiceStubsAreConstructibleAgainstAChannel) {
  const std::shared_ptr<grpc::Channel> channel = DeadChannel();
  ASSERT_NE(channel, nullptr);

  std::unique_ptr<ondewo::s2t::Speech2Text::Stub> speech_to_text =
      ondewo::s2t::Speech2Text::NewStub(channel);

  EXPECT_NE(speech_to_text, nullptr);
}

TEST(TypedApi, ServicesKeepTheirFullyQualifiedNames) {
  EXPECT_STREQ(ondewo::s2t::Speech2Text::service_full_name(), "ondewo.s2t.Speech2Text");
}

// Actually issue an RPC. Nothing is listening, so the only correct outcome is a failure -
// but reaching a transport-level failure means the stub, the request/response types and
// the generated method descriptor all linked and dispatched. A crash or an OK here would
// mean the generated client is broken.
TEST(TypedApi, UnaryRpcAgainstADeadEndpointFailsCleanly) {
  std::unique_ptr<ondewo::s2t::Speech2Text::Stub> speech_to_text =
      ondewo::s2t::Speech2Text::NewStub(DeadChannel());

  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

  ondewo::s2t::S2tPipelineId request;
  request.set_id("pipeline-de-1");
  ondewo::s2t::Speech2TextConfig response;

  const grpc::Status status =
      speech_to_text->GetS2tPipeline(&client_context, request, &response);

  EXPECT_FALSE(status.ok()) << "an RPC to a dead endpoint reported success";
  EXPECT_TRUE(status.error_code() == grpc::StatusCode::UNAVAILABLE ||
              status.error_code() == grpc::StatusCode::DEADLINE_EXCEEDED)
      << "unexpected status " << status.error_code() << ": " << status.error_message();
}

// TranscribeStream is bidirectional, so it gets its own generated ClientReaderWriter type.
// Driving one proves that half of the generated service compiled and dispatches too.
TEST(TypedApi, BidiStreamingRpcStubIsUsable) {
  std::unique_ptr<ondewo::s2t::Speech2Text::Stub> speech_to_text =
      ondewo::s2t::Speech2Text::NewStub(DeadChannel());

  grpc::ClientContext client_context;
  client_context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(5));

  std::unique_ptr<grpc::ClientReaderWriterInterface<ondewo::s2t::TranscribeStreamRequest,
                                                    ondewo::s2t::TranscribeStreamResponse>>
      stream(speech_to_text->TranscribeStream(&client_context));
  ASSERT_NE(stream, nullptr);

  ondewo::s2t::TranscribeStreamRequest request;
  request.set_audio_chunk("RIFF....WAVE");
  request.set_end_of_stream(true);
  stream->Write(request);
  stream->WritesDone();

  ondewo::s2t::TranscribeStreamResponse response;
  EXPECT_FALSE(stream->Read(&response)) << "a dead endpoint returned a streamed response";

  const grpc::Status status = stream->Finish();
  EXPECT_FALSE(status.ok()) << "a stream to a dead endpoint reported success";
}

}  // namespace
}  // namespace ondewo_client_test
