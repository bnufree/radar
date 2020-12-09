// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: ZCHXBd.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "ZCHXBd.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace com {
namespace zhichenhaixin {
namespace proto {

namespace {

const ::google::protobuf::Descriptor* BD_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  BD_reflection_ = NULL;
const ::google::protobuf::Descriptor* BDList_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  BDList_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_ZCHXBd_2eproto() {
  protobuf_AddDesc_ZCHXBd_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "ZCHXBd.proto");
  GOOGLE_CHECK(file != NULL);
  BD_descriptor_ = file->message_type(0);
  static const int BD_offsets_[7] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BD, hour_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BD, minute_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BD, second_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BD, millisecond_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BD, lon_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BD, lat_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BD, id_),
  };
  BD_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      BD_descriptor_,
      BD::default_instance_,
      BD_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BD, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BD, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(BD));
  BDList_descriptor_ = file->message_type(1);
  static const int BDList_offsets_[2] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BDList, utc_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BDList, bd_),
  };
  BDList_reflection_ =
    new ::google::protobuf::internal::GeneratedMessageReflection(
      BDList_descriptor_,
      BDList::default_instance_,
      BDList_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BDList, _has_bits_[0]),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(BDList, _unknown_fields_),
      -1,
      ::google::protobuf::DescriptorPool::generated_pool(),
      ::google::protobuf::MessageFactory::generated_factory(),
      sizeof(BDList));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_ZCHXBd_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    BD_descriptor_, &BD::default_instance());
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
    BDList_descriptor_, &BDList::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_ZCHXBd_2eproto() {
  delete BD::default_instance_;
  delete BD_reflection_;
  delete BDList::default_instance_;
  delete BDList_reflection_;
}

void protobuf_AddDesc_ZCHXBd_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\014ZCHXBd.proto\022\027com.zhichenhaixin.proto\""
    "m\n\002BD\022\014\n\004hour\030\001 \002(\005\022\016\n\006minute\030\002 \002(\005\022\016\n\006s"
    "econd\030\003 \002(\005\022\023\n\013millisecond\030\004 \002(\005\022\013\n\003lon\030"
    "\005 \002(\001\022\013\n\003lat\030\006 \002(\001\022\n\n\002id\030\007 \002(\t\">\n\006BDList"
    "\022\013\n\003UTC\030\001 \002(\004\022\'\n\002bd\030\002 \003(\0132\033.com.zhichenh"
    "aixin.proto.BD", 214);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "ZCHXBd.proto", &protobuf_RegisterTypes);
  BD::default_instance_ = new BD();
  BDList::default_instance_ = new BDList();
  BD::default_instance_->InitAsDefaultInstance();
  BDList::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_ZCHXBd_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_ZCHXBd_2eproto {
  StaticDescriptorInitializer_ZCHXBd_2eproto() {
    protobuf_AddDesc_ZCHXBd_2eproto();
  }
} static_descriptor_initializer_ZCHXBd_2eproto_;

// ===================================================================

#ifndef _MSC_VER
const int BD::kHourFieldNumber;
const int BD::kMinuteFieldNumber;
const int BD::kSecondFieldNumber;
const int BD::kMillisecondFieldNumber;
const int BD::kLonFieldNumber;
const int BD::kLatFieldNumber;
const int BD::kIdFieldNumber;
#endif  // !_MSC_VER

BD::BD()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:com.zhichenhaixin.proto.BD)
}

void BD::InitAsDefaultInstance() {
}

BD::BD(const BD& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:com.zhichenhaixin.proto.BD)
}

void BD::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  hour_ = 0;
  minute_ = 0;
  second_ = 0;
  millisecond_ = 0;
  lon_ = 0;
  lat_ = 0;
  id_ = const_cast< ::std::string*>(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

BD::~BD() {
  // @@protoc_insertion_point(destructor:com.zhichenhaixin.proto.BD)
  SharedDtor();
}

void BD::SharedDtor() {
  if (id_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
    delete id_;
  }
  if (this != default_instance_) {
  }
}

void BD::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* BD::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return BD_descriptor_;
}

