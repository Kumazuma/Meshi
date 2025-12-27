// CdrLibrary.cpp : 애플리케이션의 진입점을 정의합니다.
//

#include "CdrLibrary.h"
#include <array>
#include <string>
#include <vector>
#include <optional>
#include <bit>
#include <span>
#include <map>
#include <stack>

#include "xtypes.h"

// For MD5 Hash, We uses Windows Crypto API
#if defined(_WIN32)
#include <Windows.h>
#include <Wincrypt.h>
#endif

using namespace std;
using namespace meshi::dds;
using namespace meshi::dds::xtypes;

class ICdrHandler
{
public:
	virtual bool Null() = 0;
	virtual bool Bool(bool) = 0;
	virtual bool Octet(uint8_t) = 0;
	virtual bool Short(int16_t) = 0;
	virtual bool Long(int32_t) = 0;
	virtual bool LongLong(int64_t) = 0;
	virtual bool UnsignedShort(uint16_t) = 0;
	virtual bool UnsignedLong(uint32_t) = 0;
	virtual bool UnsignedLongLong(uint64_t) = 0;
	virtual bool Float(float) = 0;
	virtual bool Double(double) = 0;
	virtual bool Char8(char) = 0;
	virtual bool Char16(char16_t) = 0;
	virtual bool String8(const std::string&) = 0;
	virtual bool String16(const std::u16string&) = 0;
	virtual bool Key(uint16_t id) = 0;
	virtual bool BeginStructure() = 0;
	virtual bool EndStructure() = 0;
	virtual bool StartArray(uint32_t) = 0;
	virtual bool EndArray() = 0;
	virtual bool BeginSequence(uint32_t) = 0;
	virtual bool EndSequence(uint32_t) = 0;
};

class ICdrWriter
{
public:
	virtual void WriteCdrHeader() = 0;
	virtual bool BeginOptional(meshi::dds::xtypes::MemberId id, bool none) = 0;
	virtual bool EndOptional() = 0;
	virtual bool Bool(bool) = 0;
	virtual bool Octet(uint8_t) = 0;
	virtual bool Short(int16_t) = 0;
	virtual bool Long(int32_t) = 0;
	virtual bool LongLong(int64_t) = 0;
	virtual bool UnsignedShort(uint16_t) = 0;
	virtual bool UnsignedLong(uint32_t) = 0;
	virtual bool UnsignedLongLong(uint64_t) = 0;
	virtual bool Float(float) = 0;
	virtual bool Double(double) = 0;
	virtual bool Char8(char) = 0;
	virtual bool Char16(char16_t) = 0;
	virtual bool String8(const std::string&) = 0;
	virtual bool String16(const std::u16string&) = 0;
	virtual bool BeginAggregated(meshi::dds::xtypes::Extensibility extensibility = meshi::dds::xtypes::Extensibility::Final) = 0;
	virtual bool EndAggregated() = 0;
	virtual bool BeginMember(meshi::dds::xtypes::MemberId id, bool mustUnderstand, uint8_t lengthCode) = 0;
	virtual bool EndMember() = 0;
	virtual bool BeginDiscriminator(uint8_t lengthCode) = 0;
	virtual bool EndDiscriminator() = 0;

	virtual bool BeginSequence(uint32_t count, bool nonFinalType = true) = 0;
	virtual bool EndSequence() = 0;

	virtual bool BeginArray(bool nonFinalType = true) = 0;
	virtual bool EndArray() = 0;

};

enum CdrWriterRole
{
	CDR1WRITER_ROLE_SERIALIZATION,
	CDR1WRITER_ROLE_CALC_SIZE
};

template<uint32_t VERSION, CdrWriterRole ROLE, std::endian ENDIAN = std::endian::native>
class CdrCommon : public ICdrWriter
{
public:
	enum AlignKind
	{
		ALIGN_1,
		ALIGN_2,
		ALIGN_4,
		ALIGN_8
	};

	CdrCommon()
	{
		m_buffer = nullptr;
		m_orgin = 0;
		m_length = 0;
		m_offset = 0;
		if constexpr (ROLE == CDR1WRITER_ROLE_CALC_SIZE)
		{
			m_length = UINT32_MAX;
		}
	}

	CdrCommon(uint8_t* buffer, size_t length)
	{
		m_buffer = buffer;
		m_length = length;
		m_orgin = 0;
		m_offset = 0;
	}

	template<typename T>
	void Write(uint32_t offset, T data)
	{
		if constexpr (ROLE == CDR1WRITER_ROLE_SERIALIZATION)
		{
			if constexpr (ENDIAN == std::endian::native || sizeof(T) == 1)
			{
				*reinterpret_cast<T*>(m_buffer + offset) = data;
			}
			else
			{
				static_assert(sizeof(T) <= 8 || sizeof(T) % 2 == 0, "Unsupported Primitive Type");
				if constexpr (sizeof(T) == 8)
				{
					(m_buffer + offset)[0] = (std::bit_cast<uint64_t>(data) >> 00) & 0xFF;
					(m_buffer + offset)[1] = (std::bit_cast<uint64_t>(data) >> 010) & 0xFF; // 8
					(m_buffer + offset)[2] = (std::bit_cast<uint64_t>(data) >> 020) & 0xFF; // 16
					(m_buffer + offset)[3] = (std::bit_cast<uint64_t>(data) >> 030) & 0xFF; // 24
					(m_buffer + offset)[4] = (std::bit_cast<uint64_t>(data) >> 040) & 0xFF; // 32
					(m_buffer + offset)[5] = (std::bit_cast<uint64_t>(data) >> 050) & 0xFF; // 40
					(m_buffer + offset)[6] = (std::bit_cast<uint64_t>(data) >> 060) & 0xFF; // 48
					(m_buffer + offset)[7] = (std::bit_cast<uint64_t>(data) >> 070) & 0xFF; // 56
				}
				else if constexpr (sizeof(T) == 4)
				{
					(m_buffer + offset)[0] = (std::bit_cast<uint32_t>(data) >> 00) & 0xFF;
					(m_buffer + offset)[1] = (std::bit_cast<uint32_t>(data) >> 010) & 0xFF; // 8
					(m_buffer + offset)[2] = (std::bit_cast<uint32_t>(data) >> 020) & 0xFF; // 16
					(m_buffer + offset)[3] = (std::bit_cast<uint32_t>(data) >> 030) & 0xFF; // 24
				}
				else if constexpr (sizeof(T) == 2)
				{
					(m_buffer + offset)[0] = (std::bit_cast<uint16_t>(data) >> 00) & 0xFF;
					(m_buffer + offset)[1] = (std::bit_cast<uint16_t>(data) >> 010) & 0xFF; // 8
				}
			}
		}
	}

	template<typename T>
	void Write(T data)
	{
		Write(m_offset, data);
	}

	template<typename T>
	void MemCpy(const T* data, size_t length)
	{
		if constexpr (ROLE == CDR1WRITER_ROLE_SERIALIZATION)
		{
			if constexpr (ENDIAN == std::endian::native || sizeof(T) == 1)
			{
				memcpy(m_buffer + m_offset, data, length);
			}
			else
			{
				static_assert(sizeof(T) <= 8 || sizeof(T) % 2 == 0, "Unsupported Primitive Type");
				size_t index = 0;
				for (size_t i = 0; i < length; ++i)
				{
					T item = data[i];
					if constexpr (sizeof(T) == 8)
					{
						(m_buffer + index)[0] = (std::bit_cast<uint64_t>(item) >> 00) & 0xFF;
						(m_buffer + index)[1] = (std::bit_cast<uint64_t>(item) >> 010) & 0xFF; // 8
						(m_buffer + index)[2] = (std::bit_cast<uint64_t>(item) >> 020) & 0xFF; // 16
						(m_buffer + index)[3] = (std::bit_cast<uint64_t>(item) >> 030) & 0xFF; // 24
						(m_buffer + index)[4] = (std::bit_cast<uint64_t>(item) >> 040) & 0xFF; // 32
						(m_buffer + index)[5] = (std::bit_cast<uint64_t>(item) >> 050) & 0xFF; // 40
						(m_buffer + index)[6] = (std::bit_cast<uint64_t>(item) >> 060) & 0xFF; // 48
						(m_buffer + index)[7] = (std::bit_cast<uint64_t>(item) >> 070) & 0xFF; // 56
					}
					else if constexpr (sizeof(T) == 4)
					{
						(m_buffer + index)[0] = (std::bit_cast<uint32_t>(item) >> 00) & 0xFF;
						(m_buffer + index)[1] = (std::bit_cast<uint32_t>(item) >> 010) & 0xFF; // 8
						(m_buffer + index)[2] = (std::bit_cast<uint32_t>(item) >> 020) & 0xFF; // 16
						(m_buffer + index)[3] = (std::bit_cast<uint32_t>(item) >> 030) & 0xFF; // 24
					}
					else if constexpr (sizeof(T) == 2)
					{
						(m_buffer + index)[0] = (std::bit_cast<uint16_t>(item) >> 00) & 0xFF;
						(m_buffer + index)[1] = (std::bit_cast<uint16_t>(item) >> 010) & 0xFF; // 8
					}

					index += sizeof(T);
				}
			}
		}
	}

	bool BeginOptional(uint64_t id, uint32_t length)
	{
		// Optional : write PL header
		// Write PL Header for optional field
		// PL header format:
		// - 2 bytes: member id (uint16_t)
		// - 2 bytes: EMHEADER (length in bytes, uint16_t)
		// Total: 4 bytes for PL header

		Align(4);  // PL header requires 4-byte alignment

		if (m_length < m_offset + 4)
			return false;

		if constexpr (ROLE == CDR1WRITER_ROLE_SERIALIZATION)
		{
			// Clear PL header area
			memset(m_buffer + m_offset, 0, 4);
		}

		// Write member id (lower 16 bits of id)
		Write<uint16_t>(static_cast<uint16_t>(id));
		m_offset += 2;

		// Write EMHEADER (length of the parameter value)
		Write<uint16_t>(static_cast<uint16_t>(length));
		m_offset += 2;

		return true;
	}

