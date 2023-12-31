// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: echo.proto

#ifndef PROTOBUF_INCLUDED_echo_2eproto
#define PROTOBUF_INCLUDED_echo_2eproto

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3006001
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3006001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
#include "base.pb.h"
// @@protoc_insertion_point(includes)
#define PROTOBUF_INTERNAL_EXPORT_protobuf_echo_2eproto 

namespace protobuf_echo_2eproto {
// Internal implementation detail -- do not use these members.
struct TableStruct {
  static const ::google::protobuf::internal::ParseTableField entries[];
  static const ::google::protobuf::internal::AuxillaryParseTableField aux[];
  static const ::google::protobuf::internal::ParseTable schema[4];
  static const ::google::protobuf::internal::FieldMetadata field_metadata[];
  static const ::google::protobuf::internal::SerializationTable serialization_table[];
  static const ::google::protobuf::uint32 offsets[];
};
void AddDescriptors();
}  // namespace protobuf_echo_2eproto
namespace MySvr {
namespace Echo {
class EchoMySelfRequest;
class EchoMySelfRequestDefaultTypeInternal;
extern EchoMySelfRequestDefaultTypeInternal _EchoMySelfRequest_default_instance_;
class EchoMySelfResponse;
class EchoMySelfResponseDefaultTypeInternal;
extern EchoMySelfResponseDefaultTypeInternal _EchoMySelfResponse_default_instance_;
class FastRespRequest;
class FastRespRequestDefaultTypeInternal;
extern FastRespRequestDefaultTypeInternal _FastRespRequest_default_instance_;
class OneWayMessage;
class OneWayMessageDefaultTypeInternal;
extern OneWayMessageDefaultTypeInternal _OneWayMessage_default_instance_;
}  // namespace Echo
}  // namespace MySvr
namespace google {
namespace protobuf {
template<> ::MySvr::Echo::EchoMySelfRequest* Arena::CreateMaybeMessage<::MySvr::Echo::EchoMySelfRequest>(Arena*);
template<> ::MySvr::Echo::EchoMySelfResponse* Arena::CreateMaybeMessage<::MySvr::Echo::EchoMySelfResponse>(Arena*);
template<> ::MySvr::Echo::FastRespRequest* Arena::CreateMaybeMessage<::MySvr::Echo::FastRespRequest>(Arena*);
template<> ::MySvr::Echo::OneWayMessage* Arena::CreateMaybeMessage<::MySvr::Echo::OneWayMessage>(Arena*);
}  // namespace protobuf
}  // namespace google
namespace MySvr {
namespace Echo {

// ===================================================================

class EchoMySelfRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:MySvr.Echo.EchoMySelfRequest) */ {
 public:
  EchoMySelfRequest();
  virtual ~EchoMySelfRequest();

  EchoMySelfRequest(const EchoMySelfRequest& from);

