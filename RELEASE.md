# Release History

*****************

## Release ONDEWO S2T C++ Client 7.6.2

### New Features

* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) Keycloak authentication with offline-token start-up, matching ondewo-s2t-client-python 7.6.2 (this release carries the python SDK's 7.6.1 and 7.6.2 Keycloak work at once, so every language shares one version for one feature set): the header-only `<ondewo/client/keycloak.h>`: `ClientConfig` gains `keycloak_url`, `realm`, `client_id`, `username`, `password`, `token_expiration_in_s`, `verify_ssl` and `refresh_token` (appended last), plus `KeycloakTokenProvider`, a gRPC `MetadataCredentialsPlugin`, `CreateAuthenticatedChannel()` and `AddKeycloakAuthorization()`; the libcurl transport lives in `<ondewo/client/keycloak_curl.h>`, so only its users link libcurl.
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) A handed-off Keycloak *offline* refresh token starts the client with one `grant_type=refresh_token` exchange, and the client then never sends a password grant, not even as a fallback; a refused token is never answered with a password login. A Keycloak config needs `password` OR `refresh_token`, all Keycloak fields are validated all-or-nothing, and the password and the token are redacted in every rendering.
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) Resilient refresh: a failed refresh never ends the refresh loop; the proactive refresh and the per-call (lazy) refresh share one full-jitter backoff (5 s doubling to 300 s, measured from the recorded failure) and send no request while it runs; concurrent callers share one in-flight refresh. A still-valid token keeps being served while a refresh fails, and only an expired token raises `KeycloakAuthenticationError`.
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) One shared provider per credential set, keyed by a SHA-256 of the credentials, so two configs of different tenants never share one. The token request has a 10 s timeout, quoted Keycloak error bodies are cut at 500 characters, and a token response is reported by its field names only, never its values.

### Bug Fixes

* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) A quoted error body is cut at 500 characters (UTF-8 code points), as python's `body[:500]`, instead of 500 bytes; a multi-byte character is never split.
* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) A provider whose `token_expiration_in_s` deadline ended its background refresh is still handed out by the registry, as in python; only a provider closed with `Stop()` / `Close()` is replaced (new `closed()` accessor).

### Improvements

