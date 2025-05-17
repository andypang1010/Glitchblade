//
//  CUNetcodeSerializer.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for serializing and deserializing information
//  across the network. The class NetcodeConnection can only handle byte arrays.
//  These classes allow you to transfer much more complex data into these byte
//  arrays.
//
//  These classes do not QUITE use our standard shared-pointer architecture. That
//  is because there is no non-trivial constructor patterns and so an init method
//  is not necessary. However, we do still include an alloc method for creating
//  shared pointers.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Authors: Michal Xing and Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
// With help from onewordstudios:
// - Demi Chang
// - Aashna Saxena
// - Sam Sorenson
// - Michael Xing
// - Jeffrey Yao
// - Wendy Zhang
// https://onewordstudios.com/
//
#include <cugl/netcode/CUNetcodeSerializer.h>
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/util/CUEndian.h>
#include <cugl/core/util/CUHashtools.h>

#include <stdexcept>
#include <sstream>

using namespace cugl;
using namespace cugl::netcode;

#pragma mark -
#pragma mark NetcodeSerializer
/**
 * Writes a single boolean value.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param b The value to write
 */
void NetcodeSerializer::writeBool(bool b) {
    _data.push_back(static_cast<std::byte>(b ? BooleanTrue : BooleanFalse));
}

/**
 * Writes a single float value.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param f The value to write
 */
void NetcodeSerializer::writeFloat(float f) {
    float ii = marshall(f);
    const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
    _data.push_back(static_cast<std::byte>(FloatType));
    for (size_t j = 0; j < sizeof(float); j++) {
        _data.push_back(bytes[j]);
    }
}

/**
 * Writes a single double value.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param d The value to write
 */
void NetcodeSerializer::writeDouble(double d) {
    double ii = marshall(d);
    const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
    _data.push_back(static_cast<std::byte>(DoubleType));
    for (size_t j = 0; j < sizeof(double); j++) {
        _data.push_back(bytes[j]);
    }
}

/**
 * Writes a single unsigned (32 bit) int value.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param i The value to write
 */
void NetcodeSerializer::writeUint32(Uint32 i) {
    Uint32 ii = marshall(i);
    const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
    _data.push_back(static_cast<std::byte>(UInt32Type));
    for (size_t j = 0; j < sizeof(Uint32); j++) {
        _data.push_back(bytes[j]);
    }
}

/**
 * Writes a single unsigned (64 bit) int value.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param i The value to write
 */
void NetcodeSerializer::writeUint64(Uint64 i) {
    Uint64 ii = marshall(i);
    const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
    _data.push_back(static_cast<std::byte>(UInt64Type));
    for (size_t j = 0; j < sizeof(Uint64); j++) {
        _data.push_back(bytes[j]);
    }
}

/**
 * Writes a single signed (32 bit) int value.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param i The value to write
 */
void NetcodeSerializer::writeSint32(Sint32 i) {
    Sint32 ii = marshall(i);
    const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
    _data.push_back(static_cast<std::byte>(SInt32Type));
    for (size_t j = 0; j < sizeof(Sint32); j++) {
        _data.push_back(bytes[j]);
    }
}

/**
 * Writes a single signed (64 bit) int value.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param i The value to write
 */
void NetcodeSerializer::writeSint64(Sint64 i) {
    Sint64 ii = marshall(i);
    const std::byte* bytes = reinterpret_cast<const std::byte*>(&ii);
    _data.push_back(static_cast<std::byte>(SInt64Type));
    for (size_t j = 0; j < sizeof(Sint64); j++) {
        _data.push_back(bytes[j]);
    }
}

/**
 * Writes a single string value.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param s The value to write
 */
void NetcodeSerializer::writeString(std::string s) {
    _data.push_back(static_cast<std::byte>(StringType));
    writeUint64((Uint64)(s.size()));
    for (char& c : s) {
        _data.push_back(static_cast<std::byte>(c));
    }
}
/**
 * Writes a single string value.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * Note that this value will be deserialized by {@link NetcodeSerializer} as
 * a `std::string` object.
 *
 * @param s The value to write
 */
void NetcodeSerializer::writeChars(char* s) {
    writeString(std::string(s));
}

/**
 * Writes a single {@link JsonValue}.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param j The value to write
 */