  inline EchoMySelfRequest& operator=(const EchoMySelfRequest& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  EchoMySelfRequest(EchoMySelfRequest&& from) noexcept
    : EchoMySelfRequest() {
    *this = ::std::move(from);
  }

  inline EchoMySelfRequest& operator=(EchoMySelfRequest&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const EchoMySelfRequest& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const EchoMySelfRequest* internal_default_instance() {
    return reinterpret_cast<const EchoMySelfRequest*>(
               &_EchoMySelfRequest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  void Swap(EchoMySelfRequest* other);
  friend void swap(EchoMySelfRequest& a, EchoMySelfRequest& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline EchoMySelfRequest* New() const final {
    return CreateMaybeMessage<EchoMySelfRequest>(NULL);
  }

  EchoMySelfRequest* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<EchoMySelfRequest>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const EchoMySelfRequest& from);
  void MergeFrom(const EchoMySelfRequest& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(EchoMySelfRequest* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string message = 1;
  void clear_message();
  static const int kMessageFieldNumber = 1;
  const ::std::string& message() const;
  void set_message(const ::std::string& value);
  #if LANG_CXX11
  void set_message(::std::string&& value);
  #endif
  void set_message(const char* value);
  void set_message(const char* value, size_t size);
  ::std::string* mutable_message();
  ::std::string* release_message();
  void set_allocated_message(::std::string* message);

  // @@protoc_insertion_point(class_scope:MySvr.Echo.EchoMySelfRequest)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr message_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_echo_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class EchoMySelfResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:MySvr.Echo.EchoMySelfResponse) */ {
 public:
  EchoMySelfResponse();
  virtual ~EchoMySelfResponse();

  EchoMySelfResponse(const EchoMySelfResponse& from);

  inline EchoMySelfResponse& operator=(const EchoMySelfResponse& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  EchoMySelfResponse(EchoMySelfResponse&& from) noexcept
    : EchoMySelfResponse() {
    *this = ::std::move(from);
  }

  inline EchoMySelfResponse& operator=(EchoMySelfResponse&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const EchoMySelfResponse& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const EchoMySelfResponse* internal_default_instance() {
    return reinterpret_cast<const EchoMySelfResponse*>(
               &_EchoMySelfResponse_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    1;

  void Swap(EchoMySelfResponse* other);
  friend void swap(EchoMySelfResponse& a, EchoMySelfResponse& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline EchoMySelfResponse* New() const final {
    return CreateMaybeMessage<EchoMySelfResponse>(NULL);
  }

  EchoMySelfResponse* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<EchoMySelfResponse>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const EchoMySelfResponse& from);
  void MergeFrom(const EchoMySelfResponse& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(EchoMySelfResponse* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string message = 1;
  void clear_message();
  static const int kMessageFieldNumber = 1;
  const ::std::string& message() const;
  void set_message(const ::std::string& value);
  #if LANG_CXX11
  void set_message(::std::string&& value);
  #endif
  void set_message(const char* value);
  void set_message(const char* value, size_t size);
  ::std::string* mutable_message();
  ::std::string* release_message();
  void set_allocated_message(::std::string* message);

  // @@protoc_insertion_point(class_scope:MySvr.Echo.EchoMySelfResponse)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr message_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_echo_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class OneWayMessage : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:MySvr.Echo.OneWayMessage) */ {
 public:
  OneWayMessage();
  virtual ~OneWayMessage();

  OneWayMessage(const OneWayMessage& from);

  inline OneWayMessage& operator=(const OneWayMessage& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  OneWayMessage(OneWayMessage&& from) noexcept
    : OneWayMessage() {
    *this = ::std::move(from);
  }

  inline OneWayMessage& operator=(OneWayMessage&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const OneWayMessage& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const OneWayMessage* internal_default_instance() {
    return reinterpret_cast<const OneWayMessage*>(
               &_OneWayMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    2;

  void Swap(OneWayMessage* other);
  friend void swap(OneWayMessage& a, OneWayMessage& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline OneWayMessage* New() const final {
    return CreateMaybeMessage<OneWayMessage>(NULL);
  }

  OneWayMessage* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<OneWayMessage>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const OneWayMessage& from);
  void MergeFrom(const OneWayMessage& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(OneWayMessage* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string message = 1;
  void clear_message();
  static const int kMessageFieldNumber = 1;
  const ::std::string& message() const;
  void set_message(const ::std::string& value);
  #if LANG_CXX11
  void set_message(::std::string&& value);
  #endif
  void set_message(const char* value);
  void set_message(const char* value, size_t size);
  ::std::string* mutable_message();
  ::std::string* release_message();
  void set_allocated_message(::std::string* message);

  // @@protoc_insertion_point(class_scope:MySvr.Echo.OneWayMessage)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr message_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_echo_2eproto::TableStruct;
};
// -------------------------------------------------------------------

class FastRespRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:MySvr.Echo.FastRespRequest) */ {
 public:
  FastRespRequest();
  virtual ~FastRespRequest();

  FastRespRequest(const FastRespRequest& from);

  inline FastRespRequest& operator=(const FastRespRequest& from) {
    CopyFrom(from);
    return *this;
  }
  #if LANG_CXX11
  FastRespRequest(FastRespRequest&& from) noexcept
    : FastRespRequest() {
    *this = ::std::move(from);
  }

  inline FastRespRequest& operator=(FastRespRequest&& from) noexcept {
    if (GetArenaNoVirtual() == from.GetArenaNoVirtual()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }
  #endif
  static const ::google::protobuf::Descriptor* descriptor();
  static const FastRespRequest& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const FastRespRequest* internal_default_instance() {
    return reinterpret_cast<const FastRespRequest*>(
               &_FastRespRequest_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    3;

  void Swap(FastRespRequest* other);
  friend void swap(FastRespRequest& a, FastRespRequest& b) {
    a.Swap(&b);
  }

  // implements Message ----------------------------------------------

  inline FastRespRequest* New() const final {
    return CreateMaybeMessage<FastRespRequest>(NULL);
  }

  FastRespRequest* New(::google::protobuf::Arena* arena) const final {
    return CreateMaybeMessage<FastRespRequest>(arena);
  }
  void CopyFrom(const ::google::protobuf::Message& from) final;
  void MergeFrom(const ::google::protobuf::Message& from) final;
  void CopyFrom(const FastRespRequest& from);
  void MergeFrom(const FastRespRequest& from);
  void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input) final;
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const final;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic, ::google::protobuf::uint8* target) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(FastRespRequest* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return NULL;
  }
  inline void* MaybeArenaPtr() const {
    return NULL;
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const final;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // string message = 1;
  void clear_message();
  static const int kMessageFieldNumber = 1;
  const ::std::string& message() const;
  void set_message(const ::std::string& value);
  #if LANG_CXX11
  void set_message(::std::string&& value);
  #endif
  void set_message(const char* value);
  void set_message(const char* value, size_t size);
  ::std::string* mutable_message();
  ::std::string* release_message();
  void set_allocated_message(::std::string* message);

  // @@protoc_insertion_point(class_scope:MySvr.Echo.FastRespRequest)
 private:

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::internal::ArenaStringPtr message_;
  mutable ::google::protobuf::internal::CachedSize _cached_size_;
  friend struct ::protobuf_echo_2eproto::TableStruct;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// EchoMySelfRequest

// string message = 1;
inline void EchoMySelfRequest::clear_message() {
  message_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& EchoMySelfRequest::message() const {
  // @@protoc_insertion_point(field_get:MySvr.Echo.EchoMySelfRequest.message)
  return message_.GetNoArena();
}
inline void EchoMySelfRequest::set_message(const ::std::string& value) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:MySvr.Echo.EchoMySelfRequest.message)
}
#if LANG_CXX11
inline void EchoMySelfRequest::set_message(::std::string&& value) {
  
  message_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:MySvr.Echo.EchoMySelfRequest.message)
}
#endif
inline void EchoMySelfRequest::set_message(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:MySvr.Echo.EchoMySelfRequest.message)
}
inline void EchoMySelfRequest::set_message(const char* value, size_t size) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:MySvr.Echo.EchoMySelfRequest.message)
}
inline ::std::string* EchoMySelfRequest::mutable_message() {
  
  // @@protoc_insertion_point(field_mutable:MySvr.Echo.EchoMySelfRequest.message)
  return message_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* EchoMySelfRequest::release_message() {
  // @@protoc_insertion_point(field_release:MySvr.Echo.EchoMySelfRequest.message)
  
  return message_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void EchoMySelfRequest::set_allocated_message(::std::string* message) {
  if (message != NULL) {
    
  } else {
    
  }
  message_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), message);
  // @@protoc_insertion_point(field_set_allocated:MySvr.Echo.EchoMySelfRequest.message)
}

// -------------------------------------------------------------------

// EchoMySelfResponse

// string message = 1;
inline void EchoMySelfResponse::clear_message() {
  message_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& EchoMySelfResponse::message() const {
  // @@protoc_insertion_point(field_get:MySvr.Echo.EchoMySelfResponse.message)
  return message_.GetNoArena();
}
inline void EchoMySelfResponse::set_message(const ::std::string& value) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:MySvr.Echo.EchoMySelfResponse.message)
}
#if LANG_CXX11
inline void EchoMySelfResponse::set_message(::std::string&& value) {
  
  message_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:MySvr.Echo.EchoMySelfResponse.message)
}
#endif
inline void EchoMySelfResponse::set_message(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:MySvr.Echo.EchoMySelfResponse.message)
}
inline void EchoMySelfResponse::set_message(const char* value, size_t size) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:MySvr.Echo.EchoMySelfResponse.message)
}
inline ::std::string* EchoMySelfResponse::mutable_message() {
  
  // @@protoc_insertion_point(field_mutable:MySvr.Echo.EchoMySelfResponse.message)
  return message_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* EchoMySelfResponse::release_message() {
  // @@protoc_insertion_point(field_release:MySvr.Echo.EchoMySelfResponse.message)
  
  return message_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void EchoMySelfResponse::set_allocated_message(::std::string* message) {
  if (message != NULL) {
    
  } else {
    
  }
  message_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), message);
  // @@protoc_insertion_point(field_set_allocated:MySvr.Echo.EchoMySelfResponse.message)
}

// -------------------------------------------------------------------

// OneWayMessage

// string message = 1;
inline void OneWayMessage::clear_message() {
  message_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& OneWayMessage::message() const {
  // @@protoc_insertion_point(field_get:MySvr.Echo.OneWayMessage.message)
  return message_.GetNoArena();
}
inline void OneWayMessage::set_message(const ::std::string& value) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:MySvr.Echo.OneWayMessage.message)
}
#if LANG_CXX11
inline void OneWayMessage::set_message(::std::string&& value) {
  
  message_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:MySvr.Echo.OneWayMessage.message)
}
#endif
inline void OneWayMessage::set_message(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:MySvr.Echo.OneWayMessage.message)
}
inline void OneWayMessage::set_message(const char* value, size_t size) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:MySvr.Echo.OneWayMessage.message)
}
inline ::std::string* OneWayMessage::mutable_message() {
  
  // @@protoc_insertion_point(field_mutable:MySvr.Echo.OneWayMessage.message)
  return message_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* OneWayMessage::release_message() {
  // @@protoc_insertion_point(field_release:MySvr.Echo.OneWayMessage.message)
  
  return message_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void OneWayMessage::set_allocated_message(::std::string* message) {
  if (message != NULL) {
    
  } else {
    
  }
  message_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), message);
  // @@protoc_insertion_point(field_set_allocated:MySvr.Echo.OneWayMessage.message)
}

// -------------------------------------------------------------------

// FastRespRequest

// string message = 1;
inline void FastRespRequest::clear_message() {
  message_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& FastRespRequest::message() const {
  // @@protoc_insertion_point(field_get:MySvr.Echo.FastRespRequest.message)
  return message_.GetNoArena();
}
inline void FastRespRequest::set_message(const ::std::string& value) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:MySvr.Echo.FastRespRequest.message)
}
#if LANG_CXX11
inline void FastRespRequest::set_message(::std::string&& value) {
  
  message_.SetNoArena(
    &::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::move(value));
  // @@protoc_insertion_point(field_set_rvalue:MySvr.Echo.FastRespRequest.message)
}
#endif
inline void FastRespRequest::set_message(const char* value) {
  GOOGLE_DCHECK(value != NULL);
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:MySvr.Echo.FastRespRequest.message)
}
inline void FastRespRequest::set_message(const char* value, size_t size) {
  
  message_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:MySvr.Echo.FastRespRequest.message)
}
inline ::std::string* FastRespRequest::mutable_message() {
  
  // @@protoc_insertion_point(field_mutable:MySvr.Echo.FastRespRequest.message)
  return message_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* FastRespRequest::release_message() {
  // @@protoc_insertion_point(field_release:MySvr.Echo.FastRespRequest.message)
  
  return message_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void FastRespRequest::set_allocated_message(::std::string* message) {
  if (message != NULL) {
    
  } else {
    
  }
  message_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), message);
  // @@protoc_insertion_point(field_set_allocated:MySvr.Echo.FastRespRequest.message)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace Echo
}  // namespace MySvr

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_INCLUDED_echo_2eproto