* [[OND236-120]](https://ondewo.atlassian.net/browse/OND236-120) Tests: `tests/test_keycloak.cc` implements T01-T62 on a fake token endpoint and clock, plus the bearer header on the wire and the curl transport on loopback; `make coverage` keeps 100 % lines.

API unchanged: tracking API Version [7.6.0](https://github.com/ondewo/ondewo-s2t-api/releases/tag/7.6.0) ( [Documentation](https://ondewo.github.io/ondewo-s2t-api/) )

*****************

## Release ONDEWO S2T C++ Client 7.6.0

### Improvements

* Tracking API Version [7.6.0](https://github.com/ondewo/ondewo-s2t-api/releases/tag/7.6.0) ( [Documentation](https://ondewo.github.io/ondewo-s2t-api/) )
* Generated with ondewo-proto-compiler 5.17.0 (protoc 35.1). Runtime floors unchanged: protobuf 3.21.12 / grpc 1.51.1 (Debian trixie).
* The build and the release run in docker, as in ondewo-nlu-client-cpp: CMake, protobuf/gRPC, GoogleTest and `gh` come from the utils image built from `Dockerfile.utils`, the compiler container runs as the invoking user (no `sudo`), the GitHub token is validated before anything is pushed, and the GitHub release is created last, in one `gh release create` with the package archive and its `.sha256` attached. The tag-triggered `release.yml` workflow is removed: CI only tests.

*****************

## Release ONDEWO S2T C++ Client 7.5.1

### New Features

* TLS / mutual TLS: new header-only channel helper `ondewo/client/channel.h`, installed beside the generated
  headers and on the exported `ondewo::ondewo_s2t_client` target's include path. `ondewo::client::ClientConfig`
  (`host`, `port`, `grpc_cert`, `grpc_client_cert`, `grpc_client_key`, `use_secure_channel`) takes PEM **content**,
  not file paths. `CreateChannel`, `CreateChannelCredentials`, `ValidateConfig` and `DefaultChannelArguments()`
  report errors as `grpc::Status` `INVALID_ARGUMENT`, never as exceptions.
* An empty `grpc_cert` uses the platform trust store. Half a client identity (certificate without key or key
  without certificate) and a plaintext channel with a client identity are refused before gRPC is called. A
  plaintext channel logs a warning on `std::clog` naming `host:port`. No error message renders a PEM, a key or the
  config; `ToString()` / `operator<<` print the client key as `***REDACTED***`. Bare IPv6 hosts are bracketed;
  CRLF PEMs work.
* Default channel arguments match the ONDEWO Python SDKs: keepalive every 30 s during calls with a 20 s timeout,
  `grpc.http2.max_pings_without_data=2`, reconnect backoff capped at 5 s, 2^31-1 byte send/receive message limit.
  Documented gaps: gRPC 1.51 ignores `grpc.http2.ping_timeout_ms`, and no per-method retry policy is configured.
* README: new section "TLS, mutual TLS and certificates" (modes, loading a PEM from a file, test PKI, security
  notes, troubleshooting).

### Improvements

* `tests/test_tls.cc`: real handshakes against in-process servers with a PKI generated by the openssl CLI at test
  time - TLS, mutual TLS, server rejecting a missing or foreign client certificate, wrong CA, system trust store,
  non-PEM CA (error, not crash), CRLF PEMs, server-name override, `[::1]` - plus the refusal, warning, redaction
  and default-argument cases. `make coverage` now also holds `client/` to 100 % line coverage.
* `tests/test_release_notes.sh` (CTest) pins the release-notes slice, the heading spelling, the `*****` separators
  and non-empty notes for the current version.
* The pinned `ondewo-proto-compiler` submodule moves from 5.15.1 to 5.15.2 (its fixes concern the PHP, Rust and
  JavaScript images and the release automation; the C++ generation is unchanged).
* Tracking API Version [7.5.0](https://github.com/ondewo/ondewo-s2t-api/releases/tag/7.5.0) ( [Documentation](https://ondewo.github.io/ondewo-s2t-api/) )

*****************

## Release ONDEWO S2T C++ Client 7.5.0

### New Features

* Initial release of `ondewo-s2t-client-cpp`, the C++ gRPC client library for the
  [ONDEWO S2T API](https://github.com/ondewo/ondewo-s2t-api) (Speech-to-Text). The stubs are generated
  from the `ondewo-s2t-api` submodule by the `ondewo-cpp-proto-compiler` image of
  [ondewo-proto-compiler](https://github.com/ondewo/ondewo-proto-compiler) 5.15.1, using protoc's
  built-in `--cpp_out` for the messages and `grpc_cpp_plugin` for the service stubs.
* Ships as a consumable CMake package: `find_package(ondewo_s2t_client CONFIG REQUIRED)` plus
  `target_link_libraries(my_app PRIVATE ondewo::ondewo_s2t_client)`. The package installs the generated
  headers under `include/ondewo_s2t_client/`, the static archive
  `lib/libondewo_s2t_client.a` and `lib/cmake/ondewo_s2t_client/`, and re-finds `Protobuf` and
  `gRPC` on the consumer's machine through `find_dependency`.
* `public-api.h` is an umbrella header that includes every generated `*.pb.h` and `*.grpc.pb.h`, so a single
  `#include "public-api.h"` gives access to the whole API surface.
* The generated stub sources are committed to the repository. C++ has no package registry, so a git tag is the
  distribution channel: a plain `git clone` or a CMake `FetchContent_Declare` yields a buildable project with
  no Docker and no code generation step.
* `make build` reproduces the whole pipeline - submodule checkout at the pinned tags, compiler image build,
  code generation and a host-native CMake build - and `make test` verifies that every `.proto` produced a
  header and that the installed CMake package is consumable by a downstream project.

*****************