void NetcodeSerializer::writeJson(const std::shared_ptr<JsonValue>& j) {
    _data.push_back(static_cast<std::byte>(JsonType));
    switch (j->type()) {
    case JsonValue::Type::NullType:
        _data.push_back(static_cast<std::byte>(NoneType));
        break;
    case JsonValue::Type::BoolType:
        writeBool(j->asBool());
        break;
    case JsonValue::Type::NumberType:
        writeDouble(j->asDouble());
        break;
    case JsonValue::Type::StringType:
        writeString(j->asString());
        break;
    case JsonValue::Type::ArrayType: {
        _data.push_back(static_cast<std::byte>(ArrayType));
        writeUint64((Uint64)(j->_children.size()));
        for (auto& item : j->_children) {
            writeJson(item);
        }
        break;
    }
    case JsonValue::Type::ObjectType:
        _data.push_back(static_cast<std::byte>(JsonType));
        writeUint64((Uint64)(j->_children.size()));
        for (auto& item : j->_children) {
            writeString(item->key());
            writeJson(item);
        }
        break;
    }
}

/**
 * Writes a vector of boolean values.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param v The vector to write
 */
void NetcodeSerializer::writeBoolVector(std::vector<bool> v) {
    _data.push_back(static_cast<std::byte>(ArrayType + BooleanTrue));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
        writeBool(v[i]);
    }
}

/**
 * Writes a vector of float values.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param v The vector to write
 */
void NetcodeSerializer::writeFloatVector(std::vector<float> v) {
    _data.push_back(static_cast<std::byte>(ArrayType + FloatType));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
        writeFloat(v[i]);
    }
}

/**
 * Writes a vector of double values.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param v The vector to write
 */
void NetcodeSerializer::writeDoubleVector(std::vector<double> v) {
    _data.push_back((std::byte)(ArrayType + DoubleType));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
        writeDouble(v[i]);
    }
}

/**
 * Writes a vector of unsigned (32 bit) int values.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param v The vector to write
 */
void NetcodeSerializer::writeUint32Vector(std::vector<Uint32> v) {
    _data.push_back(static_cast<std::byte>(ArrayType + UInt32Type));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
            writeUint32(v[i]);
    }
}

/**
 * Writes a vector of unsigned (64 bit) int values.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param v The vector to write
 */
void NetcodeSerializer::writeUint64Vector(std::vector<Uint64> v) {
    _data.push_back(static_cast<std::byte>(ArrayType + UInt64Type));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
            writeUint64(v[i]);
    }
}

/**
 * Writes a vector of signed (32 bit) int values.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param v The vector to write
 */void NetcodeSerializer::writeSint32Vector(std::vector<Sint32> v) {
    _data.push_back(static_cast<std::byte>(ArrayType + SInt32Type));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
        writeSint32(v[i]);
    }
}

/**
 * Writes a vector of signed (64 bit) int values.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param v The vector to write
 */
void NetcodeSerializer::writeSint64Vector(std::vector<Sint64> v) {
    _data.push_back(static_cast<std::byte>(ArrayType + SInt64Type));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
        writeSint64(v[i]);
    }
}

/**
 * Writes a vector of string values.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param v The vector to write
 */
void NetcodeSerializer::writeStringVector(std::vector<std::string> v) {
    _data.push_back(static_cast<std::byte>(ArrayType + StringType));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
        writeString(v[i]);
    }
}

/**
 * Writes a vector of string values.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * Note that the vector contents will be deserialized by {@link NetcodeSerializer}
 * as `std::string` objects.
 *
 * @param v The vector to write
 */
void NetcodeSerializer::writeCharsVector(std::vector<char*> v) {
    _data.push_back(static_cast<std::byte>(ArrayType + StringType));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
        writeChars(v[i]);
    }
}

/**
 * Write a vector of {@link JsonValue} objects.
 *
 * Values will be deserialized on other machines in the same order they were
 * written in. You should pass the result of {@link #serialize} to the
 * {@link NetcodeConnection} to send all values buffered up to this point.
 *
 * @param v The vector to write
 */
void NetcodeSerializer::writeJsonVector(std::vector<std::shared_ptr<JsonValue>> v) {
    _data.push_back(static_cast<std::byte>(ArrayType + JsonType));
    writeUint64((Uint64)(v.size()));
    for (size_t i = 0; i < v.size(); i++) {
        writeJson(v[i]);
    }
}