	bool EndOptional()
	{
		return true;
	}

	bool Bool(bool value)
	{
		if (m_length < m_offset + 1)
			return false;

		Write<uint8_t>(value ? 1 : 0);
		Advance(1);
		return true;
	}

	bool Octet(uint8_t value)
	{
		if (m_length < m_offset + 1)
			return false;

		Write<uint8_t>(value);
		Advance(1);
		return true;
	}

	bool Short(int16_t value)
	{
		TmplAlign<ALIGN_2>();
		if (m_length < m_offset + 2)
			return false;

		Write<int16_t>(value);
		Advance(2);
		return true;
	}

	bool Long(int32_t value)
	{
		TmplAlign<ALIGN_4>();
		if (m_length < m_offset + 4)
			return false;

		Write<int32_t>(value);
		Advance(4);
		return true;
	}

	bool LongLong(int64_t value)
	{
		TmplAlign<ALIGN_8>();
		if (m_length < m_offset + 8)
			return false;

		Write<int64_t>(value);
		Advance(8);
		return true;
	}

	bool UnsignedShort(uint16_t value)
	{
		TmplAlign<ALIGN_2>();
		if (m_length < m_offset + 2)
			return false;

		Write<uint16_t>(value);
		Advance(2);
		return true;

	}

	bool UnsignedLong(uint32_t value)
	{
		TmplAlign<ALIGN_4>();
		if (m_length < m_offset + 4)
			return false;

		Write<uint32_t>(value);
		Advance(4);
		return true;
	}

	bool UnsignedLongLong(uint64_t value)
	{
		TmplAlign<ALIGN_8>();
		if (m_length < m_offset + 8)
			return false;

		Write<uint64_t>(value);
		Advance(8);
		return true;
	}

	bool Float(float value)
	{
		TmplAlign<ALIGN_4>();
		if (m_length < m_offset + 4)
			return false;

		Write<float>(value);
		Advance(4);
		return true;
	}

	bool Double(double value)
	{
		TmplAlign<ALIGN_8>();
		if (m_length < m_offset + 8)
			return false;

		Write<double>(value);
		Advance(8);
		return true;
	}

	bool Char8(char value)
	{
		if (m_length < m_offset + 1)
			return false;

		Write<uint8_t>(static_cast<uint8_t>(value));
		Advance(1);
		return true;
	}

	bool Char16(char16_t value)
	{
		TmplAlign<ALIGN_2>();
		if (m_length < m_offset + 2)
			return false;

		Write<char16_t>(value);
		Advance(2);
		return true;
	}

	bool String8(const std::string& value)
	{
		uint32_t strLength = static_cast<uint32_t>(value.length()) + 1;
		TmplAlign<ALIGN_4>();
		if (m_length < m_offset + 4 + strLength)
			return false;

		UnsignedLong(strLength);
		MemCpy(value.c_str(), strLength);
		Advance(strLength);
		return true;
	}

	bool String16(const std::u16string& value)
	{
		uint32_t strLength = static_cast<uint32_t>(value.length());
		TmplAlign<ALIGN_4>();
		if (m_length < m_offset + 4 + strLength * 2)
			return false;

		if (!UnsignedLong(strLength))
		{
			return false;
		}

		MemCpy(value.c_str(), strLength * 2);
		Advance(strLength * 2);
		return true;
	}

	void Align(uint32_t alignment)
	{
		if constexpr (VERSION == 2)
		{
			if (alignment == 8)
			{
				// XCDR2 Max Alignment is 4 bytes
				alignment = 4;
			}
		}

		uint32_t padding = (alignment - ((m_offset - m_orgin) % alignment)) % alignment;
		m_offset += padding;
	}

	template<AlignKind TALIGN>
	void TmplAlign()
	{
		uint32_t alignment = 1;
		switch (TALIGN)
		{
		case ALIGN_2:
			alignment = 2;
			break;
		case ALIGN_4:
			alignment = 4;
			break;
		case ALIGN_8:
			if constexpr (VERSION == 1)
			{
				alignment = 8;
			}
			else if constexpr (VERSION == 2)
			{
				alignment = 4;
			}

			break;
			
		}

		uint32_t padding = (alignment - ((m_offset - m_orgin) % alignment)) % alignment;
		m_offset += padding;
	}

	void Advance(uint32_t size)
	{
		m_offset += size;
	}

	uint8_t* m_buffer;
	uint32_t m_length;
	uint32_t m_orgin;
	uint32_t m_offset;

protected:

};

template<CdrWriterRole ROLE, std::endian ENDIAN = std::endian::native>
class Cdr2Writer : public CdrCommon<2, ROLE, ENDIAN>
{
	using Base = CdrCommon<2, ROLE, ENDIAN>;
public:
	enum class EncodingFormat : uint8_t
	{
		Plain = 0x06,
		Delimited = 0x08,
		Pl = 0x0A
	};
	
	using CdrCommon<2, ROLE, ENDIAN>::CdrCommon;

	void WriteCdrHeader() override
	{
		if constexpr (ROLE == CDR1WRITER_ROLE_SERIALIZATION)
		{
			// CDR1 Header
			// - 1 byte: options
			// - 3 bytes: unused, set to 0
			Base::m_buffer[0] = 0x00; // options
			Base::m_buffer[1] = (ENDIAN == std::endian::little ? 1 : 0); // endianness: little endian
			Base::m_buffer[2] = 0x00; // unused
			Base::m_buffer[3] = 0x00; // unused
		}

		Base::m_orgin = 4;
		Base::m_offset += 4;
	}

	bool BeginAggregated(meshi::dds::xtypes::Extensibility extensibility) override
	{
		EncodingFormat format = EncodingFormat::Plain;
		switch (extensibility)
		{
		case Extensibility::Final:
			format = EncodingFormat::Plain;
			break;
		case Extensibility::Appendable:
			format = EncodingFormat::Delimited;
			break;
		case Extensibility::Mutable:
			format = EncodingFormat::Pl;
			break;
		default:
			return false;
		}

		if (m_encodingFormatStack.empty())
		{
			if constexpr (ROLE == CDR1WRITER_ROLE_SERIALIZATION) {
				Base::m_buffer[1] += static_cast<uint8_t>(format);
			}
		}

		switch (format)
		{
		case EncodingFormat::Plain:
			// DO NOTHING
			break;
		case EncodingFormat::Delimited:
			// Append Delimiter header
			Base::Align(4);
			Base::Advance(4);
			m_delimiterStack.push(Base::m_offset);
			break;

		case EncodingFormat::Pl:
			// TODO: Not implmented yet!
			Base::Align(4);
			Base::Advance(4);
			m_delimiterStack.push(Base::m_offset);
			break;
		}

		m_encodingFormatStack.push(format);
		return true;
	}

	bool EndAggregated() override
	{
		if (m_encodingFormatStack.empty())
		{
			return false;
		}

		auto format = m_encodingFormatStack.top();
		m_encodingFormatStack.pop();

		if (format == EncodingFormat::Plain)
		{
			return true;
		}

		if (format == EncodingFormat::Delimited)
		{
			auto offset = m_delimiterStack.top();
			m_delimiterStack.pop();
			uint32_t delimiterHeader;
			uint32_t size = Base::m_offset - offset;
			delimiterHeader = size;
			delimiterHeader = delimiterHeader | ((ENDIAN == std::endian::little) ? (1 << 31) : 0);
			Base::Write(offset - 4, delimiterHeader);
		}

		if (format == EncodingFormat::Pl)
		{
			// Append PID_LIST_END

			auto offset = m_delimiterStack.top();
			m_delimiterStack.pop();
			uint32_t delimiterHeader;
			uint32_t size = Base::m_offset - offset;
			delimiterHeader = size;
			delimiterHeader = delimiterHeader | ((ENDIAN == std::endian::little) ? (1 << 31) : 0);
			Base::Write(offset - 4, delimiterHeader);

			// TODO: Not implemented yet!
		}

		return true;
	}

	bool BeginMember(MemberId id, bool mustUnderstand, uint8_t lengthCodeLevel) override
	{
		if (m_encodingFormatStack.top() != EncodingFormat::Pl)
		{
			return true;
		}

		if (lengthCodeLevel == 0 || lengthCodeLevel > 7)
		{
			return false;
		}

		uint32_t memberHeader = mustUnderstand ? (1 << 31) : 0;
		memberHeader += lengthCodeLevel << 28;
		memberHeader += id;

		if (!Base::UnsignedLong(memberHeader))
		{
			return false;
		}

		if (lengthCodeLevel == 4)
		{
			Base::Advance(4);
		}

		m_memberStack.emplace(id, Base::m_offset, lengthCodeLevel);

		return true;
	}

	bool EndMember() override
	{
		if (m_encodingFormatStack.top() != EncodingFormat::Pl)
		{
			return true;
		}

		auto member = m_memberStack.top();
		m_memberStack.pop();

		if (std::get<2>(member) == 4)
		{
			auto offset = std::get<1>(member);
			auto size = Base::m_offset - offset;
			Base::Write(offset - 4, size);
		}

		return true;
	}