const BD& BD::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_ZCHXBd_2eproto();
  return *default_instance_;
}

BD* BD::default_instance_ = NULL;

BD* BD::New() const {
  return new BD;
}

void BD::Clear() {
#define OFFSET_OF_FIELD_(f) (reinterpret_cast<char*>(      \
  &reinterpret_cast<BD*>(16)->f) - \
   reinterpret_cast<char*>(16))

#define ZR_(first, last) do {                              \
    size_t f = OFFSET_OF_FIELD_(first);                    \
    size_t n = OFFSET_OF_FIELD_(last) - f + sizeof(last);  \
    ::memset(&first, 0, n);                                \
  } while (0)

  if (_has_bits_[0 / 32] & 127) {
    ZR_(hour_, lat_);
    if (has_id()) {
      if (id_ != &::google::protobuf::internal::GetEmptyStringAlreadyInited()) {
        id_->clear();
      }
    }
  }

#undef OFFSET_OF_FIELD_
#undef ZR_

  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool BD::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:com.zhichenhaixin.proto.BD)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required int32 hour = 1;
      case 1: {
        if (tag == 8) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &hour_)));
          set_has_hour();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(16)) goto parse_minute;
        break;
      }

      // required int32 minute = 2;
      case 2: {
        if (tag == 16) {
         parse_minute:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &minute_)));
          set_has_minute();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(24)) goto parse_second;
        break;
      }

      // required int32 second = 3;
      case 3: {
        if (tag == 24) {
         parse_second:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &second_)));
          set_has_second();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(32)) goto parse_millisecond;
        break;
      }

      // required int32 millisecond = 4;
      case 4: {
        if (tag == 32) {
         parse_millisecond:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::int32, ::google::protobuf::internal::WireFormatLite::TYPE_INT32>(
                 input, &millisecond_)));
          set_has_millisecond();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(41)) goto parse_lon;
        break;
      }

      // required double lon = 5;
      case 5: {
        if (tag == 41) {
         parse_lon:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &lon_)));
          set_has_lon();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(49)) goto parse_lat;
        break;
      }

      // required double lat = 6;
      case 6: {
        if (tag == 49) {
         parse_lat:
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   double, ::google::protobuf::internal::WireFormatLite::TYPE_DOUBLE>(
                 input, &lat_)));
          set_has_lat();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(58)) goto parse_id;
        break;
      }

      // required string id = 7;
      case 7: {
        if (tag == 58) {
         parse_id:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_id()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->id().data(), this->id().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "id");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:com.zhichenhaixin.proto.BD)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:com.zhichenhaixin.proto.BD)
  return false;
#undef DO_
}

void BD::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:com.zhichenhaixin.proto.BD)
  // required int32 hour = 1;
  if (has_hour()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(1, this->hour(), output);
  }

  // required int32 minute = 2;
  if (has_minute()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(2, this->minute(), output);
  }

  // required int32 second = 3;
  if (has_second()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(3, this->second(), output);
  }

  // required int32 millisecond = 4;
  if (has_millisecond()) {
    ::google::protobuf::internal::WireFormatLite::WriteInt32(4, this->millisecond(), output);
  }

  // required double lon = 5;
  if (has_lon()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(5, this->lon(), output);
  }

  // required double lat = 6;
  if (has_lat()) {
    ::google::protobuf::internal::WireFormatLite::WriteDouble(6, this->lat(), output);
  }

  // required string id = 7;
  if (has_id()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->id().data(), this->id().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "id");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      7, this->id(), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:com.zhichenhaixin.proto.BD)
}