/**
 * Returns a byte vector of all written values suitable for network transit.
 *
 * This method should be called after the appropriate write methods have been
 * called. This provides a vector for network transit. It will need to be
 * deserialized on the other end by {@link NetcodeDeserializer}.
 *
 * You MUST call reset() after this method to clear the input buffer. Otherwise,
 * the next call to this method will still contain all the contents written in
 * this call.
 *
 * The contents of the returned vector should be treated as opaque. You should
 * only read the output via the use of {@link NetcodeDeserializer}.
 *
 * @returns a byte vector of all written values suitable for network transit.
 */
const std::vector<std::byte>& NetcodeSerializer::serialize() {
	return _data;
}

/**
 * Returns a Base 64 representation of all written values.
 *
 * This method should be called after the appropriate write methods have
 * been called. This provides a string for network transit. It will need to
 * be deserialized on the other end by {@link NetcodeDeserializer}.
 *
 * This method differs from {@link #serialize} in that it returns a string
 * of printable characters. In some network settings (e.g. HTTP) this is
 * necessary to submit binary data.
 *
 * You MUST call reset() after this method to clear the input buffer.
 * Otherwise, the next call to this method will still contain all the
 * contents written in this call.
 *
 * The contents of the returned vector should be treated as opaque. You
 * should only read the output via the use of {@link NetcodeDeserializer}.
 *
 * @returns a Base 64 representation of all written values.
 */
const std::string NetcodeSerializer::serialize64() {
    return hashtool::b64_encode(_data);
}

/**
 * Clears the input buffer.
 */
void NetcodeSerializer::reset() {
	_data.clear();
}

#pragma mark -
#pragma mark NetcodeDeserializer

/**
 * Loads a new message to be read.
 *
 * Calling this method will discard any previously loaded messages. The
 * message must be serialized by {@link NetcodeSerializer}. Otherwise,
 * the results are unspecified.
 *
 * Once loaded, call {@link #read} to get a single value (or vector of values).
 * To read all of the data transmitted, call {@link #read} until it returns
 * the monostate. The values are guaranteed to be delievered in the same
 * order they were written.
 *
 * If you are uncomfortable with variants, there are type-specific methods
 * for reading data. However, you will need to call {@link #nextType} to
 * determine the correct method to call.
 *
 * @param msg The byte vector serialized by {@link NetcodeSerializer}
 */
void NetcodeDeserializer::receive(const std::vector<std::byte>& msg) {
	_data = msg;
	_pos = 0;
}

/**
 * Loads a new Base 64 message to be read.
 *
 * Calling this method will discard any previously loaded messages. The
 * message must be serialized by {@link NetcodeSerializer}. Otherwise,
 * the results are unspecified. This method is different from
 * {@link #receive} in that the data is encoded as a Base 64 string rather
 * than a byte vector. See {@link NetcodeSerializer#serialize64}.
 *
 * Once loaded, call {@link #read} to get a single value (or vector of
 * values). To read all of the data transmitted, call {@link #read} until
 * it returns the monostate. The values are guaranteed to be delievered in
 * the same order they were written.
 *
 * If you are uncomfortable with variants, there are type-specific methods
 * for reading data. However, you will need to call {@link #nextType} to
 * determine the correct method to call.
 *
 * @param msg The byte vector serialized by {@link NetcodeSerializer}
 */
void NetcodeDeserializer::receive64(const std::string msg) {
    _data = hashtool::b64_decode(msg);
    _pos = 0;
}

/**
 * Reads the next unreturned value or vector from the currently loaded byte vector.
 *
 * A byte vector should be loaded with the {@link #receive} method. If
 * nothing is loaded, this will return the monostate. This method also
 * advances the read position. If the end of the vector is reached, this
 * returns the monostate.
 *
 * The return type is a variant. You can pattern match on the variant to
 * handle different types. However, if you know what order the values were
 * written in (which you really should), you can use std::get<T>(...) to
 * just assert the next value should be of a certain type T and to extract
 * that value directly. This avoids the overhead of a pattern match on every
 * value. In addition, it is guaranteed to never corrupt the stream (unlike
 * the other read methods).
 */