	bool BeginDiscriminator(uint8_t lengthCodeLevel) override
	{
		if (m_encodingFormatStack.top() != EncodingFormat::Pl)
		{
			return true;
		}

		if (lengthCodeLevel == 0 || lengthCodeLevel > 7)
		{
			return false;
		}

		uint32_t memberHeader = (1 << 31);
		memberHeader += lengthCodeLevel << 28;

		if (!Base::UnsignedLong(memberHeader))
		{
			return false;
		}

		if (lengthCodeLevel == 4)
		{
			Base::UnsignedLong(0);
		}

		m_memberStack.emplace(0, Base::m_offset, lengthCodeLevel);
		return true;
	}

	bool EndDiscriminator() override
	{
		if (m_encodingFormatStack.top() != EncodingFormat::Pl)
		{
			return true;
		}

		auto member = m_memberStack.top();
		m_memberStack.pop();

		if (std::get<2>(member) == 4)
		{
			auto offset = std::get<1>(member);
			auto size = Base::m_offset - offset;
			Base::Write(offset - 4, size);
		}

		return true;
	}

	bool BeginSequence(uint32_t numElement, bool nonFinalType)
	{
		// Reserve Deheader
		uint32_t offset = 0;;
		if (nonFinalType)
		{
			Base::UnsignedLong(0);
			offset = Base::m_offset;
		}
		
		if (!Base::UnsignedLong(numElement))
		{
			return false;
		}

		m_delimiterStack.push(offset);
		return true;
	}

	bool EndSequence() override
	{
		if (m_delimiterStack.empty())
		{
			return false;
		}

		auto offset = m_delimiterStack.top();
		
		m_delimiterStack.pop();

		if (offset != 0)
		{
			auto size = Base::m_offset - offset;
			size += (ENDIAN == std::endian::little) ? (1 << 31) : 0;
			Base::Write(offset - 4, size);
		}

		return true;
	}

	bool BeginArray(bool nonFinalType = true) override
	{
		// Reserve Deheader
		uint32_t offset = 0;
		if (!nonFinalType)
		{
			if (!Base::UnsignedLong(0))
			{
				return false;
			}

			offset = Base::m_offset;
		}

		m_delimiterStack.push(offset);
		return true;
	}

	bool EndArray() override
	{
		if (m_delimiterStack.empty())
		{
			return false;
		}

		auto offset = m_delimiterStack.top();

		m_delimiterStack.pop();

		if (offset == 0)
		{
			auto size = Base::m_offset - offset;
			size += (ENDIAN == std::endian::little) ? (1 << 31) : 0;
			Base::Write(offset - 4, size);
		}

		return true;
	}

	bool BeginOptional(MemberId id, bool none) override
	{
		if (m_encodingFormatStack.empty())
		{
			return false;
		}

		auto encoding = m_encodingFormatStack.top();
		if (encoding == EncodingFormat::Plain || encoding == EncodingFormat::Pl)
		{
			if (!Base::Bool(!none))
			{
				return false;
			}
		}

		return true;
	}

	bool EndOptional() override
	{
		return true;
	}

	std::stack<EncodingFormat> m_encodingFormatStack;
	std::stack<uint32_t> m_delimiterStack;
	std::stack<std::tuple<MemberId, uint32_t, uint8_t>> m_memberStack;
};


namespace Hello {
	struct Msg
	{
		Long userId;
		std::string message;
		uint8_t tmp;
	};

	struct Msg2
	{
		Msg obj;
		std::string message;
	};

	const uint32_t MSG_USER_ID = offsetof(Msg, userId);
	const uint32_t MSG_MESSAGE = offsetof(Msg, message);
	const uint32_t MSG2_OBJ = offsetof(Msg2, obj);
	const uint32_t MSG2_MESSAGE = offsetof(Msg2, message);
}



bool SerializePlainCollectionHeader(ICdrWriter* writer, const meshi::dds::xtypes::PlainCollectionHeader& header)
{
	if (!writer->BeginAggregated())
	{
		return false;
	}

	writer->Octet(header.equivKind);
	writer->UnsignedShort(header.elementFlags.value);

	return writer->EndAggregated();
}


template<CdrWriterRole ROLE, std::endian ENDIAN = std::endian::native>
class Cdr1Writer : public CdrCommon<1, ROLE, ENDIAN>
{
	using Base = CdrCommon<1, ROLE, ENDIAN>;
public:
	using CdrCommon<1, ROLE, ENDIAN>::CdrCommon;

	void WriteCdrHeader() override
	{
		if constexpr (ROLE == CDR1WRITER_ROLE_SERIALIZATION)
		{
			// CDR1 Header
			// - 1 byte: options
			// - 3 bytes: unused, set to 0
			Base::m_buffer[0] = 0x00; // options
			Base::m_buffer[1] = (ENDIAN == std::endian::little ? 1 : 0); // endianness: little endian
			Base::m_buffer[2] = 0x00; // unused
			Base::m_buffer[3] = 0x00; // unused
		}

		Base::m_orgin = 4;
		Base::m_offset += 4;
	}

	bool BeginAggregated(Extensibility extensibility) override
	{
		return true;
	}

	bool EndAggregated() override
	{
		return true;
	}

	bool BeginMember(MemberId id, bool mustUnderstand, uint8_t lengthCodeLevel) override
	{
		return true;
	}

	bool EndMember() override
	{
		return true;
	}

	bool BeginDiscriminator(uint8_t lengthCodeLevel) override
	{
		return true;
	}

	bool EndDiscriminator() override
	{
		return true;
	}

	bool BeginSequence(uint32_t numElement, bool nonFinalType) override
	{
		if (!Base::UnsignedLong(numElement))
		{
			return false;
		}

		return true;
	}

	bool EndSequence() override
	{
		return true;
	}

	bool BeginArray(bool nonFinalType = true) override
	{
		return true;
	}

	bool EndArray() override
	{
		return true;
	}

	bool BeginOptional(MemberId id, bool none) override
	{
		return true;
	}

	bool EndOptional()
	{
		return true;
	}
};

template<typename T>
struct SequenceElementSerializerTrait {
	bool operator()(ICdrWriter* writer, const T& element) { return false; }
};

template<typename T>
bool SerializeElement(ICdrWriter* writer, const T& element)
{
	if constexpr (std::is_same_v<T, uint8_t>)
	{
		return writer->Octet(element);
	}
	else if constexpr (std::is_same_v<T, bool>)
	{
		return writer->Bool(element);
	}
	else if constexpr (std::is_same_v<T, uint16_t>)
	{
		return writer->UnsignedShort(element);
	}
	else if constexpr (std::is_same_v<T, int16_t>)
	{
		return writer->Short(element);
	}
	else if constexpr (std::is_same_v<T, int32_t>)
	{
		return writer->Long(element);
	}
	else if constexpr (std::is_same_v<T, uint32_t>)
	{
		return writer->UnsignedLong(element);
	}
	else if constexpr (std::is_same_v<T, int64_t>)
	{
		return writer->LongLong(element);
	}
	else if constexpr (std::is_same_v<T, uint64_t>)
	{
		return writer->UnsignedLongLong(element);
	}
	else if constexpr (std::is_same_v<T, float>)
	{
		return writer->Float(element);
	}
	else if constexpr (std::is_same_v<T, double>)
	{
		return writer->Double(element);
	}
	else if constexpr (std::is_same_v<T, char>)
	{
		return writer->Char8(element);
	}
	else if constexpr (std::is_same_v<T, char16_t>)
	{
		return writer->Char16(element);
	}
	else if constexpr (std::is_same_v<T, std::string>)
	{
		return writer->String8(element);
	}
	else if constexpr (std::is_same_v<T, std::u16string>)
	{
		return writer->String16(element);
	}
	else
	{
		return SequenceElementSerializerTrait<T>{}(writer, element);
	}
}

template<typename T>
bool SerializeSequence(ICdrWriter* writer, const BoundedSequence<T>& sequence)
{
	writer->BeginSequence(sequence.size(), ExtensibilityOf<T> != Extensibility::Final);
	for (const auto& element : sequence)
	{
		if (!SerializeElement(writer, element))
		{
			return false;
		}
	}

	writer->EndSequence();
	return true;
}

bool SerializeTypeIdentifier(ICdrWriter* writer, const TypeIdentifier& typeId);

bool SerializeSeqSmallDefn(ICdrWriter* writer, const PlainSequenceSElemDefn& seqDefn)
{
	if(!writer->BeginAggregated())
	{
		return false;
	}

	if (!SerializePlainCollectionHeader(writer, seqDefn.header))
	{
		return false;
	}

	if (!SerializeSequence(writer, seqDefn.bound))
	{
		return false;
	}

	if (!SerializeTypeIdentifier(writer, *seqDefn.element_identifier))
	{
		return false;
	}

	return writer->EndAggregated();
}

bool SerializeSeqLargeDefn(ICdrWriter* writer, const PlainSequenceLElemDefn& seqDefn)
{
	if (!writer->BeginAggregated())
	{
		return false;
	}

	if (!SerializePlainCollectionHeader(writer, seqDefn.header))
	{
		return false;
	}

	if (!SerializeSequence(writer, seqDefn.bound))
	{
		return false;
	}

	if (!SerializeTypeIdentifier(writer, *seqDefn.element_identifier))
	{
		return false;
	}

	return writer->EndAggregated();
}

bool SerializeArraySmallDefn(ICdrWriter* writer, const PlainArraySElemDefn& defn)
{
	if (!writer->BeginAggregated())
	{
		return false;
	}

	if (!SerializePlainCollectionHeader(writer, defn.header))
	{
		return false;
	}

	if (!SerializeSequence(writer, defn.arrayBoundSeq))
	{
		return false;
	}

	if (!SerializeTypeIdentifier(writer, *defn.element_identifier))
	{
		return false;
	}

	return writer->EndAggregated();
}