::google::protobuf::uint8* BD::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:com.zhichenhaixin.proto.BD)
  // required int32 hour = 1;
  if (has_hour()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(1, this->hour(), target);
  }

  // required int32 minute = 2;
  if (has_minute()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(2, this->minute(), target);
  }

  // required int32 second = 3;
  if (has_second()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(3, this->second(), target);
  }

  // required int32 millisecond = 4;
  if (has_millisecond()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteInt32ToArray(4, this->millisecond(), target);
  }

  // required double lon = 5;
  if (has_lon()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(5, this->lon(), target);
  }

  // required double lat = 6;
  if (has_lat()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteDoubleToArray(6, this->lat(), target);
  }

  // required string id = 7;
  if (has_id()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->id().data(), this->id().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "id");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        7, this->id(), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:com.zhichenhaixin.proto.BD)
  return target;
}

int BD::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required int32 hour = 1;
    if (has_hour()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->hour());
    }

    // required int32 minute = 2;
    if (has_minute()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->minute());
    }

    // required int32 second = 3;
    if (has_second()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->second());
    }

    // required int32 millisecond = 4;
    if (has_millisecond()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::Int32Size(
          this->millisecond());
    }

    // required double lon = 5;
    if (has_lon()) {
      total_size += 1 + 8;
    }

    // required double lat = 6;
    if (has_lat()) {
      total_size += 1 + 8;
    }

    // required string id = 7;
    if (has_id()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->id());
    }

  }
  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void BD::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const BD* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const BD*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void BD::MergeFrom(const BD& from) {
  GOOGLE_CHECK_NE(&from, this);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_hour()) {
      set_hour(from.hour());
    }
    if (from.has_minute()) {
      set_minute(from.minute());
    }
    if (from.has_second()) {
      set_second(from.second());
    }
    if (from.has_millisecond()) {
      set_millisecond(from.millisecond());
    }
    if (from.has_lon()) {
      set_lon(from.lon());
    }
    if (from.has_lat()) {
      set_lat(from.lat());
    }
    if (from.has_id()) {
      set_id(from.id());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void BD::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void BD::CopyFrom(const BD& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool BD::IsInitialized() const {
  if ((_has_bits_[0] & 0x0000007f) != 0x0000007f) return false;

  return true;
}

void BD::Swap(BD* other) {
  if (other != this) {
    std::swap(hour_, other->hour_);
    std::swap(minute_, other->minute_);
    std::swap(second_, other->second_);
    std::swap(millisecond_, other->millisecond_);
    std::swap(lon_, other->lon_);
    std::swap(lat_, other->lat_);
    std::swap(id_, other->id_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata BD::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = BD_descriptor_;
  metadata.reflection = BD_reflection_;
  return metadata;
}


// ===================================================================

#ifndef _MSC_VER
const int BDList::kUTCFieldNumber;
const int BDList::kBdFieldNumber;
#endif  // !_MSC_VER

BDList::BDList()
  : ::google::protobuf::Message() {
  SharedCtor();
  // @@protoc_insertion_point(constructor:com.zhichenhaixin.proto.BDList)
}

void BDList::InitAsDefaultInstance() {
}

BDList::BDList(const BDList& from)
  : ::google::protobuf::Message() {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:com.zhichenhaixin.proto.BDList)
}

void BDList::SharedCtor() {
  _cached_size_ = 0;
  utc_ = GOOGLE_ULONGLONG(0);
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

BDList::~BDList() {
  // @@protoc_insertion_point(destructor:com.zhichenhaixin.proto.BDList)
  SharedDtor();
}

void BDList::SharedDtor() {
  if (this != default_instance_) {
  }
}

void BDList::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* BDList::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return BDList_descriptor_;
}

const BDList& BDList::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_ZCHXBd_2eproto();
  return *default_instance_;
}

BDList* BDList::default_instance_ = NULL;

BDList* BDList::New() const {
  return new BDList;
}

void BDList::Clear() {
  utc_ = GOOGLE_ULONGLONG(0);
  bd_.Clear();
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  mutable_unknown_fields()->Clear();
}

bool BDList::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:com.zhichenhaixin.proto.BDList)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // required uint64 UTC = 1;
      case 1: {
        if (tag == 8) {
          DO_((::google::protobuf::internal::WireFormatLite::ReadPrimitive<
                   ::google::protobuf::uint64, ::google::protobuf::internal::WireFormatLite::TYPE_UINT64>(
                 input, &utc_)));
          set_has_utc();
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(18)) goto parse_bd;
        break;
      }

      // repeated .com.zhichenhaixin.proto.BD bd = 2;
      case 2: {
        if (tag == 18) {
         parse_bd:
          DO_(::google::protobuf::internal::WireFormatLite::ReadMessageNoVirtual(
                input, add_bd()));
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(18)) goto parse_bd;
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:com.zhichenhaixin.proto.BDList)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:com.zhichenhaixin.proto.BDList)
  return false;