NetcodeDeserializer::Message NetcodeDeserializer::read() {
	if (_pos >= _data.size()) {
		return {};
	}

    uint8_t value = static_cast<uint8_t>(_data[_pos]);
    switch (value) {
	case NoneType:
		_pos++;
		return {};
	case BooleanTrue:
		_pos++;
		return true;
	case BooleanFalse:
		_pos++;
		return false;
    case FloatType:
        return readFloat();
    case DoubleType:
        return readDouble();
    case UInt32Type:
        return readUint32();
    case UInt64Type:
        return readUint64();
    case SInt32Type:
        return readSint32();
    case SInt64Type:
        return readSint64();
    case StringType:
        return readString();
    case JsonType:
        return readJson();
    case ArrayType+BooleanTrue:
        return readBoolVector();
    case ArrayType+FloatType:
        return readFloatVector();
    case ArrayType+DoubleType:
        return readDoubleVector();
    case ArrayType+UInt32Type:
        return readUint32Vector();
    case ArrayType+UInt64Type:
        return readUint64Vector();
    case ArrayType+SInt32Type:
        return readSint32Vector();
    case ArrayType+SInt64Type:
        return readSint64Vector();
    case ArrayType+StringType:
        return readStringVector();
    case ArrayType+JsonType:
        return readJsonVector();
	default:
		throw std::domain_error("Illegal state of array; did you pass in a valid message?");
	}
}

/**
 * Returns the type of the next data value to be read.
 *
 * This method returns {@link NetcodeType#InvalidType} if the stream is
 * exhausted (nothing left to be read) or corrupted.
 *
 * @return the type of the next data value to be read.
 */
NetcodeType NetcodeDeserializer::nextType() const {
    if (_pos >= _data.size()) {
        return InvalidType;
    }
    
    uint8_t value = static_cast<uint8_t>(_data[_pos]);
    switch (value) {
    case NoneType:
    case BooleanTrue:
    case BooleanFalse:
    case FloatType:
    case DoubleType:
    case UInt32Type:
    case UInt64Type:
    case SInt32Type:
    case SInt64Type:
    case StringType:
    case JsonType:
    case ArrayType+BooleanTrue:
    case ArrayType+FloatType:
    case ArrayType+DoubleType:
    case ArrayType+UInt32Type:
    case ArrayType+UInt64Type:
    case ArrayType+SInt32Type:
    case ArrayType+SInt64Type:
    case ArrayType+StringType:
    case ArrayType+JsonType:
        return (NetcodeType)value;
    default:
        return InvalidType;
    }
}

/**
 * Returns a single boolean value.
 *
 * This method is only defined if {@link #nextType} has returned either 
 * BooleanTrue or BooleanFalse. Otherwise, calling this method will
 * potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return false.
 *
 * @return a single boolean value.
 */
bool NetcodeDeserializer::readBool() {
    if (_pos >= _data.size()) {
        return false;
    }
    uint8_t value = static_cast<uint8_t>(_data[_pos++]);
    return value == BooleanTrue;
}

/**
 * Returns a single float value.
 *
 * This method is only defined if {@link #nextType} has returned FloatType.
 * Otherwise, calling this method will potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a single float value.
 */
float NetcodeDeserializer::readFloat() {
    if (_pos >= _data.size()) {
        return 0.0f;
    }
    _pos++;
    
    // Memcopy necessary for possible alignment issues
    float value;
    std::memcpy(&value, _data.data() + _pos, sizeof(float));
    _pos += sizeof(float);
    return marshall(value);
}

/**
 * Returns a single double value.
 *
 * This method is only defined if {@link #nextType} has returned DoubleType.
 * Otherwise, calling this method will potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a single double value.
 */
double NetcodeDeserializer::readDouble() {
    if (_pos >= _data.size()) {
        return 0.0;
    }
    _pos++;
    
    // Memcopy necessary for possible alignment issues
    double value;
    std::memcpy(&value, _data.data() + _pos, sizeof(double));
    _pos += sizeof(double);
    return marshall(value);
}

/**
 * Returns a single unsigned (32 bit) int value.
 *
 * This method is only defined if {@link #nextType} has returned UInt32Type.
 * Otherwise, calling this method will potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a single unsigned (32 bit) int value.
 */
Uint32 NetcodeDeserializer::readUint32() {
    if (_pos >= _data.size()) {
        return 0;
    }
    _pos++;
    
    // Memcopy necessary for possible alignment issues
    Uint32 value;
    std::memcpy(&value, _data.data() + _pos, sizeof(Uint32));
    _pos += sizeof(Uint32);
    return marshall(value);
}

/**
 * Returns a single signed (32 bit) int value.
 *
 * This method is only defined if {@link #nextType} has returned SInt32Type.
 * Otherwise, calling this method will potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a single signed (32 bit) int value.
 */
Sint32 NetcodeDeserializer::readSint32() {
    if (_pos >= _data.size()) {
        return 0;
    }
    _pos++;
    
    // Memcopy necessary for possible alignment issues
    Sint32 value;
    std::memcpy(&value, _data.data() + _pos, sizeof(Sint32));
    _pos += sizeof(Sint32);
    return marshall(value);
}