bool SerializeArrayLargeDefn(ICdrWriter* writer, const PlainArrayLElemDefn& defn)
{
	if (!writer->BeginAggregated())
	{
		return false;
	}

	if (!SerializePlainCollectionHeader(writer, defn.header))
	{
		return false;
	}

	if (!SerializeSequence(writer, defn.arrayBoundSeq))
	{
		return false;
	}

	if (!SerializeTypeIdentifier(writer, *defn.element_identifier))
	{
		return false;
	}

	return writer->EndAggregated();
}

bool SerializeTypeObjectHashId(ICdrWriter* writer, const TypeObjectHashId& hashId)
{
	if (!writer->Octet(hashId._d))
	{
		return false;
	}

	for (uint8_t value : hashId.hash)
	{
		if (!writer->Octet(value))
		{
			return false;
		}
	}

	return true;
}

bool SerializeStronglyConnectedComponentId(ICdrWriter* writer, const StronglyConnectedComponentId& id)
{
	if (!writer->BeginAggregated(ExtensibilityOf<StronglyConnectedComponentId>))
	{
		return false;
	}

	if (!SerializeTypeObjectHashId(writer, id.sc_component_id))
	{
		return false;
	}

	if (!writer->Long(id.scc_length))
	{
		return false;
	}

	if (!writer->Long(id.scc_index))
	{
		return false;
	}

	return writer->EndAggregated();
}

bool SerializeExtendedTypeDefn(ICdrWriter* writer, const ExtendedTypeDefn& d)
{
	writer->BeginAggregated(ExtensibilityOf<ExtendedTypeDefn>);
	writer->EndAggregated();
	return true;
}

bool SerializeTypeIdentifier(ICdrWriter* writer, const TypeIdentifier& typeId)
{
	writer->BeginAggregated(ExtensibilityOf<TypeIdentifier>);
	writer->BeginDiscriminator(0);
	writer->Octet(typeId.d());
	writer->EndDiscriminator();
	// Serialize based on typeId.d()
	switch (typeId.d())
	{
	case meshi::dds::xtypes::TI_STRING8_SMALL:
	case meshi::dds::xtypes::TI_STRING16_SMALL:
		if (!writer->Octet(typeId.string_sdefn().bound))
		{
			return false;
		}

		break;
	case meshi::dds::xtypes::TI_STRING8_LARGE:
	case meshi::dds::xtypes::TI_STRING16_LARGE:
		if (!writer->UnsignedLong(typeId.string_ldefn().bound))
		{
			return false;
		}

		break;
	case meshi::dds::xtypes::TI_PLAIN_SEQUENCE_SMALL:
		if (!SerializeSeqSmallDefn(writer, typeId.seq_sdefn()))
		{
			return false;
		}

		break;
	case meshi::dds::xtypes::TI_PLAIN_SEQUENCE_LARGE:
		if (!SerializeSeqLargeDefn(writer, typeId.seq_ldefn()))
		{
			return false;
		}

		break;
	case meshi::dds::xtypes::TI_PLAIN_ARRAY_SMALL:
		if (!SerializeArraySmallDefn(writer, typeId.array_sdefn()))
		{
			return false;
		}

		break;
	case meshi::dds::xtypes::TI_PLAIN_ARRAY_LARGE:
		if (!SerializeArrayLargeDefn(writer, typeId.array_ldefn()))
		{
			return false;
		}

		break;
	case meshi::dds::xtypes::TI_PLAIN_MAP_LARGE:
	case meshi::dds::xtypes::TI_PLAIN_MAP_SMALL:
		// TODO: Current not supported!
		return false;
	case meshi::dds::xtypes::TI_STRONGLY_CONNECTED_COMPONENT:
		if (!SerializeStronglyConnectedComponentId(writer, typeId.sc_component_id()))
		{
			return false;
		}

		break;
	default:
		if (!SerializeExtendedTypeDefn(writer, typeId.extended_defn()))
		{
			return false;
		}

		break;
	}

	writer->EndAggregated();

	return true;
}

bool SerializeCompleteTypeDetail(ICdrWriter* writer, const CompleteTypeDetail& detail)
{
	writer->BeginAggregated(ExtensibilityOf<CompleteTypeDetail>);
	// Serialize ann_builtin
	if (detail.ann_builtin != nullptr)
	{
		// FIXME: Not Support anno_builtin yet
		writer->BeginOptional(0, 0);
	}
	else
	{
		writer->BeginOptional(0, 0);
	}

	writer->EndOptional();

	// Serialize ann_custom
	if (detail.ann_custom != nullptr)
	{
		// FIXME: Not Support anno_custom yet
		writer->BeginOptional(1, 0);
	}
	else
	{
		writer->BeginOptional(1, 0);
	}

	writer->EndOptional();

	// Serialize type_name
	writer->String8(detail.type_name);
	
	writer->EndAggregated();
	return true;
}

bool SerializeCompleteStructHeader(ICdrWriter* writer, const CompleteStructHeader& header)
{
	writer->BeginAggregated(ExtensibilityOf<CompleteStructHeader>);
	
	// Serialize base_type
	writer->BeginMember(0, false, 4);
	SerializeTypeIdentifier(writer, header.base_type);
	writer->EndMember();
	
	// Serialize detail
	writer->BeginMember(1, false, 4);
	SerializeCompleteTypeDetail(writer, header.detail);
	writer->EndMember();
	
	writer->EndAggregated();
	return true;
}

bool SerializeCommonStructMember(ICdrWriter* writer, const CommonStructMember& member)
{
	writer->BeginAggregated(ExtensibilityOf<CommonStructMember>);

	writer->UnsignedLong(member.member_id);
	writer->UnsignedShort(member.member_flags.value);
	SerializeTypeIdentifier(writer, member.member_type_id);

	writer->EndAggregated();
	return true;
}

bool SerializCompleteMemberDetail(ICdrWriter* writer, const CompleteMemberDetail& member)
{
	writer->BeginAggregated(ExtensibilityOf<CompleteMemberDetail>);
	writer->String8(member.type_name);
	// Serialize ann_builtin
	if (member.ann_builtin != nullptr)
	{
		// FIXME: Not Support anno_builtin yet
		writer->BeginOptional(1, true);
	}
	else
	{
		writer->BeginOptional(1, true);
	}

	writer->EndOptional();

	// Serialize ann_custom
	if (member.ann_custom != nullptr)
	{
		// FIXME: Not Support ann_custom yet
		writer->BeginOptional(2, true);
	}
	else
	{
		writer->BeginOptional(2, true);
	}

	writer->EndOptional();

	writer->EndAggregated();
	return true;
}

bool SerializeCompleteStructType(ICdrWriter* writer, const CompleteStructType& structure)
{
	//StructTypeFlag type_flags;
	//CompleteStructHeader header;
	//CompleteStructMemberSeq member_seq;
	writer->BeginAggregated(ExtensibilityOf<CompleteStructType>);
	// Serialize type_flags
	writer->BeginMember(0, false, 2);
	writer->UnsignedShort(structure.struct_flags.value);
	writer->EndMember();

	// Serialize header
	writer->BeginMember(1, false, 5);
	if (!SerializeCompleteStructHeader(writer, structure.header))
	{
		return false;
	}

	writer->EndMember();

	// Serialize member_seq
	writer->BeginMember(2, false, 5);
	SerializeSequence(writer, structure.member_seq);
	writer->EndMember();

	writer->EndAggregated();
	return true;
}

bool SerializeCompleteTypeObject(ICdrWriter* writer, const CompleteTypeObject& typeObject)
{
	writer->BeginAggregated(ExtensibilityOf<CompleteTypeObject>);
	writer->BeginDiscriminator(0);
	writer->Octet(typeObject.d());
	writer->EndDiscriminator();

	switch (typeObject.d())
	{
	case meshi::dds::xtypes::TK_ALIAS:
		// Serialize alias_type
		break;
	case meshi::dds::xtypes::TK_ANNOTATION:
		// Serialize annotation_type
		break;
	case meshi::dds::xtypes::TK_STRUCTURE:
		// Serialize struct_type
		writer->BeginMember(3, false, 4);
		if (!SerializeCompleteStructType(writer, typeObject.struct_type()))
		{
			return false;
		}

		writer->EndMember();


		return true;

		break;
	}

	writer->EndAggregated();

	return true;
}

bool SerializeMinimalTypeDetail(ICdrWriter* writer, const MinimalTypeDetail& detail)
{
	writer->BeginAggregated(ExtensibilityOf<MinimalTypeDetail>);

	writer->EndAggregated();
	return true;

}

bool SerializeMinimalStructHeader(ICdrWriter* writer, const MinimalStructHeader& header)
{
	writer->BeginAggregated(ExtensibilityOf<MinimalStructHeader>);

	// Serialize base_type
	writer->BeginMember(0, false, 4);
	SerializeTypeIdentifier(writer, header.base_type);
	writer->EndMember();

	// Serialize detail
	writer->BeginMember(1, false, 4);
	SerializeMinimalTypeDetail(writer, header.detail);
	writer->EndMember();

	writer->EndAggregated();
	return true;
}

bool SerializeMinimalStructType(ICdrWriter* writer, const MinimalStructType& structure)
{
	//StructTypeFlag type_flags;
	//CompleteStructHeader header;
	//CompleteStructMemberSeq member_seq;
	writer->BeginAggregated(ExtensibilityOf<MinimalStructType>);
	// Serialize type_flags
	writer->BeginMember(0, false, 2);
	writer->UnsignedShort(structure.struct_flags.value);
	writer->EndMember();

	// Serialize header
	writer->BeginMember(1, false, 5);
	if (!SerializeMinimalStructHeader(writer, structure.header))
	{
		return false;
	}

	writer->EndMember();

	// Serialize member_seq
	writer->BeginMember(2, false, 5);
	SerializeSequence(writer, structure.member_seq);
	writer->EndMember();

	writer->EndAggregated();
	return true;
}