#undef DO_
}

void BDList::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:com.zhichenhaixin.proto.BDList)
  // required uint64 UTC = 1;
  if (has_utc()) {
    ::google::protobuf::internal::WireFormatLite::WriteUInt64(1, this->utc(), output);
  }

  // repeated .com.zhichenhaixin.proto.BD bd = 2;
  for (int i = 0; i < this->bd_size(); i++) {
    ::google::protobuf::internal::WireFormatLite::WriteMessageMaybeToArray(
      2, this->bd(i), output);
  }

  if (!unknown_fields().empty()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:com.zhichenhaixin.proto.BDList)
}

::google::protobuf::uint8* BDList::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:com.zhichenhaixin.proto.BDList)
  // required uint64 UTC = 1;
  if (has_utc()) {
    target = ::google::protobuf::internal::WireFormatLite::WriteUInt64ToArray(1, this->utc(), target);
  }

  // repeated .com.zhichenhaixin.proto.BD bd = 2;
  for (int i = 0; i < this->bd_size(); i++) {
    target = ::google::protobuf::internal::WireFormatLite::
      WriteMessageNoVirtualToArray(
        2, this->bd(i), target);
  }

  if (!unknown_fields().empty()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:com.zhichenhaixin.proto.BDList)
  return target;
}

int BDList::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    // required uint64 UTC = 1;
    if (has_utc()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::UInt64Size(
          this->utc());
    }

  }
  // repeated .com.zhichenhaixin.proto.BD bd = 2;
  total_size += 1 * this->bd_size();
  for (int i = 0; i < this->bd_size(); i++) {
    total_size +=
      ::google::protobuf::internal::WireFormatLite::MessageSizeNoVirtual(
        this->bd(i));
  }

  if (!unknown_fields().empty()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void BDList::MergeFrom(const ::google::protobuf::Message& from) {
  GOOGLE_CHECK_NE(&from, this);
  const BDList* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const BDList*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void BDList::MergeFrom(const BDList& from) {
  GOOGLE_CHECK_NE(&from, this);
  bd_.MergeFrom(from.bd_);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_utc()) {
      set_utc(from.utc());
    }
  }
  mutable_unknown_fields()->MergeFrom(from.unknown_fields());
}

void BDList::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void BDList::CopyFrom(const BDList& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool BDList::IsInitialized() const {
  if ((_has_bits_[0] & 0x00000001) != 0x00000001) return false;

  if (!::google::protobuf::internal::AllAreInitialized(this->bd())) return false;
  return true;
}

void BDList::Swap(BDList* other) {
  if (other != this) {
    std::swap(utc_, other->utc_);
    bd_.Swap(&other->bd_);
    std::swap(_has_bits_[0], other->_has_bits_[0]);
    _unknown_fields_.Swap(&other->_unknown_fields_);
    std::swap(_cached_size_, other->_cached_size_);
  }
}

::google::protobuf::Metadata BDList::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = BDList_descriptor_;
  metadata.reflection = BDList_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace proto
}  // namespace zhichenhaixin
}  // namespace com

// @@protoc_insertion_point(global_scope)