/**
 * Returns a single unsigned (64 bit) int value.
 *
 * This method is only defined if {@link #nextType} has returned UInt64Type.
 * Otherwise, calling this method will potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a single unsigned (64 bit) int value.
 */Uint64 NetcodeDeserializer::readUint64() {
    if (_pos >= _data.size()) {
        return 0;
    }
    _pos++;
    
    // Memcopy necessary for possible alignment issues
    Uint64 value;
    std::memcpy(&value, _data.data() + _pos, sizeof(Uint64));
    _pos += sizeof(Uint64);
    return marshall(value);
}

/**
 * Returns a single signed (64 bit) int value.
 *
 * This method is only defined if {@link #nextType} has returned SInt64Type.
 * Otherwise, calling this method will potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a single signed (64 bit) int value.
 */
Sint64 NetcodeDeserializer::readSint64() {
    if (_pos >= _data.size()) {
        return 0;
    }
    _pos++;
    
    // Memcopy necessary for possible alignment issues
    Sint64 value;
    std::memcpy(&value, _data.data() + _pos, sizeof(Sint64));
    _pos += sizeof(Sint64);
    return marshall(value);
}

/**
 * Returns a single string.
 *
 * This method is only defined if {@link #nextType} has returned StringType.
 * Otherwise, calling this method will potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return the empty string.
 *
 * @return a single string.
 */
std::string NetcodeDeserializer::readString() {
    if (_pos >= _data.size()) {
        return std::string();
    }
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    std::ostringstream disp;
    for (size_t i = 0; i < size; i++, _pos++) {
        disp << static_cast<char>(_data[_pos]);
    }
    return disp.str();
}

/**
 * Returns a single {@link JsonValue} object.
 *
 * This method is only defined if {@link #nextType} has returned JsonType.
 * Otherwise, calling this method will potentially corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return nullptr.
 *
 * @return a single {@link JsonValue} object.
 */
std::shared_ptr<JsonValue> NetcodeDeserializer::readJson() {
    if (_pos >= _data.size()) {
        return nullptr;
    }
    _pos++;
    uint8_t value = static_cast<uint8_t>(_data[_pos]);
    switch (value) {
    case NoneType:
        _pos++;
        return JsonValue::allocNull();
    case BooleanTrue:
        _pos++;
        return JsonValue::alloc(true);
    case BooleanFalse:
        _pos++;
        return JsonValue::alloc(false);
    case DoubleType:
        return JsonValue::alloc(std::get<double>(read()));
    case StringType:
        return JsonValue::alloc(std::get<std::string>(read()));
    case ArrayType: {
        auto ret = JsonValue::allocArray();
        _pos++;
        Uint64 size = std::get<Uint64>(read());
        for (size_t ii = 0; ii < size; ii++) {
            ret->appendChild(std::get<std::shared_ptr<JsonValue>>(read()));
        }
        return ret;
    }
    case JsonType: {
        auto ret = JsonValue::allocObject();
        _pos++;
        Uint64 size = std::get<Uint64>(read());
        for (size_t ii = 0; ii < size; ii++) {
            std::string key = std::get<std::string>(read());
            ret->appendChild(key, std::get<std::shared_ptr<JsonValue>>(read()));
        }
        return ret;
    }
    default:
        throw std::domain_error("Illegal json");
    }
}

/**
 * Returns a vector of boolean values.
 *
 * This method is only defined if {@link #nextType} has returned
 * (ArrayType+BooleanTrue). Otherwise, calling this method will potentially
 * corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return false.
 *
 * @return a vector of boolean values.
 */
std::vector<bool> NetcodeDeserializer::readBoolVector()  {
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    std::vector<bool> vv;
    for (size_t i = 0; i < size; i++) {
        vv.push_back(std::get<bool>(read()));
    }
    return vv;
}

/**
 * Returns a vector of float values.
 *
 * This method is only defined if {@link #nextType} has returned
 * (ArrayType+FloatType). Otherwise, calling this method will potentially
 * corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a vector of float values.
 */
std::vector<float> NetcodeDeserializer::readFloatVector() {
    std::vector<float> vv;
    if (_pos >= _data.size()) {
        return vv;
    }
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    for (size_t i = 0; i < size; i++) {
        vv.push_back(std::get<float>(read()));
    }
    return vv;
}