bool SerializeMinimalTypeObject(ICdrWriter* writer, const MinimalTypeObject& typeObject)
{
	writer->BeginAggregated(ExtensibilityOf<MinimalTypeObject>);
	writer->BeginDiscriminator(0);
	writer->Octet(typeObject.d());
	writer->EndDiscriminator();

	switch (typeObject.d())
	{
	case meshi::dds::xtypes::TK_ALIAS:
		// Serialize alias_type
		break;
	case meshi::dds::xtypes::TK_ANNOTATION:
		// Serialize annotation_type
		break;
	case meshi::dds::xtypes::TK_STRUCTURE:
		// Serialize struct_type
		writer->BeginMember(3, false, 4);
		if (!SerializeMinimalStructType(writer, typeObject.struct_type()))
		{
			return false;
		}

		writer->EndMember();


		return true;

		break;
	}

	writer->EndAggregated();

	return true;
}

bool SerializMinimalMemberDetail(ICdrWriter* writer, const MinimalMemberDetail& member)
{
	writer->BeginAggregated(ExtensibilityOf<MinimalMemberDetail>);
	writer->BeginArray(false);

	for (auto it : member.name_hash)
	{
		writer->Octet(it);
	}

	writer->EndArray();

	writer->EndAggregated();
	return true;
}

template<>
struct SequenceElementSerializerTrait<CompleteStructMember>{
	bool operator()(ICdrWriter* writer, const CompleteStructMember& element) { 
		if (!writer->BeginAggregated(ExtensibilityOf<CompleteStructMember>))
		{
			return false;
		}

		if (!SerializeCommonStructMember(writer, element.common))
		{
			return false;
		}

		if (!SerializCompleteMemberDetail(writer, element.detail))
		{
			return false;
		}

		return writer->EndAggregated();
	}
};

template<>
struct SequenceElementSerializerTrait<MinimalStructMember> {
	bool operator()(ICdrWriter* writer, const MinimalStructMember& element) {
		if (!writer->BeginAggregated(ExtensibilityOf<MinimalStructMember>))
		{
			return false;
		}

		if (!SerializeCommonStructMember(writer, element.common))
		{
			return false;
		}

		if (!SerializMinimalMemberDetail(writer, element.detail))
		{
			return false;
		}

		return writer->EndAggregated();
	}
};