/**
 * Returns a vector of double values.
 *
 * This method is only defined if {@link #nextType} has returned
 * (ArrayType+DoubleType). Otherwise, calling this method will potentially
 * corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a vector of double values.
 */
std::vector<double> NetcodeDeserializer::readDoubleVector()     {
    std::vector<double> vv;
    if (_pos >= _data.size()) {
        return vv;
    }
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    for (size_t i = 0; i < size; i++) {
        vv.push_back(std::get<double>(read()));
    }
    return vv;
}

/**
 * Returns a vector of unsigned (32 bit) int values.
 *
 * This method is only defined if {@link #nextType} has returned
 * (ArrayType+UInt32Type). Otherwise, calling this method will potentially
 * corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a vector of unsigned (32 bit) int values.
 */
std::vector<Uint32> NetcodeDeserializer::readUint32Vector() {
    std::vector<Uint32> vv;
    if (_pos >= _data.size()) {
        return vv;
    }
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    for (size_t i = 0; i < size; i++) {
        vv.push_back(std::get<Uint32>(read()));
    }
    return vv;
}

/**
 * Returns a vector of signed (32 bit) int values.
 *
 * This method is only defined if {@link #nextType} has returned
 * (ArrayType+SInt32Type). Otherwise, calling this method will potentially
 * corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a vector of signed (32 bit) int values.
 */
std::vector<Sint32> NetcodeDeserializer::readSint32Vector()  {
    std::vector<Sint32> vv;
    if (_pos >= _data.size()) {
        return vv;
    }
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    for (size_t i = 0; i < size; i++) {
        vv.push_back(std::get<Sint32>(read()));
    }
    return vv;
}

/**
 * Returns a vector of unsigned (64 bit) int values.
 *
 * This method is only defined if {@link #nextType} has returned
 * (ArrayType+UInt64Type). Otherwise, calling this method will potentially
 * corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a vector of unsigned (64 bit) int values.
 */
std::vector<Uint64> NetcodeDeserializer::readUint64Vector() {
    std::vector<Uint64> vv;
    if (_pos >= _data.size()) {
        return vv;
    }
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    for (size_t i = 0; i < size; i++) {
        vv.push_back(std::get<Uint64>(read()));
    }
    return vv;
}

/**
 * Returns a vector of signed (64 bit) int values.
 *
 * This method is only defined if {@link #nextType} has returned
 * (ArrayType+SInt64Type). Otherwise, calling this method will potentially
 * corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return 0.
 *
 * @return a vector of signed (64 bit) int values.
 */
std::vector<Sint64> NetcodeDeserializer::readSint64Vector() {
    std::vector<Sint64> vv;
    if (_pos >= _data.size()) {
        return vv;
    }
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    for (size_t i = 0; i < size; i++) {
        vv.push_back(std::get<Sint64>(read()));
    }
    return vv;
}

/**
 * Returns a vector of strings.
 *
 * This method is only defined if {@link #nextType} has returned
 * (ArrayType+StringType). Otherwise, calling this method will potentially
 * corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return the empty string.
 *
 * @return a vector of strings.
 */
std::vector<std::string> NetcodeDeserializer::readStringVector() {
    std::vector<std::string> vv;
    if (_pos >= _data.size()) {
        return vv;
    }
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    for (size_t i = 0; i < size; i++) {
        vv.push_back(std::get<std::string>(read()));
    }
    return vv;
}

/**
 * Returns a vector of {@link JsonValue} objects.
 *
 * This method is only defined if {@link #nextType} has returned
 * (ArrayType+JsonType). Otherwise, calling this method will potentially
 * corrupt the stream.
 *
 * The method advances the read position. If called when no more data is
 * available, this method will return nullptr.
 *
 * @return a vector of {@link JsonValue} objects.
 */
std::vector<std::shared_ptr<JsonValue>> NetcodeDeserializer::readJsonVector() {
    std::vector<std::shared_ptr<JsonValue>> vv;
    if (_pos >= _data.size()) {
        return vv;
    }
    _pos++;
    Uint64 size = std::get<Uint64>(read());
    for (size_t i = 0; i < size; i++) {
        vv.push_back(std::get<std::shared_ptr<JsonValue>>(read()));
    }
    return vv;
}

/**
 * Clears the buffer and ignore any remaining data in it.
 */
void NetcodeDeserializer::reset() {
	_pos = 0;
	_data.clear();
}