bool GenerateMd5(std::span<uint8_t> data, std::array<uint8_t, 16>& md5Ret)
{
#if defined(_WIN32)
	BOOL bResult = false;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	DWORD dwStatus;
	DWORD cbHash;
	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv,
		NULL,
		NULL,
		PROV_RSA_FULL,
		CRYPT_VERIFYCONTEXT))
	{
		dwStatus = GetLastError();
		printf("CryptAcquireContext failed: %d\n", dwStatus);
		return false;
	}

	if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
	{
		dwStatus = GetLastError();
		printf("CryptCreateHash failed: %d\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		return false;
	}

	if (!CryptHashData(hHash, data.data(), data.size(), 0))
	{
		dwStatus = GetLastError();
		printf("CryptHashData failed: %d\n", dwStatus);
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		return false;
	}

	cbHash = md5Ret.size();
	if (!CryptGetHashParam(hHash, HP_HASHVAL, md5Ret.data(), &cbHash, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return false;
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	return true;
#endif
}

TypeObjectHashId CalculateTypeEquivalenceHash(const TypeObject& typeObject)
{
	TypeObjectHashId objHashId {  };
	if (typeObject.d() == EK_COMPLETE)
	{
		objHashId._d = EK_COMPLETE;
		Cdr2Writer<CDR1WRITER_ROLE_CALC_SIZE, std::endian::little> writerForCalcSerializeSize;
		writerForCalcSerializeSize.WriteCdrHeader();
		SerializeCompleteTypeObject(&writerForCalcSerializeSize, typeObject.complete_type());
		std::vector<uint8_t> serializedTypeObject;
		serializedTypeObject.resize(writerForCalcSerializeSize.m_offset);

		Cdr2Writer<CDR1WRITER_ROLE_SERIALIZATION, std::endian::little> writer{ serializedTypeObject.data(), serializedTypeObject.size() };
		writer.WriteCdrHeader();
		SerializeCompleteTypeObject(&writer, typeObject.complete_type());
		std::array<uint8_t, 16> md5{};
		GenerateMd5(std::span<uint8_t>{serializedTypeObject}, md5);
		memcpy(objHashId.hash.data(), md5.data(), objHashId.hash.size());
	}
	else if (typeObject.d() == EK_MINIMAL)
	{
		objHashId._d = EK_MINIMAL;
		Cdr2Writer<CDR1WRITER_ROLE_CALC_SIZE, std::endian::little> writerForCalcSerializeSize;
		writerForCalcSerializeSize.WriteCdrHeader();
		SerializeMinimalTypeObject(&writerForCalcSerializeSize, typeObject.minimal_type());
		std::vector<uint8_t> serializedTypeObject;
		serializedTypeObject.resize(writerForCalcSerializeSize.m_offset);

		Cdr2Writer<CDR1WRITER_ROLE_SERIALIZATION, std::endian::little> writer{ serializedTypeObject.data(), serializedTypeObject.size() };
		writer.WriteCdrHeader();
		SerializeMinimalTypeObject(&writer, typeObject.minimal_type());
		std::array<uint8_t, 16> md5{};
		GenerateMd5(std::span<uint8_t>{serializedTypeObject}, md5);
		memcpy(objHashId.hash.data(), md5.data(), objHashId.hash.size());
	}

	return objHashId;
}

template<>
struct std::less<EquivalenceHash>
{
	using T = EquivalenceHash;
	bool operator()(const T& _Left, const T& _Right) const
	{
		return memcmp(_Left.data(), _Right.data(), sizeof(T)) < 0;
	}
};

class TypeLibrary
{
public:
	template<typename TTypeObject>
	struct CppTTypeObject
	{
		uint32_t alignment;
		uint32_t typeSizeOf;
		// TODO: 추후에 Max Serialized Size를 계산하여 직렬화 속도를 향상시킬 수 있다.
		// uint32_t maxSerializedSize;
		TTypeObject typeObject;
		EquivalenceHash hashId;
	};

	using CppTypeObject = CppTTypeObject<CompleteTypeObject>;
	using CppMinimalTypeObject = CppTTypeObject<MinimalTypeObject>;

	const CppTypeObject* GetCompleteTypeObject(const EquivalenceHash& typeHashId)
	{
		auto it = m_typeObjectTable.find(typeHashId);
		if (it != m_typeObjectTable.end())
		{
			return &it->second;
		}

		return nullptr;
	}

	const MinimalTypeObject* GetMinimalTypeObject(const EquivalenceHash& typeHashId)
	{
		auto it = m_minimalTypeObjectTable.find(typeHashId);
		if (it != m_minimalTypeObjectTable.end())
		{
			return &it->second.typeObject;
		}

		return nullptr;
	}

	bool RegisterType(const MinimalTypeObject& typeObject, EquivalenceHash* const ret)
	{
		TypeObject to{ typeObject };
		auto typeHashId = CalculateTypeEquivalenceHash(to);
		if (typeHashId._d != EK_MINIMAL)
		{
			return false;
		}
		if (m_minimalTypeObjectTable.count(typeHashId.hash) != 0)
		{
			return false;
		}

		// TODO: 
		// Alignment와 TypeSizeOf를 계산한다.
		auto alignment = GetTypeAlignment(typeObject);
		uint32_t typeSizeOf = GetTypeDeserializedSize(typeObject);
		if (alignment == 0 || typeSizeOf == 0)
		{
			return false;
		}
		CppMinimalTypeObject cppTypeObject{ alignment , typeSizeOf, typeObject, typeHashId.hash };
		m_minimalTypeObjectTable.emplace(typeHashId.hash, cppTypeObject);
		m_minimalTopLevelTypeObjectTable.emplace(typeHashId.hash, cppTypeObject);
		if (ret != nullptr)
		{
			*ret = typeHashId.hash;
		}
		return true;
	}

	bool RegisterType(const CompleteTypeObject& typeObject, EquivalenceHash* const ret)
	{
		TypeObject to{ typeObject };
		auto typeHashId = CalculateTypeEquivalenceHash(to);
		if (typeHashId._d != EK_COMPLETE)
		{
			return false;
		}

		if (m_typeObjectTable.count(typeHashId.hash) != 0)
		{
			return false;
		}

		// Alignment와 TypeSizeOf를 계산한다.
		auto alignment = GetAlignment(typeObject);
		uint32_t typeSizeOf = GetTypeDeserializedSize(typeObject);
		if (alignment == 0 || typeSizeOf == 0)
		{
			return false;
		}

		CppTypeObject cppTypeObject{ alignment , typeSizeOf, typeObject, typeHashId.hash };
		m_typeObjectTable.emplace(typeHashId.hash, cppTypeObject);
		m_topLevelTypeObjectTable.emplace(typeHashId.hash, cppTypeObject);
		if (ret != nullptr)
		{
			*ret = typeHashId.hash;
		}

		return true;
	}

	uint32_t GetAlignment(const TypeIdentifier& typeIdentifier)
	{
		switch (typeIdentifier.d())
		{
		case TK_BYTE:
		case TK_CHAR8:
		case TK_BOOLEAN:
			return 1;
		case TK_INT16:
		case TK_UINT16:
		case TK_CHAR16:
			return 2;
		case TK_INT32:
		case TK_UINT32:
		case TK_FLOAT32:
			return 4;
		case TK_INT64:
		case TK_UINT64:
		case TK_FLOAT64:
			return 8;
		case TK_STRING8:
		case TI_STRING8_SMALL:
		case TI_STRING8_LARGE:
			return std::alignment_of_v<std::string>;
		case TK_STRING16:
		case TI_STRING16_SMALL:
		case TI_STRING16_LARGE:
			return std::alignment_of_v<std::u16string>;
		case TI_PLAIN_SEQUENCE_SMALL:
		case TI_PLAIN_SEQUENCE_LARGE:
			return std::alignment_of_v<std::vector<uintptr_t>>;
		case TI_PLAIN_ARRAY_SMALL:
			return GetAlignment(*typeIdentifier.array_sdefn().element_identifier);
		case TI_PLAIN_ARRAY_LARGE:
			return GetAlignment(*typeIdentifier.array_ldefn().element_identifier);

		case TI_STRONGLY_CONNECTED_COMPONENT:
			// NOTE: TI_STRONGLY_CONNECTED_COMPONENT is not support in current version.
			return 0;

		case EK_COMPLETE:
			if (auto it = m_typeObjectTable.find(typeIdentifier.equivalence_hash())
				; it != m_typeObjectTable.end())
			{ 
				return it->second.alignment;
			}

			return 0;

		case EK_MINIMAL:
			// NOTE: Minimal is not support in current version.
			return 0;

		default:
			return 0;
		}
	}

	uint32_t GetAlignment(const CompleteStructType& typeObject)
	{
		if (typeObject.header.base_type.d() != TK_NONE)
		{
			// NOTE: Inheritance is unsupported in Current Version
			return 0;
		}

		if (typeObject.member_seq.size() == 0)
		{
			// NOTE: Empty structure is unsupported in Current Version
			return 0;
		}

		uint32_t maxAlignment = 0;
		for (auto& it : typeObject.member_seq)
		{
			auto alignment = GetAlignment(it.common.member_type_id);
			maxAlignment = std::max(maxAlignment, alignment);
		}

		return maxAlignment;
	}

	uint32_t GetAlignment(const CompleteUnionType& typeObject)
	{
		if (typeObject.member_seq.size() == 0)
		{
			// NOTE: Empty structure is unsupported in Current Version
			return 0;
		}

		auto& firstField = typeObject.discriminator.common.type_id;
		auto maxAlignment = GetAlignment(firstField);
		for (auto& it : typeObject.member_seq)
		{
			auto fieldAlignment = GetAlignment(it.common.type_id);
			maxAlignment = std::max(maxAlignment, fieldAlignment);
		}

		return maxAlignment;
	}

	uint32_t GetAlignmentByTypeId(const TypeIdentifier& typeIdentifier)
	{
		switch (typeIdentifier.d())
		{
		case TK_BYTE:
		case TK_CHAR8:
		case TK_BOOLEAN:
			return 1;
		case TK_INT16:
		case TK_UINT16:
		case TK_CHAR16:
			return 2;
		case TK_INT32:
		case TK_UINT32:
		case TK_FLOAT32:
			return 4;
		case TK_INT64:
		case TK_UINT64:
		case TK_FLOAT64:
			return 8;
		case TK_STRING8:
		case TI_STRING8_SMALL:
		case TI_STRING8_LARGE:
			return std::alignment_of_v<std::string>;
		case TK_STRING16:
		case TI_STRING16_SMALL:
		case TI_STRING16_LARGE:
			return std::alignment_of_v<std::u16string>;
		case TI_PLAIN_SEQUENCE_SMALL:
		case TI_PLAIN_SEQUENCE_LARGE:
			return std::alignment_of_v<std::vector<uintptr_t>>;
		case TI_PLAIN_ARRAY_SMALL:
			return GetAlignmentByTypeId(*typeIdentifier.array_sdefn().element_identifier);
		case TI_PLAIN_ARRAY_LARGE:
			return GetAlignmentByTypeId(*typeIdentifier.array_ldefn().element_identifier);

		case TI_STRONGLY_CONNECTED_COMPONENT:
			// NOTE: TI_STRONGLY_CONNECTED_COMPONENT is not support in current version.
			return 0;

		case EK_COMPLETE:
			if (auto it = m_typeObjectTable.find(typeIdentifier.equivalence_hash())
				; it != m_typeObjectTable.end())
			{
				if (it->second.alignment == 0)
				{
					return 0;
				}

				return it->second.alignment;
			}
			
			return { 0 };

		case EK_MINIMAL:
			if (auto it = m_minimalTypeObjectTable.find(typeIdentifier.equivalence_hash())
				; it != m_minimalTypeObjectTable.end())
			{
				if (it->second.alignment == 0)
				{
					return 0;
				}

				return it->second.alignment;
			}

			return 0;

		default:
			return 0;
		}
	}

	template<typename TUnionType>
	uint32_t GetUnionAlignment(const TUnionType& unionType)
	{
		if (unionType.member_seq.size() == 0)
		{
			// NOTE: Empty structure is unsupported in Current Version
			return 0;
		}

		uint32_t maxAlignment = GetAlignment(unionType.discriminator.common.type_id);
		for (auto& it : unionType.member_seq)
		{
			uint32_t alignment = GetAlignmentByTypeId(it.common.type_id);
			maxAlignment = std::max(maxAlignment, alignment);
		}

		return maxAlignment;
	}

	template<typename TStructType>
	uint32_t GetStructAlignment(const TStructType& structType)
	{
		if (structType.header.base_type.d() != TK_NONE)
		{
			// NOTE: Inheritance is unsupported in Current Version
			return 0;
		}

		if (structType.member_seq.size() == 0)
		{
			// NOTE: Empty structure is unsupported in Current Version
			return 0;
		}

		uint32_t maxAlignment = 0;
		for (auto& it : structType.member_seq)
		{
			uint32_t alignment = GetAlignmentByTypeId(it.common.member_type_id);
			maxAlignment = std::max(maxAlignment, alignment);
		}

		return maxAlignment;
	}

	template<typename TTypeObject>
	uint32_t GetTypeAlignment(const TTypeObject typeObject)
	{
		switch (typeObject.d())
		{
		case TK_ALIAS:
			return GetAlignmentByTypeId(typeObject.alias_type().body.common.related_type);
		case TK_UNION:
			return GetUnionAlignment(typeObject.union_type());
		case TK_STRUCTURE:
			return GetStructAlignment(typeObject.struct_type());
		}

		return 0;
	}

	uint32_t GetAlignment(const CompleteTypeObject& typeObject)
	{
		static_assert(std::alignment_of_v<std::vector<std::string>> == sizeof(void*), "Unsupported C++ Runtime");
		static_assert(sizeof(std::vector<std::string>) == sizeof(std::vector<char>), "Unsupported C++ Runtime");
		switch (typeObject.d())
		{
		case TK_ALIAS:
			return GetAlignment(typeObject.alias_type().body.common.related_type);
		case TK_UNION:
			return GetAlignment(typeObject.union_type());
		case TK_STRUCTURE:
			return GetAlignment(typeObject.struct_type());
		}

		return 0;
	}

	template<typename TArrayElemDefn>
	uint32_t GetArrayDeserializedSize(const PlainArrayTElemDefn<TArrayElemDefn>& def)
	{
		uint32_t arraySize = GetDeserializedSizeByTypeId(*def.element_identifier);
		for (const auto& b : def.arrayBoundSeq)
		{
			arraySize *= b;
		}

		return arraySize;
	}

	uint32_t GetDeserializedSizeByTypeId(const TypeIdentifier& typeIdentifier)
	{
		switch (typeIdentifier.d())
		{
		case TK_BYTE:
		case TK_CHAR8:
		case TK_BOOLEAN:
			return 1;
		case TK_INT16:
		case TK_UINT16:
		case TK_CHAR16:
			return 2;
		case TK_INT32:
		case TK_UINT32:
		case TK_FLOAT32:
			return 4;
		case TK_INT64:
		case TK_UINT64:
		case TK_FLOAT64:
			return 8;
		case TK_STRING8:
		case TI_STRING8_SMALL:
		case TI_STRING8_LARGE:
			return sizeof(std::string);
		case TK_STRING16:
		case TI_STRING16_SMALL:
		case TI_STRING16_LARGE:
			return sizeof(std::u16string);
		case TI_PLAIN_SEQUENCE_SMALL:
			if (typeIdentifier.seq_sdefn().element_identifier->d() == TK_BOOLEAN)
			{
				return sizeof(std::vector<bool>);
			}

			return sizeof(std::vector<uintptr_t>);
		case TI_PLAIN_SEQUENCE_LARGE:
			if (typeIdentifier.seq_ldefn().element_identifier->d() == TK_BOOLEAN)
			{
				return sizeof(std::vector<bool>);
			}

			return sizeof(std::vector<uintptr_t>);
		case TI_PLAIN_ARRAY_SMALL:
			return GetArrayDeserializedSize(typeIdentifier.array_sdefn());
		case TI_PLAIN_ARRAY_LARGE:
			return GetArrayDeserializedSize(typeIdentifier.array_ldefn());
		case EK_COMPLETE:
			if (auto it = m_typeObjectTable.find(typeIdentifier.equivalence_hash())
				; it != m_typeObjectTable.end())
			{
				if (it->second.typeSizeOf == 0)
				{
					it->second.typeSizeOf = GetTypeDeserializedSize(it->second.typeObject);
				}

				return it->second.typeSizeOf;
			}

			return 0;

		case EK_MINIMAL:
			// NOTE: Minimal is not support in current version.
			return 0;
		default:
			return 0;
		}
	}

	template<typename TStructType>
	uint32_t GetStructDeserializedSize(const TStructType& typeObject)
	{
		if (typeObject.header.base_type.d() != TK_NONE)
		{
			// NOTE: Inheritance is unsupported in Current Version
			return 0;
		}
		
		uint32_t totalSize = 0;
		uint32_t structAlignment = 1;
		for (const auto& member : typeObject.member_seq)
		{
			auto alignment = GetAlignmentByTypeId(member.common.member_type_id);
			structAlignment = std::max(alignment, structAlignment);
			totalSize = (totalSize + (alignment - 1)) & ~(alignment - 1);
			totalSize += GetDeserializedSizeByTypeId(member.common.member_type_id);
		}

		totalSize = (totalSize + (structAlignment - 1)) & ~(structAlignment - 1);
		return totalSize;
	}

	template<typename TUnionType>
	uint32_t GetUnionDeserializedSize(const TUnionType& typeObject)
	{
		uint32_t totalSize = 0;
		uint32_t structAlignment = 1;
		uint32_t alignment;
		uint32_t memberSize = 0;
		uint32_t discriminator = GetDeserializedSizeByTypeId(typeObject.discriminator.common.type_id);

		for (const auto& member : typeObject.member_seq)
		{
			auto alignment = GetAlignmentByTypeId(member.common.type_id);
			structAlignment = std::max(alignment, structAlignment);
			memberSize = std::max(memberSize, GetDeserializedSizeByTypeId(member.common.type_id));
		}

		totalSize = discriminator;
		totalSize = (totalSize + (structAlignment - 1)) & ~(structAlignment - 1);
		totalSize += memberSize;
		totalSize = (totalSize + (structAlignment - 1)) & ~(structAlignment - 1);
		return totalSize;
	}

	template<typename TTypeObject>
	uint32_t GetTypeDeserializedSize(const TTypeObject& typeObject)
	{
		switch (typeObject.d())
		{
		case TK_ALIAS:
			return GetDeserializedSizeByTypeId(typeObject.alias_type().body.common.related_type);
		case TK_UNION:
			return GetUnionDeserializedSize(typeObject.union_type());
		case TK_STRUCTURE:
			return GetStructDeserializedSize(typeObject.struct_type());
		}

		return 0;
	}

	std::vector<uint8_t> Serialize(const TypeObjectHashId& typeIden, const void* obj, uint32_t length)
	{
		if (typeIden._d == EK_COMPLETE)
		{
			auto it = m_topLevelTypeObjectTable.find(typeIden.hash);
			if (it == m_topLevelTypeObjectTable.end())
			{
				return {};
			}

			return SerializeObject(it->second, obj, length);
		}
		else if(typeIden._d == EK_MINIMAL)
		{
			auto it = m_minimalTopLevelTypeObjectTable.find(typeIden.hash);
			if (it == m_minimalTopLevelTypeObjectTable.end())
			{
				return {};
			}

			return SerializeObject(it->second, obj, length);
		}

		return {};
	}

	template<typename TStructType>
	bool SerializeStruct(ICdrWriter& writer, const TStructType& typeObject, uint32_t alignment, const uint8_t* obj, uint32_t& offset, uint32_t length)
	{
		writer.BeginAggregated();
		if (typeObject.header.base_type.d() != TK_NONE)
		{
			// NOTE: Inheritance is unsupported in Current Version
			return false;
		}

		for (const auto& member : typeObject.member_seq)
		{
			writer.BeginMember(member.common.member_id, false, 0);
			SerializeByTypeId(writer, member.common.member_type_id, obj, offset, length);
			writer.EndMember();
		}

		writer.EndAggregated();

		offset = Align(offset, alignment);
		return true;
	}


	template<typename TUnionType>
	bool SerializeUnion(ICdrWriter& writer, const TUnionType& typeObject, uint32_t maxAlignment, const uint8_t* obj, uint32_t& offset, uint32_t length)
	{
		int32_t discriminator = 0;
		writer.BeginAggregated();
		// TODO: Correct LengthCode
		writer.BeginDiscriminator(0);
		SerializeDiscriminator(writer, typeObject.discriminator.common.type_id, obj, offset, length, discriminator);
		writer.EndDiscriminator();

		bool hit = false;
		for (const auto& member : typeObject.member_seq)
		{
			for (auto label : member.common.label_seq)
			{
				if (discriminator != label)
					continue;

				hit = true;
				break;
			}

			if (!hit)
				continue;

			writer.BeginMember(member.common.member_id, false, 0);
			SerializeByTypeId(writer, member.common.type_id, obj, offset, length);
			writer.EndMember();
			break;
		}

		writer.EndAggregated();

		if (!hit)
			return false;

		offset = Align(offset, maxAlignment);
		return true;
	}

	template<typename TTypeObject>
	bool SerializeAny(ICdrWriter& writer, const CppTTypeObject<TTypeObject>& typeObject, const void* obj, uint32_t& offset, uint32_t length)
	{
		switch (typeObject.typeObject.d())
		{
		case TK_ALIAS:
			// TODO: Not supported yet
			return false;
			// return GetDeserializedSize(typeObject.alias_type().body.common.related_type);
		case TK_UNION:
			return SerializeUnion(writer, typeObject.typeObject.union_type(), typeObject.alignment, (const uint8_t*)obj, offset, length);
		case TK_STRUCTURE:
			return SerializeStruct(writer, typeObject.typeObject.struct_type(), typeObject.alignment, (const uint8_t*)obj, offset, length);
		}

		return false;
	}

	template<typename TTypeObject>
	std::vector<uint8_t> SerializeObject(const CppTTypeObject<TTypeObject>& typeObject, const void* obj, uint32_t length)
	{
		std::vector<uint8_t> ret;
		Cdr1Writer<CDR1WRITER_ROLE_CALC_SIZE> writerForCalcSize{  };
		uint32_t offset = 0;
		writerForCalcSize.WriteCdrHeader();
		SerializeAny(writerForCalcSize, typeObject, (const uint8_t*)obj, offset, length);
		ret.resize(writerForCalcSize.m_offset);
		Cdr1Writer<CDR1WRITER_ROLE_SERIALIZATION> writer{ ret.data(), ret.size() };
		offset = 0;
		writer.WriteCdrHeader();
		SerializeAny(writer, typeObject, (const uint8_t*)obj, offset, length);
		ret.erase(ret.begin() + writer.m_offset, ret.end());
		return ret;
	}

	uint32_t Align(uint32_t offset, uint32_t alignment)
	{
		return (offset + (alignment - 1)) & ~(alignment - 1);
	}

	bool SerializeByTypeId(ICdrWriter& writer, const TypeIdentifier& identifier, const uint8_t* obj, uint32_t& offset, uint32_t length)
	{
		switch (identifier.d())
		{
		case TK_BYTE:
		case TK_CHAR8:
		case TK_BOOLEAN:
			if (!writer.Octet(*(const uint8_t*)(obj + offset)))
				return false;

			offset += 1;
			return true;
		case TK_INT16:
		case TK_UINT16:
		case TK_CHAR16:
			offset = Align(offset, 2);
			if (!writer.UnsignedShort(*(const uint16_t*)(obj + offset)))
				return false;

			offset += 2;
			return true;
		case TK_INT32:
		case TK_UINT32:
		case TK_FLOAT32:
			offset = Align(offset, 4);
			if (!writer.UnsignedLong(*(const uint32_t*)(obj + offset)))
				return false;

			offset += 4;
			return true;
		case TK_INT64:
		case TK_UINT64:
		case TK_FLOAT64:
			offset = Align(offset, 8);
			if (!writer.UnsignedLongLong(*(const uint64_t*)(obj + offset)))
				return false;

			offset += 8;
			return true;
		case TK_STRING8:
		case TI_STRING8_SMALL:
		case TI_STRING8_LARGE:
			offset = Align(offset, std::alignment_of_v<std::string>);
			if (!writer.String8(*(const std::string*)(obj + offset)))
				return false;

			offset += sizeof(std::string);
			return true;
		case TK_STRING16:
		case TI_STRING16_SMALL:
		case TI_STRING16_LARGE:
			offset = Align(offset, std::alignment_of_v<std::u16string>);
			if (!writer.String16(*(const std::u16string*)(obj + offset)))
				return false;

			offset += sizeof(std::u16string);
			return true;
		case TI_PLAIN_SEQUENCE_SMALL:
			// TODO: Implement Sequence Serialization
			return false;
		case TI_PLAIN_SEQUENCE_LARGE:
			// TODO: Implement Sequence Serialization
			return false;
		case TI_PLAIN_ARRAY_SMALL:
			// TODO: Implement Array Serialization
			return false;
		case TI_PLAIN_ARRAY_LARGE:
			// TODO: Implement Array Serialization
			return false;
		case EK_COMPLETE:
			if (auto it = m_typeObjectTable.find(identifier.equivalence_hash())
				; it != m_typeObjectTable.end())
			{
				if (it->second.typeSizeOf == 0)
				{
					return false;
				}

				return SerializeAny(writer, it->second, obj, offset, length);
			}

			return false;

		case EK_MINIMAL:
			if (auto it = m_minimalTypeObjectTable.find(identifier.equivalence_hash())
				; it != m_minimalTypeObjectTable.end())
			{
				if (it->second.typeSizeOf == 0)
				{
					return false;
				}

				return SerializeAny(writer, it->second, obj, offset, length);
			}

			return false;
		default:
			return false;
		}

		return false;
	}

	bool SerializeDiscriminator(ICdrWriter& writer, const TypeIdentifier& typeId, const uint8_t* obj, uint32_t& offset, uint32_t length, Long& discriminator)
	{
		switch (typeId.d())
		{
		case TK_BYTE:
		case TK_CHAR8:
		case TK_BOOLEAN:
			if (!writer.Octet(*(const uint8_t*)(obj + offset)))
				return false;

			discriminator = *(const uint8_t*)(obj + offset);
			offset += 1;
			return true;
		case TK_INT16:
			offset = Align(offset, 2);
			if (!writer.UnsignedShort(*(const uint16_t*)(obj + offset)))
				return false;

			discriminator = *(const int16_t*)(obj + offset);
			offset += 2;
			return true;
		case TK_UINT16:
		case TK_CHAR16:
			offset = Align(offset, 2);
			if (!writer.UnsignedShort(*(const uint16_t*)(obj + offset)))
				return false;

			discriminator = *(const uint16_t*)(obj + offset);
			offset += 2;
			return true;
			
		case TK_INT32:
			offset = Align(offset, 4);
			if (!writer.UnsignedLong(*(const uint32_t*)(obj + offset)))
				return false;

			discriminator = *(const int32_t*)(obj + offset);
			offset += 4;
			return true;
		case TK_UINT32:
			offset = Align(offset, 4);
			if (!writer.UnsignedLong(*(const uint32_t*)(obj + offset)))
				return false;

			discriminator = *(const uint32_t*)(obj + offset);
			offset += 4;
			return true;
		case TK_FLOAT32:
			offset = Align(offset, 4);
			if (!writer.UnsignedLong(*(const uint32_t*)(obj + offset)))
				return false;

			discriminator = *(const float*)(obj + offset);
			offset += 4;
			return true;
		case TK_INT64:
			offset = Align(offset, 8);
			if (!writer.UnsignedLongLong(*(const uint64_t*)(obj + offset)))
				return false;

			discriminator = *(const int64_t*)(obj + offset);
			offset += 8;
			return true;
		case TK_UINT64:
			offset = Align(offset, 8);
			if (!writer.UnsignedLongLong(*(const uint64_t*)(obj + offset)))
				return false;

			discriminator = *(const uint64_t*)(obj + offset);
			offset += 8;
			return true;
		case TK_FLOAT64:
			offset = Align(offset, 8);
			if (!writer.UnsignedLongLong(*(const uint64_t*)(obj + offset)))
				return false;

			discriminator = *(const double*)(obj + offset);
			offset += 8;
			return true;
		case TK_STRING8:
		case TI_STRING8_SMALL:
		case TI_STRING8_LARGE:
		case TK_STRING16:
		case TI_STRING16_SMALL:
		case TI_STRING16_LARGE:
		case TI_PLAIN_SEQUENCE_SMALL:
		case TI_PLAIN_SEQUENCE_LARGE:
		case TI_PLAIN_ARRAY_SMALL:
		case TI_PLAIN_ARRAY_LARGE:
			return false;
		case EK_COMPLETE:
			if (auto it = m_typeObjectTable.find(typeId.equivalence_hash())
				; it != m_typeObjectTable.end())
			{
				if (it->second.typeSizeOf == 0)
				{
					return false;
				}

				// TODO: Check TypeAlias and identify integer primitive type, otherwise, it cannot be discriminator.
				// return Serialize(writer, it->second, obj, offset, length);
			}

			return false;
		case EK_MINIMAL:

			// NOTE: Minimal is not support in current version.
			return false;
		default:
			return false;
		}

		return false;
	}

private:
	std::map<EquivalenceHash, CppTypeObject> m_topLevelTypeObjectTable;
	std::map<EquivalenceHash, CppTypeObject> m_typeObjectTable;
	std::map<EquivalenceHash, CppMinimalTypeObject> m_minimalTypeObjectTable;
	std::map<EquivalenceHash, CppMinimalTypeObject> m_minimalTopLevelTypeObjectTable;

};

std::optional<MinimalTypeObject> CreateFromCompleteTypeObject(const CompleteTypeObject& completeTypeObject, TypeLibrary& library) {
	TypeKind d = completeTypeObject.d();
	if (d == TK_STRUCTURE)
	{
		const auto& structType = completeTypeObject.struct_type();
		MinimalStructType minimalStructType{ structType.struct_flags, MinimalStructHeader{ structType.header.base_type } };
		for (auto& it : structType.member_seq)
		{
			std::array<uint8_t, 16> nameHashMd5;
			MinimalStructMember minimalMember{ it.common, {} };
			if (minimalMember.common.member_type_id.d() == EK_COMPLETE)
			{
				// MinimalTypeObject는 CompleteTypeObject를 참조하지 않도록 강제한다.
				if (auto nestedTypeObject = library.GetCompleteTypeObject(minimalMember.common.member_type_id.equivalence_hash());
					nestedTypeObject != nullptr)
				{
					if (auto minimalNestedTypeObject = CreateFromCompleteTypeObject(nestedTypeObject->typeObject, library);
						minimalNestedTypeObject.has_value())
					{
						TypeObjectHashId hash{};
						hash = CalculateTypeEquivalenceHash(nestedTypeObject->typeObject);
						auto obj = library.GetMinimalTypeObject(hash.hash);
						if (obj == nullptr)
						{
							library.RegisterType(minimalNestedTypeObject.value(), &hash.hash);
						}
						
						minimalMember.common.member_type_id.d(EK_MINIMAL);
						minimalMember.common.member_type_id.equivalence_hash() = hash.hash;
					}
				}
			}

			GenerateMd5(std::span<uint8_t>{(uint8_t*)it.detail.type_name.data(), it.detail.type_name.length()}, nameHashMd5);
			memcpy(minimalMember.detail.name_hash.data(), nameHashMd5.data(), minimalMember.detail.name_hash.size());

			minimalStructType.member_seq.push_back(minimalMember);
		}

		return MinimalTypeObject{ minimalStructType };
	}
	else if (d == TK_UNION)
	{

	}
	else if (d == TK_ALIAS)
	{
		
	}

	return std::nullopt;
}

class CdrReader
{
public:

	void AddTypeObject(const CompleteTypeObject& typeObject)
	{

	}

	void RegisterHandler(const EquivalenceHash& typeHash, ICdrHandler* handler);

	bool Deserialize(const uint8_t* serPayload, uint32_t length)
	{
		Cdr1Writer<CDR1WRITER_ROLE_CALC_SIZE> writerForCalcSize{  };


		return true;
	}

private:
	std::shared_ptr<TypeLibrary> m_typeLibrary;
};


int main()
{
	struct B
	{
		char c;
		int64_t d;
	};
	struct A
	{
		int a;
		B b;
	};


	using namespace meshi::dds::xtypes;

	CompleteTypeObject HelloMsgCompleteTypeObject{
		CompleteStructType{
			StructTypeFlag{},
			CompleteStructHeader{
				TypeIdentifier{TK_NONE},
				CompleteTypeDetail{
					nullptr,
					nullptr,
					"Hello::Msg"}},
			CompleteStructMemberSeq{
				CompleteStructMember{
					CommonStructMember{
						0,
						StructMemberFlag{},
						TypeIdentifier{TK_INT32}},
					CompleteMemberDetail{
						"userId",
						nullptr,
						nullptr}},
				CompleteStructMember{
					CommonStructMember{
						1,
						StructMemberFlag{},
						TypeIdentifier{TK_STRING8}},
					CompleteMemberDetail{
						"message",
						nullptr,
						nullptr}},
				CompleteStructMember{
					CommonStructMember{
						2,
						StructMemberFlag{},
						TypeIdentifier{TK_BYTE}},
					CompleteMemberDetail{
						"tmp",
						nullptr,
						nullptr}}
			}
		}
	};

	TypeObject HelloMsgTypeObject{ HelloMsgCompleteTypeObject };
	// auto typeHashId = CalculateTypeEquivalenceHash(HelloMsgTypeObject);

	EquivalenceHash helloMsgHashId;
	EquivalenceHash helloMsg2HashId;
	auto typeLibrary = std::make_shared<TypeLibrary>();
	typeLibrary->RegisterType(HelloMsgCompleteTypeObject, &helloMsgHashId);

	CompleteTypeObject HelloMsg2CompleteTypeObject{
		CompleteStructType{
			StructTypeFlag{},
			CompleteStructHeader{
				TypeIdentifier{TK_NONE},
				CompleteTypeDetail{
					nullptr,
					nullptr,
					"Hello::Msg2"}},
			CompleteStructMemberSeq{
				CompleteStructMember{
					CommonStructMember{
						0,
						StructMemberFlag{},
						TypeIdentifier{EK_COMPLETE, helloMsgHashId }},
					CompleteMemberDetail{
						"obj",
						nullptr,
						nullptr}},
				CompleteStructMember{
					CommonStructMember{
						1,
						StructMemberFlag{},
						TypeIdentifier{TK_STRING8}},
					CompleteMemberDetail{
						"message",
						nullptr,
						nullptr}}
			}
		}
	};

	typeLibrary->RegisterType(HelloMsg2CompleteTypeObject, &helloMsg2HashId);

	EquivalenceHash minimalHash{};
	auto helloMsg2MinimalObject = CreateFromCompleteTypeObject(HelloMsg2CompleteTypeObject, *typeLibrary);
	typeLibrary->RegisterType(helloMsg2MinimalObject.value(), &minimalHash);
	Hello::Msg2 msg{};
	msg.obj.userId = 555;
	msg.obj.message = "Hello, World!";
	msg.obj.tmp = 1;
	msg.message = "This is Msg2";

	auto ret = typeLibrary->Serialize(TypeObjectHashId{ EK_MINIMAL, minimalHash }, &msg, sizeof(msg));
	return 0;
}
