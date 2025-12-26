#pragma once
#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <optional>
#include <string>
#include <cassert>
#include <exception>

namespace meshi
{
	namespace dds {
		using Boolean = bool;
		using Octet = uint8_t;
		using Short = int16_t;
		using Long = int32_t;
		using LongLong = int64_t;
		using UnsignedShort = uint16_t;
		using UnsignedLong = uint32_t;
		using UnsignedLongLong = uint64_t;
		using Float = float;
		using Double = double;
		using Char = char;
		using WideChar = wchar_t;
		template<uint32_t N = std::numeric_limits<uint32_t>::max()>
		using String = std::string;

		template<uint32_t N = std::numeric_limits<uint32_t>::max()>
		using WideString = std::wstring;

		template<typename TElem, uint32_t MAXIMUM = std::numeric_limits<uint32_t>::max()>
		using Sequence = std::vector<TElem>;

		// TODO: Implement bounded string
		/*
		template<size_t STRING_MAX_LENGTH>
		class BoundedString
		{

		};
		*/

		template<size_t STRING_MAX_LENGTH>
		using BoundedString = std::string;

		// TODO: Implement bounded sequence
		/*template<typename TEle, size_t SEQUENCE_MAX_LENGTH = SIZE_MAX>
		class BoundedSequence
		{

		};*/

		template<typename TEle, size_t SEQUENCE_MAX_LENGTH = SIZE_MAX>
		using BoundedSequence = std::vector<TEle>;

		template<typename T>
		using External = T*;

		template<typename T>
		using Optional = T*;

		namespace xtypes
		{
			enum class Extensibility {
				Final,
				Appendable,
				Mutable
			};

			template<typename T>
			struct XTypesTrait {
				static constexpr Extensibility extensibility = Extensibility::Final;
			};

			template<typename T>
			constexpr Extensibility ExtensibilityOf = XTypesTrait<std::remove_cvref_t<std::remove_all_extents_t<T>>>::extensibility;

			using EquivalenceKind = Octet;

			constexpr UnsignedLong ANNOTATION_STR_VALUE_MAX_LEN = 128;
			constexpr UnsignedLong ANNOTATION_OCTETSEC_VALUE_MAX_LEN = 128;

			typedef Octet TypeKind;

			constexpr EquivalenceKind EK_MINIMAL = 0xF1;
			constexpr EquivalenceKind EK_COMPLETE = 0xF2;
			constexpr EquivalenceKind EK_BOTH = 0xF3;

			constexpr TypeKind TK_NONE = 0x00;
			constexpr TypeKind TK_BOOLEAN = 0x01;
			constexpr TypeKind TK_BYTE = 0x02;
			constexpr TypeKind TK_INT16 = 0x03;
			constexpr TypeKind TK_INT32 = 0x04;
			constexpr TypeKind TK_INT64 = 0x05;
			constexpr TypeKind TK_UINT16 = 0x06;
			constexpr TypeKind TK_UINT32 = 0x07;
			constexpr TypeKind TK_UINT64 = 0x08;
			constexpr TypeKind TK_FLOAT32 = 0x09;
			constexpr TypeKind TK_FLOAT64 = 0x0A;
			constexpr TypeKind TK_FLOAT128 = 0x0B;
			constexpr TypeKind TK_CHAR8 = 0x10;
			constexpr TypeKind TK_CHAR16 = 0x11;

			constexpr TypeKind TK_STRING8 = 0x20;
			constexpr TypeKind TK_STRING16 = 0x21;

			constexpr TypeKind TK_ALIAS = 0x30;

			constexpr TypeKind TK_ENUM = 0x40;
			constexpr TypeKind TK_BITMASK = 0x41;

			constexpr TypeKind TK_ANNOTATION = 0x50;
			constexpr TypeKind TK_STRUCTURE = 0x51;
			constexpr TypeKind TK_UNION = 0x52;
			constexpr TypeKind TK_BITSET = 0x53;

			constexpr TypeKind TK_SEQUENCE = 0x60;
			constexpr TypeKind TK_ARRAY = 0x61;
			constexpr TypeKind TK_MAP = 0x62;

			using TypeIdentiferKind = Octet;
			constexpr TypeIdentiferKind TI_STRING8_SMALL = 0x70;
			constexpr TypeIdentiferKind TI_STRING8_LARGE = 0x71;
			constexpr TypeIdentiferKind TI_STRING16_SMALL = 0x72;
			constexpr TypeIdentiferKind TI_STRING16_LARGE = 0x73;

			constexpr TypeIdentiferKind TI_PLAIN_SEQUENCE_SMALL = 0x80;
			constexpr TypeIdentiferKind TI_PLAIN_SEQUENCE_LARGE = 0x81;

			constexpr TypeIdentiferKind TI_PLAIN_ARRAY_SMALL = 0x90;
			constexpr TypeIdentiferKind TI_PLAIN_ARRAY_LARGE = 0x91;

			constexpr TypeIdentiferKind TI_PLAIN_MAP_SMALL = 0xA0;
			constexpr TypeIdentiferKind TI_PLAIN_MAP_LARGE = 0xA1;

			constexpr TypeIdentiferKind TI_STRONGLY_CONNECTED_COMPONENT = 0xB0;

			constexpr Long MEMBER_NAME_MAX_LEMGTH = 256;

			using MemberName = BoundedString<MEMBER_NAME_MAX_LEMGTH>;

			constexpr Long TYPE_NAME_MAX_LENGTH = 256;

			using QualifiedTypeName = BoundedString<MEMBER_NAME_MAX_LEMGTH>;

			using PrimitiveTypeId = Octet;

			using EquivalenceHash = std::array<Octet, 14>;

			using NameHash = std::array<Octet, 4>;

			using LBound = UnsignedLong;

			using LBoundSeq = BoundedSequence<LBound>;

			using SBound = Octet;

			using SBoundSeq = BoundedSequence<SBound>;

			struct TypeObjectHashId
			{
				uint8_t _d;
				EquivalenceHash hash;
			};

			struct MemberFlag
			{
				enum MemberFlagBit
				{
					TRY_CONSTRUCT1, // T1
					TRY_CONSTRUCT2, // T2
					IS_EXTERNAL, // X
					IS_OPTIONAL, // O
					IS_MUST_UNDERSTAND, // M
					IS_KEY, // K
					IS_DEFAULT, // D
					T1 = TRY_CONSTRUCT1,
					T2 = TRY_CONSTRUCT2,
					X = IS_EXTERNAL,
					M = IS_MUST_UNDERSTAND,
					K = IS_KEY,
					D = IS_DEFAULT
				};

				uint16_t value;

				MemberFlag& operator += (MemberFlagBit flag)
				{
					value |= flag;
					return *this;
				}

				MemberFlag& operator -= (MemberFlagBit flag)
				{
					value = value & (~flag);
					return *this;
				}

				bool operator & (MemberFlagBit flag)
				{
					return (value & flag) != 0;
				}
			};

			// T1, T2, X
			using CollectionElementFlag = MemberFlag;

			// T1, T2, O, M, K, X
			using StructMemberFlag = MemberFlag;

			// T1, T2, D, X
			using UnionMemberFlag = MemberFlag;

			// T1, T2, K
			using UnionDiscriminatorFlag = MemberFlag;

			// D
			using EnumeratedLiteralFlag = MemberFlag;

			using AnnotationParameterFlag = MemberFlag;

			using AliasMemberFlag = MemberFlag;

			using BitflagFlag = MemberFlag;

			using BitsetMemberFlag = MemberFlag;

			constexpr uint16_t MemberFlagMinimalMask = 0x003F;

			struct TypeFlag
			{
				enum TypeFlagBit
				{
					IS_FINAL,       // F
					IS_APPENDABLE,  // A
					IS_MUTABLE,     // M
					IS_NESTED,      // N
					IS_AUTOID_HASH, // H
				};

				TypeFlag()
					: value() {

				}

				uint16_t value;

				TypeFlag& operator += (TypeFlagBit flag)
				{
					value |= flag;
					return *this;
				}

				TypeFlag& operator -= (TypeFlagBit flag)
				{
					value = value & (~flag);
					return *this;
				}

				bool operator & (TypeFlagBit flag)
				{
					return (value & flag) != 0;
				}
			};

			using UnusedTypeFlag = TypeFlag;

			using StructTypeFlag = TypeFlag;
			using UnionTypeFlag = TypeFlag;
			using CollectionTypeFlag = UnusedTypeFlag;
			using AnnotationTypeFlag = UnusedTypeFlag;
			using AliasTypeFlag = UnusedTypeFlag;
			using EnumTypeFlag = UnusedTypeFlag;
			using BitmaskTypeFlag = UnusedTypeFlag;
			using BitsetTypeFlag = UnusedTypeFlag;

			constexpr uint16_t TypeFlagMinimalMask = 0x0007;

			class TypeIdentifier;

			struct StringSTypeDefn
			{
				SBound bound;
			};

			struct StringLTypeDefn
			{
				LBound bound;
			};

			struct PlainCollectionHeader
			{
				EquivalenceKind equivKind;
				CollectionElementFlag elementFlags;
			};

			struct PlainSequenceSElemDefn
			{
				PlainCollectionHeader header;
				SBoundSeq bound;
				// External
				External<TypeIdentifier> element_identifier;
			};

			struct PlainSequenceLElemDefn
			{
				PlainCollectionHeader header;
				LBoundSeq bound;
				// External
				External<TypeIdentifier> element_identifier;
			};

			template<typename TBoundSeq>
			struct PlainArrayTElemDefn
			{
				PlainCollectionHeader header;
				TBoundSeq arrayBoundSeq;
				// External
				External<TypeIdentifier> element_identifier;
				
			};


			using PlainArraySElemDefn = PlainArrayTElemDefn<SBoundSeq>;
			//struct PlainArraySElemDefn
			//{
			//	PlainCollectionHeader header;
			//	SBoundSeq arrayBoundSeq;
			//	// External
			//	External<TypeIdentifier> element_identifier;
			//};

			using PlainArrayLElemDefn = PlainArrayTElemDefn<LBoundSeq>;
			//struct PlainArrayLElemDefn
			//{
			//	PlainCollectionHeader header;
			//	LBoundSeq arrayBoundSeq;
			//	// External
			//	External<TypeIdentifier> element_identifier;
			//};

			struct PlainMapSTypeDefn
			{
				PlainCollectionHeader header;
				SBound bound;
				External<TypeIdentifier> element_identifier;
				CollectionElementFlag keyFlags;
				External<TypeIdentifier> keyIdentifier;
			};

			struct PlainMapLTypeDefn
			{
				PlainCollectionHeader header;
				LBound bound;
				External<TypeIdentifier> element_identifier;
				CollectionElementFlag keyFlags;
				External<TypeIdentifier> keyIdentifier;
			};

			struct StronglyConnectedComponentId
			{
				TypeObjectHashId sc_component_id;
				int32_t scc_length;
				int32_t scc_index;
			};

			template<>
			struct XTypesTrait<StronglyConnectedComponentId> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct ExtendedTypeDefn
			{
				// Empty. Available for future extension
			};

			template<>
			struct XTypesTrait<ExtendedTypeDefn> {
				static constexpr Extensibility extensibility = Extensibility::Mutable;
			};

			class TypeIdentifier {
				union Union {
					Union(Octet d) {
						switch (d) {
						case TI_STRING8_SMALL:
							new (&string_sdefn) StringSTypeDefn();
							break;
						case TI_STRING16_SMALL:
							new (&string_sdefn) StringSTypeDefn();
							break;
						case TI_STRING8_LARGE:
							new (&string_ldefn) StringLTypeDefn();
							break;
						case TI_STRING16_LARGE:
							new (&string_ldefn) StringLTypeDefn();
							break;
						case TI_PLAIN_SEQUENCE_SMALL:
							new (&seq_sdefn) PlainSequenceSElemDefn();
							break;
						case TI_PLAIN_SEQUENCE_LARGE:
							new (&seq_ldefn) PlainSequenceLElemDefn();
							break;
						case TI_PLAIN_ARRAY_SMALL:
							new (&array_sdefn) PlainArraySElemDefn();
							break;
						case TI_PLAIN_ARRAY_LARGE:
							new (&array_ldefn) PlainArrayLElemDefn();
							break;
						case TI_PLAIN_MAP_SMALL:
							new (&map_sdefn) PlainMapSTypeDefn();
							break;
						case TI_PLAIN_MAP_LARGE:
							new (&map_ldefn) PlainMapLTypeDefn();
							break;
						case TI_STRONGLY_CONNECTED_COMPONENT:
							new (&sc_component_id) StronglyConnectedComponentId();
							break;

							// ============  The remaining cases - use EquivalenceKind ========= 
						case EK_COMPLETE:
						case EK_MINIMAL:
							new (&equivalence_hash) EquivalenceHash();
							break;
						default:
							new (&extended_defn) ExtendedTypeDefn();
							break;
						}
					}

					Union(Octet d, const Union& rhs)
					{
						switch (d)
						{
							case TI_STRING8_SMALL:
								new (&string_sdefn) StringSTypeDefn(rhs.string_sdefn);
								break;
							case TI_STRING16_SMALL:
								new (&string_sdefn) StringSTypeDefn(rhs.string_sdefn);
								break;
							case TI_STRING8_LARGE:
								new (&string_ldefn) StringLTypeDefn(rhs.string_ldefn);
								break;
							case TI_STRING16_LARGE:
								new (&string_ldefn) StringLTypeDefn(rhs.string_ldefn);
								break;
							case TI_PLAIN_SEQUENCE_SMALL:
								new (&seq_sdefn) PlainSequenceSElemDefn(rhs.seq_sdefn);
								break;
							case TI_PLAIN_SEQUENCE_LARGE:
								new (&seq_ldefn) PlainSequenceLElemDefn(rhs.seq_ldefn);
								break;
							case TI_PLAIN_ARRAY_SMALL:
								new (&array_sdefn) PlainArraySElemDefn(rhs.array_sdefn);
								break;
							case TI_PLAIN_ARRAY_LARGE:
								new (&array_ldefn) PlainArrayLElemDefn(rhs.array_ldefn);
								break;
							case TI_PLAIN_MAP_SMALL:
								new (&map_sdefn) PlainMapSTypeDefn(rhs.map_sdefn);
								break;
							case TI_PLAIN_MAP_LARGE:
								new (&map_ldefn) PlainMapLTypeDefn(rhs.map_ldefn);
								break;
							case EK_COMPLETE:
							case EK_MINIMAL:
								new (&equivalence_hash) EquivalenceHash(rhs.equivalence_hash);
								break;
							case TI_STRONGLY_CONNECTED_COMPONENT:
								new (&sc_component_id) StronglyConnectedComponentId(rhs.sc_component_id);
								break;
						}
					}

					Union(Octet d, const EquivalenceHash& hash)
					{
						switch (d)
						{
							case EK_COMPLETE:
							case EK_MINIMAL:
								new (&equivalence_hash) EquivalenceHash(hash);
								break;
							default:
								throw std::runtime_error("");
						}
					}

					~Union() {}

					StringSTypeDefn string_sdefn;
					StringLTypeDefn string_ldefn;
					PlainSequenceSElemDefn seq_sdefn;
					PlainSequenceLElemDefn seq_ldefn;
					PlainArraySElemDefn array_sdefn;
					PlainArrayLElemDefn array_ldefn;
					PlainMapSTypeDefn map_sdefn;
					PlainMapLTypeDefn map_ldefn;
					StronglyConnectedComponentId sc_component_id;
					EquivalenceHash equivalence_hash;
					ExtendedTypeDefn extended_defn;
				};
			public:
				TypeIdentifier(Octet d)
					: _d{ d }
					, value(d) {
				}

				TypeIdentifier(const TypeIdentifier& rhs)
					: _d{ rhs._d }
					, value(rhs._d, rhs.value) {
				}

				TypeIdentifier(Octet d, const EquivalenceHash& hash)
									: _d{ d }
					, value(d, hash) {
				}

				Octet d() const {
					return _d;
				}

				void d(Octet discriminator) {
					if (_d == discriminator) {
						return;
					}
				
					this->~TypeIdentifier();
					new(this) TypeIdentifier(discriminator);
				}

				StringSTypeDefn& string_sdefn() {
					if (_d != TI_STRING8_SMALL && _d != TI_STRING16_SMALL) {
						throw std::runtime_error("");
					}
					return value.string_sdefn;
				}

				const StringSTypeDefn& string_sdefn() const {
					if (_d != TI_STRING8_SMALL && _d != TI_STRING16_SMALL) {
						throw std::runtime_error("");
					}
					return value.string_sdefn;
				}

				StringLTypeDefn& string_ldefn() {
					if (_d != TI_STRING8_LARGE && _d != TI_STRING16_LARGE) {
						throw std::runtime_error("");
					}
					return value.string_ldefn;
				}

				const StringLTypeDefn& string_ldefn() const {
					if (_d != TI_STRING8_LARGE && _d != TI_STRING16_LARGE) {
						throw std::runtime_error("");
					}
					return value.string_ldefn;
				}

				PlainSequenceSElemDefn& seq_sdefn() {
					if (_d != TI_PLAIN_SEQUENCE_SMALL) {
						throw std::runtime_error("");
					}
					return value.seq_sdefn;
				}

				const PlainSequenceSElemDefn& seq_sdefn() const {
					if (_d != TI_PLAIN_SEQUENCE_SMALL) {
						throw std::runtime_error("");
					}
					return value.seq_sdefn;
				}

				PlainSequenceLElemDefn& seq_ldefn() {
					if (_d != TI_PLAIN_SEQUENCE_LARGE) {
						throw std::runtime_error("");
					}
					return value.seq_ldefn;
				}

				const PlainSequenceLElemDefn& seq_ldefn() const {
					if (_d != TI_PLAIN_SEQUENCE_LARGE) {
						throw std::runtime_error("");
					}
					return value.seq_ldefn;
				}

				PlainArraySElemDefn& array_sdefn() {
					if (_d != TI_PLAIN_ARRAY_SMALL) {
						throw std::runtime_error("");
					}
					return value.array_sdefn;
				}

				const PlainArraySElemDefn& array_sdefn() const {
					if (_d != TI_PLAIN_ARRAY_SMALL) {
						throw std::runtime_error("");
					}
					return value.array_sdefn;
				}

				PlainArrayLElemDefn& array_ldefn() {
					if (_d != TI_PLAIN_ARRAY_LARGE) {
						throw std::runtime_error("");
					}
					return value.array_ldefn;
				}

				const PlainArrayLElemDefn& array_ldefn() const {
					if (_d != TI_PLAIN_ARRAY_LARGE) {
						throw std::runtime_error("");
					}
					return value.array_ldefn;
				}

				PlainMapSTypeDefn& map_sdefn() {
					if (_d != TI_PLAIN_MAP_SMALL) {
						throw std::runtime_error("");
					}
					return value.map_sdefn;
				}

				const PlainMapSTypeDefn& map_sdefn() const {
					if (_d != TI_PLAIN_MAP_SMALL) {
						throw std::runtime_error("");
					}
					return value.map_sdefn;
				}

				PlainMapLTypeDefn& map_ldefn() {
					if (_d != TI_PLAIN_MAP_LARGE) {
						throw std::runtime_error("");
					}
					return value.map_ldefn;
				}

				const PlainMapLTypeDefn& map_ldefn() const {
					if (_d != TI_PLAIN_MAP_LARGE) {
						throw std::runtime_error("");
					}
					return value.map_ldefn;
				}

				StronglyConnectedComponentId& sc_component_id() {
					if (_d != TI_STRONGLY_CONNECTED_COMPONENT) {
						throw std::runtime_error("");
					}
					return value.sc_component_id;
				}

				const StronglyConnectedComponentId& sc_component_id() const {
					if (_d != TI_STRONGLY_CONNECTED_COMPONENT) {
						throw std::runtime_error("");
					}
					return value.sc_component_id;
				}

				const EquivalenceHash& equivalence_hash() const {
					if (_d != EK_COMPLETE && _d != EK_MINIMAL) {
						throw std::runtime_error("");
					}

					return value.equivalence_hash;
				}

				const ExtendedTypeDefn& extended_defn() const {
					switch (_d) {
					case TI_STRING8_SMALL:
					case TI_STRING16_SMALL:
					case TI_STRING8_LARGE:
					case TI_STRING16_LARGE:
					case TI_PLAIN_SEQUENCE_SMALL:
					case TI_PLAIN_SEQUENCE_LARGE:
					case TI_PLAIN_ARRAY_SMALL:
					case TI_PLAIN_ARRAY_LARGE:
					case TI_PLAIN_MAP_SMALL:
					case TI_PLAIN_MAP_LARGE:
					case TI_STRONGLY_CONNECTED_COMPONENT:
						throw std::runtime_error("");
					}

					return value.extended_defn;
				}

				ExtendedTypeDefn& extended_defn() {
					switch (_d) {
					case TI_STRING8_SMALL:
					case TI_STRING16_SMALL:
					case TI_STRING8_LARGE:
					case TI_STRING16_LARGE:
					case TI_PLAIN_SEQUENCE_SMALL:
					case TI_PLAIN_SEQUENCE_LARGE:
					case TI_PLAIN_ARRAY_SMALL:
					case TI_PLAIN_ARRAY_LARGE:
					case TI_PLAIN_MAP_SMALL:
					case TI_PLAIN_MAP_LARGE:
					case TI_STRONGLY_CONNECTED_COMPONENT:
						throw std::runtime_error("");
					}

					return value.extended_defn;
				}

				~TypeIdentifier() {
					switch (_d) {
					case TI_STRING8_SMALL:
					case TI_STRING16_SMALL:
						value.string_sdefn.~StringSTypeDefn();
						break;
					case TI_STRING8_LARGE:
					case TI_STRING16_LARGE:
						value.string_ldefn.~StringLTypeDefn();
						break;
					case TI_PLAIN_SEQUENCE_SMALL:
						value.seq_sdefn.~PlainSequenceSElemDefn();
						break;
					case TI_PLAIN_SEQUENCE_LARGE:
						value.seq_ldefn.~PlainSequenceLElemDefn();
						break;
					case TI_PLAIN_ARRAY_SMALL:
						value.array_sdefn.~PlainArraySElemDefn();
						break;
					case TI_PLAIN_ARRAY_LARGE:
						value.array_ldefn.~PlainArrayLElemDefn();
						break;
					case TI_PLAIN_MAP_SMALL:
						value.map_sdefn.~PlainMapSTypeDefn();
						break;
					case TI_PLAIN_MAP_LARGE:
						value.map_ldefn.~PlainMapLTypeDefn();
						break;
					case TI_STRONGLY_CONNECTED_COMPONENT:
						value.sc_component_id.~StronglyConnectedComponentId();
						break;
					}
				}

			private:
				Octet _d;
				Union value;
			};

			//struct TypeIdentifier
			//{
			//	Octet _d;
			//	// TI_STRING8_SMALL
			//	// TI_STRING16_SMALL
			//	StringSTypeDefn string_sdefn;

			//	// TI_STRING8_LARGE
			//	// TI_STRING16_LARGE
			//	StringLTypeDefn string_ldefn;

			//	// TI_PLAIN_SEQUENCE_SMALL
			//	PlainSequenceSElemDefn seq_sdefn;

			//	// TI_PLAIN_SEQEUENCE_LARGE
			//	PlainSequenceLElemDefn seq_ldefn;

			//	// TI_PLAIN_ARRAY_SMALL
			//	PlainArraySElemDefn array_sdefn;

			//	// TI_PLAIN_ARRAY_LARGE
			//	PlainArrayLElemDefn array_ldefn;

			//	// TI_PLAIN_MAP_SMALL
			//	PlainMapSTypeDefn map_sdefn;

			//	// TI_PLAIN_MAP_LARGE
			//	PlainMapLTypeDefn map_ldefn;

			//	// TI_STRONGLY_CONNECTED_COMPONENT
			//	StronglyConnectedComponentId sc_component_id;

			//	// EK_COMPLETE
			//	// EK_MINIMAL
			//	EquivalenceHash equivalence_hash;

			//	// Else
			//	ExtendedTypeDefn extended_defn;
			//};

			using MemberId = UnsignedLong;

			struct ExtendedAnnotationParameterValue
			{
				// Empty. Avaliable for future extension.
			};

			template<>
			struct XTypesTrait<ExtendedAnnotationParameterValue> {
				static constexpr Extensibility extensibility = Extensibility::Mutable;
			};

			// Liternal value of an annotation member: either the default value in its
			// definition of the value applied in its usage
			class AnnotationParameterValue {
				union Union {
					Union(Octet d) {
						switch (d) {
						case TK_STRING8:
							new (&string8_value) String<ANNOTATION_STR_VALUE_MAX_LEN>();
							break;

						case TK_STRING16:
							new (&string16_value) WideString<ANNOTATION_STR_VALUE_MAX_LEN>();
							break;

						case TK_FLOAT32:
							float32_value = 0.f;
							break;

						case TK_FLOAT64:
							float64_value = 0.;
							break;

						default:
							uint64_value = 0;
							break;
						}
					}

					Union(Octet d, const Union& rhs) {
						switch (d) {
						case TK_STRING8:
							new (&string8_value) String<ANNOTATION_STR_VALUE_MAX_LEN>();
							break;

						case TK_STRING16:
							new (&string16_value) WideString<ANNOTATION_STR_VALUE_MAX_LEN>();
							break;
						default:
							uint64_value = rhs.uint64_value;
							break;
						}
					}

					~Union() {}

					Boolean boolean_value;
					Octet byte_value;
					Short int16_value;
					UnsignedShort uint16_value;
					Long int32_value;
					UnsignedLong uint32_value;
					LongLong int64_value;
					UnsignedLongLong uint64_value;
					Float float32_value;
					Double float64_value;
					Char char_value;
					WideChar wchar_value;
					Long enumerated_value;
					String<ANNOTATION_STR_VALUE_MAX_LEN> string8_value;
					WideString<ANNOTATION_OCTETSEC_VALUE_MAX_LEN> string16_value;
				};

			public:
				AnnotationParameterValue(Octet d)
					: _d{ d }
					, value(d) {
				}

				AnnotationParameterValue(const AnnotationParameterValue& rhs)
					: _d{ rhs._d }
					, value(rhs._d, rhs.value) {

				}

				Octet d() const {
					return _d;
				}

				void d(Octet discriminator) {
					if (_d == discriminator) {
						return;
					}

					this->~AnnotationParameterValue();
					new(this) AnnotationParameterValue(discriminator);
				}

				template<typename T>
				void d(Octet discriminator, T&& rhs) {
					if (_d == discriminator) {
						return;
					}

					this->~AnnotationParameterValue();
					new(this) AnnotationParameterValue(discriminator, std::forward<T>(rhs));
				}

				Boolean& boolean_value() {
					if (_d != TK_BOOLEAN) {
						throw std::runtime_error("");
					}

					return value.boolean_value;
				}

				Octet& byte_value() {
					if (_d != TK_BYTE) {
						throw std::runtime_error("");
					}

					return value.byte_value;
				}

				Short& int16_value() {
					if (_d != TK_INT16) {
						throw std::runtime_error("");
					}

					return value.int16_value;
				}

				UnsignedShort& uint16_value() {
					if (_d != TK_UINT16) {
						throw std::runtime_error("");
					}

					return value.uint16_value;
				}

				Long& int32_value() {
					if (_d != TK_INT32) {
						throw std::runtime_error("");
					}

					return value.int32_value;
				}

				UnsignedLong& uint32_value() {
					if (_d != TK_UINT32) {
						throw std::runtime_error("");
					}

					return value.uint32_value;
				}

				LongLong& int64_value() {
					if (_d != TK_INT64) {
						throw std::runtime_error("");
					}

					return value.int64_value;
				}

				UnsignedLongLong& uint64_value() {
					if (_d != TK_UINT64) {
						throw std::runtime_error("");
					}

					return value.uint64_value;
				}

				Float& float32_value() {
					if (_d != TK_FLOAT32) {
						throw std::runtime_error("");
					}

					return value.float32_value;
				}

				Double& float64_value() {
					if (_d != TK_FLOAT64) {
						throw std::runtime_error("");
					}

					return value.float64_value;
				}

				Char& char_value() {
					if (_d != TK_CHAR8) {
						throw std::runtime_error("");
					}

					return value.char_value;
				}

				WideChar& wchar_value() {
					if (_d != TK_CHAR16) {
						throw std::runtime_error("");
					}

					return value.wchar_value;
				}

				Long& enumerated_value() {
					if (_d != TK_ENUM) {
						throw std::runtime_error("");
					}

					return value.enumerated_value;
				}

				String<ANNOTATION_OCTETSEC_VALUE_MAX_LEN>& string8_value() {
					if (_d != TK_STRING8) {
						throw std::runtime_error("");
					}

					return value.string8_value;
				}

				WideString<ANNOTATION_OCTETSEC_VALUE_MAX_LEN>& string16_value() {
					if (_d != TK_STRING16) {
						throw std::runtime_error("");
					}
				}

				~AnnotationParameterValue() {
					switch (_d) {
					case TK_STRING8:
						value.string8_value.~basic_string();
						break;
					case TK_STRING16:
						value.string16_value.~basic_string();
						break;
					}
				}

				Octet _d;
				Union value;
			};

			struct AppliedAnnotationParameter
			{
				NameHash paramname_hash;
				AnnotationParameterValue value;
			};

			template<>
			struct XTypesTrait<AppliedAnnotationParameter> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			using AppliedAnnotationParameterSeq = BoundedSequence<AppliedAnnotationParameter>;

			struct AppliedAnnotation
			{
				TypeIdentifier annotation_typeid;
				Optional<AppliedAnnotationParameterSeq> param_seq;
			};

			template<>
			struct XTypesTrait<AppliedAnnotation> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			using AppliedAnnotationSeq = BoundedSequence<AppliedAnnotation>;

			// @verbatim(placement="<placement>", language="<lang>", text="<text>")
			struct AppliedVerbatimAnnotation
			{
				std::string/*<32>*/ placement;
				std::string/*<32>*/ language;
				std::string text;
			};

			struct AppliedBuiltinMemberAnnotations
			{
				Optional<std::string> unit; // @unit("<unit>")
				Optional<AnnotationParameterValue> min; // @min , @range
				Optional<AnnotationParameterValue> max; // @max , @range
				Optional<std::string> hash_id; // @hash_id("<membername>")
			};

			template<>
			struct XTypesTrait<AppliedBuiltinMemberAnnotations> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CommonStructMember
			{
				MemberId member_id;
				StructMemberFlag member_flags;
				TypeIdentifier member_type_id;
			};

			struct CompleteMemberDetail
			{
				MemberName type_name;
				Optional<AppliedBuiltinMemberAnnotations> ann_builtin;
				Optional<AppliedAnnotationSeq> ann_custom;
			};

			struct MinimalMemberDetail
			{
				NameHash name_hash;
			};

			struct CompleteStructMember
			{
				CommonStructMember common;
				CompleteMemberDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteStructMember> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			using CompleteStructMemberSeq = std::vector<CompleteStructMember>;

			struct MinimalStructmMember
			{
				CommonStructMember common;
				MinimalMemberDetail detail;
			};

			template<>
			struct XTypesTrait<MinimalStructmMember> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			// Ordered by common.member_id
			using MinimalStructMemberSeq = std::vector<MinimalStructmMember>;

			struct AppliedBuiltinTypeAnnotations {
				Optional<AppliedVerbatimAnnotation> verbatim;  // @verbatim(...)
			};

			template<>
			struct XTypesTrait<AppliedBuiltinTypeAnnotations> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalTypeDetail
			{
				// Empty. Available for future extension
			};

			struct CompleteTypeDetail
			{
				Optional<AppliedBuiltinTypeAnnotations> ann_builtin;
				Optional<AppliedAnnotationSeq> ann_custom;
				QualifiedTypeName type_name;
			};

			struct CompleteStructHeader
			{
				TypeIdentifier base_type;
				CompleteTypeDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteStructHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalStructHeader
			{
				TypeIdentifier base_type;
				MinimalTypeDetail detail;
			};

			template<>
			struct XTypesTrait<MinimalStructHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteStructType
			{
				StructTypeFlag struct_flags;
				CompleteStructHeader header;
				CompleteStructMemberSeq member_seq;
			};

			struct MinimalStructType
			{
				StructTypeFlag struct_flags;
				MinimalStructHeader header;
				MinimalStructMemberSeq member_seq;
			};

			using UnionCaseLabelSeq = std::vector<int32_t>;

			struct CommonUnionMember
			{
				MemberId member_id;
				UnionMemberFlag member_flags;
				TypeIdentifier type_id;
				UnionCaseLabelSeq label_seq;
			};

			// Member of a union type
			struct CompleteUnionMember
			{
				CommonUnionMember common;
				CompleteMemberDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteUnionMember> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			// Ordered by member_index
			using CompleteUnionMemberSeq = BoundedSequence<CompleteUnionMember>;

			struct MinimalUnionMember
			{
				CommonUnionMember common;
				MinimalMemberDetail detail;
			};

			template<>
			struct XTypesTrait<MinimalUnionMember> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			// Ordered by MinimalUnionMember.common.member_id
			using MinimalUnionMemberSeq = BoundedSequence<MinimalUnionMember>;

			struct CommonDiscriminatorMember
			{
				UnionDiscriminatorFlag member_flags;
				TypeIdentifier type_id;
			};

			struct CompleteDiscriminatorMember
			{
				CommonDiscriminatorMember common;
				Optional<AppliedBuiltinTypeAnnotations> ann_builtin;
				Optional<AppliedAnnotationSeq> ann_custom;
			};

			template<>
			struct XTypesTrait<CompleteDiscriminatorMember> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalDiscriminatorMember
			{
				CommonDiscriminatorMember common;
			};

			template<>
			struct XTypesTrait<MinimalDiscriminatorMember> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteUnionHeader
			{
				CompleteTypeDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteUnionHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalUnionHeader
			{
				MinimalTypeDetail detail;
			};

			template<>
			struct XTypesTrait<MinimalUnionHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteUnionType
			{
				UnionTypeFlag union_flags;
				CompleteUnionHeader header;
				CompleteDiscriminatorMember discriminator;
				CompleteUnionMemberSeq member_seq;
			};

			struct MinimalUnionType
			{
				UnionTypeFlag union_flags;
				MinimalUnionHeader header;
				MinimalDiscriminatorMember discriminator;
				MinimalUnionMemberSeq member_seq;
			};

			struct CommonAnnotationParameter
			{
				AnnotationParameterFlag member_flags;
				TypeIdentifier member_type_id;
			};

			struct CompleteAnnotationParameter
			{
				CommonAnnotationParameter common;
				MemberName type_name;
				AnnotationParameterValue default_value;
			};

			template<>
			struct XTypesTrait<CompleteAnnotationParameter> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			// Ordered by CompleteAnnotationParameter.name
			using CompleteAnnotationParameterSeq = BoundedSequence<CompleteAnnotationParameter>;

			struct MinimalAnnotationParameter
			{
				CommonAnnotationParameter common;
				NameHash name_hash;
				AnnotationParameterValue default_value;
			};

			template<>
			struct XTypesTrait<MinimalAnnotationParameter> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			// Ordered by MinimalAnnotationParameter.name_hash
			using MinimalAnnotationParameterSeq = BoundedSequence<MinimalAnnotationParameter>;

			struct CompleteAnnotationHeader
			{
				QualifiedTypeName type_name;
			};

			template<>
			struct XTypesTrait<CompleteAnnotationHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalAnnotationHeader
			{
				// Empty. Available for future extension
			};

			template<>
			struct XTypesTrait<MinimalAnnotationHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteAnnotationType
			{
				AnnotationTypeFlag annotation_flags;
				CompleteAnnotationHeader header;
				CompleteAnnotationParameterSeq member_seq;
			};

			struct MinimalAnnotationType
			{
				AnnotationTypeFlag annotation_flags;
				MinimalAnnotationHeader header;
				MinimalAnnotationParameterSeq member_seq;
			};

			struct CommonAliasBody
			{
				CommonAliasBody(const CommonAliasBody& rhs)
					: related_flags{ rhs.related_flags }
					, related_type{ rhs.related_type }
				{
				}

				AliasMemberFlag related_flags;
				TypeIdentifier related_type;
			};

			struct CompleteAliasBody
			{
				CompleteAliasBody(const CompleteAliasBody& rhs)
					: common{ rhs.common }
					, ann_builtin{ rhs.ann_builtin }
					, ann_custom{ rhs.ann_custom }
				{

				}

				CommonAliasBody common;
				Optional<AppliedBuiltinMemberAnnotations> ann_builtin;
				Optional<AppliedAnnotationSeq> ann_custom;
			};

			template<>
			struct XTypesTrait<CompleteAliasBody> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalAliasBody
			{
				CommonAliasBody common;
			};

			template<>
			struct XTypesTrait<MinimalAliasBody> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteAliasHeader
			{
				CompleteTypeDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteAliasHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalAliasHeader
			{
				// Empty. Available for future extension
			};

			template<>
			struct XTypesTrait<MinimalAliasHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteAliasType
			{
				CompleteAliasType(const CompleteAliasType& rhs)
					: alias_flags{ rhs.alias_flags }
					, header{ rhs.header }
					, body{ rhs.body }
				{

				}

				AliasTypeFlag alias_flags;
				CompleteAliasHeader header;
				CompleteAliasBody body;
			};

			struct MinimalAliasType
			{
				AliasTypeFlag alias_flags;
				MinimalAliasHeader header;
				MinimalAliasBody body;
			};

			struct CompleteElementDetail
			{
				Optional<AppliedBuiltinMemberAnnotations> ann_builtin;
				Optional<AppliedAnnotationSeq> ann_custom;
			};

			struct CommonCollectionElement
			{
				CollectionElementFlag element_flags;
				TypeIdentifier type;
			};

			struct CompleteCollectionElement
			{
				CommonCollectionElement common;
				CompleteElementDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteCollectionElement> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalCollectionElement
			{
				CommonCollectionElement common;
			};

			template<>
			struct XTypesTrait<MinimalCollectionElement> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CommonCollectionHeader
			{
				LBound bound;
			};

			template<>
			struct XTypesTrait<CommonCollectionHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteCollectionHeader
			{
				CommonCollectionHeader common;
				Optional<CompleteTypeDetail> detail;
			};

			struct MinimalCollectionHeader
			{
				CommonCollectionHeader common;
			};

			struct CompleteSequenceType
			{
				CollectionTypeFlag collection_flags;
				CompleteCollectionHeader header;
				CompleteCollectionElement element;
			};

			struct MinimalSequenceType
			{
				CollectionTypeFlag collection_flags;
				MinimalCollectionHeader header;
				MinimalCollectionElement element;
			};

			struct CommonArrayHeader
			{
				LBoundSeq bound_seq;
			};

			struct CompleteArrayHeader
			{
				CommonArrayHeader common;
				CompleteTypeDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteArrayHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalArrayHeader
			{
				CommonArrayHeader common;
			};

			template<>
			struct XTypesTrait<MinimalArrayHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteArrayType
			{
				CollectionTypeFlag collection_flags;
				CompleteArrayHeader header;
				CompleteCollectionElement element;
			};

			template<>
			struct XTypesTrait<CompleteArrayType> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalArrayType
			{
				CollectionTypeFlag collection_flags;
				MinimalArrayHeader header;
				MinimalCollectionElement element;
			};

			struct CompleteMapType
			{
				CollectionTypeFlag collection_flags;
				CompleteCollectionHeader header;
				CompleteCollectionElement key;
				CompleteCollectionElement element;
			};

			struct MinimalMapType
			{
				CollectionTypeFlag collection_flags;
				MinimalCollectionHeader header;
				MinimalCollectionElement key;
				MinimalCollectionElement element;
			};

			using BitBound = uint16_t;

			struct CommonEnumeratedLiteral
			{
				int32_t value;
				EnumeratedLiteralFlag flags;
			};

			template<>
			struct XTypesTrait<CommonEnumeratedLiteral> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteEnumeratedLiteral
			{
				CommonEnumeratedLiteral common;
				CompleteMemberDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteEnumeratedLiteral> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			using CompleteEnumeratedLiteralSeq = BoundedSequence<CompleteEnumeratedLiteral>;

			struct MinimalEnumeratedLiteral
			{
				CommonEnumeratedLiteral common;
				MinimalMemberDetail detail;
			};

			template<>
			struct XTypesTrait<MinimalEnumeratedLiteral> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			using MinimalEnumeratedLiteralSeq = BoundedSequence<MinimalEnumeratedLiteral>;

			struct CommonEnumeratedHeader
			{
				BitBound bound;
			};

			struct CompleteEnumeratedHeader
			{
				CommonEnumeratedHeader common;
				CompleteTypeDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteEnumeratedHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalEnumeratedHeader
			{
				CommonEnumeratedHeader common;
			};

			template<>
			struct XTypesTrait<MinimalEnumeratedHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteEnumeratedType
			{
				// Unused
				EnumTypeFlag enum_flags;
				CompleteEnumeratedHeader header;
				CompleteEnumeratedLiteralSeq literal_seq;
			};

			struct MinimalEnumeratedType
			{
				// Unused
				EnumTypeFlag enum_flags;
				MinimalEnumeratedHeader header;
				MinimalEnumeratedLiteralSeq literal_seq;
			};

			struct CommonBitflag
			{
				uint16_t position;
				BitflagFlag flags;
			};

			struct CompleteBitflag
			{
				CommonBitflag common;
				CompleteMemberDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteBitflag> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			using CompleteBitflagSeq = BoundedSequence<CompleteBitflag>;

			struct MinimalBitflag
			{
				CommonBitflag common;
				MinimalMemberDetail detail;
			};

			template<>
			struct XTypesTrait<MinimalBitflag> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			using MinimalBitflagSeq = BoundedSequence<MinimalBitflag>;

			struct CommonBitmaskHeader
			{
				BitBound bound;
			};

			using CompleteBitmaskHeader = CompleteEnumeratedHeader;

			using MinimalBitmaskHeader = MinimalEnumeratedHeader;

			struct CompleteBitmaskType
			{
				// Unused
				BitmaskTypeFlag bitmask_flags;
				CompleteBitmaskHeader header;
				CompleteBitflagSeq bitflag_seq;
			};

			template<>
			struct XTypesTrait<CompleteBitmaskType> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalBitmaskType
			{
				// Unused
				BitmaskTypeFlag bitmask_flags;
				MinimalBitmaskHeader header;
				MinimalBitflagSeq bitflag_seq;
			};

			template<>
			struct XTypesTrait<MinimalBitmaskType> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CommonBitfield
			{
				uint16_t position;
				BitsetMemberFlag flags;
				uint8_t bitcount;
				// Must be primitive integer type
				TypeKind holder_type;
			};

			template<>
			struct XTypesTrait<CommonBitfield> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteBitfield
			{
				CommonBitfield common;
				CompleteMemberDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteBitfield> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			// Ordered by Bitfield.position
			using CompleteBitfieldSeq = BoundedSequence<CompleteBitfield>;

			struct MinimalBitfield
			{
				CommonBitfield common;
				MinimalMemberDetail detail;
			};

			// Ordered by Bitfield.position
			using MinimalBitfieldSeq = BoundedSequence<MinimalBitfield>;

			struct CompleteBitsetHeader
			{
				CompleteTypeDetail detail;
			};

			template<>
			struct XTypesTrait<CompleteBitsetHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalBitsetHeader
			{
				// Empty. Available for future extension
			};

			template<>
			struct XTypesTrait<MinimalBitsetHeader> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteBitsetType
			{
				// Unused
				BitsetTypeFlag bitset_flags;
				CompleteBitsetHeader header;
				CompleteBitfieldSeq field_seq;
			};

			template<>
			struct XTypesTrait<CompleteBitsetType> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct MinimalBitsetType
			{
				// Unused
				BitsetTypeFlag bitset_flags;
				MinimalBitsetHeader header;
				MinimalBitfieldSeq field_seq;
			};

			template<>
			struct XTypesTrait<MinimalBitsetType> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct CompleteExtendedType
			{
				// Empty. Available for future extension
			};

			template<>
			struct XTypesTrait<CompleteExtendedType> {
				static constexpr Extensibility extensibility = Extensibility::Mutable;
			};

			class CompleteTypeObject {
				union Union {
					Union(Octet d) {
						switch (d) {
						case TK_ALIAS:
							assert(d != TK_ALIAS);
							break;
						case TK_ANNOTATION:
							new (&annotation_type) CompleteAnnotationType();
							break;
						case TK_STRUCTURE:
							assert(d != TK_STRUCTURE);
							break;
						case TK_UNION:
							assert(d != TK_UNION);
							break;
						case TK_BITSET:
							new (&bitset_type) CompleteBitsetType();
							break;
						case TK_SEQUENCE:
							assert(d != TK_SEQUENCE);
							break;
						case TK_ARRAY:
							assert(d != TK_ARRAY);
							break;
						case TK_MAP:
							assert(d != TK_MAP);
							break;
						case TK_ENUM:
							new (&enumerated_type) CompleteEnumeratedType();
							break;
						case TK_BITMASK:
							new (&bitmask_type) CompleteBitmaskType();
							break;
						default:
							new (&extended_type) CompleteExtendedType();
							break;
						}
					}

					Union(Octet d, const Union& rhs) {
						switch (d) {
						case TK_ALIAS:
							new (&alias_type) CompleteAliasType(rhs.alias_type);
							break;
						case TK_ANNOTATION:
							new (&annotation_type) CompleteAnnotationType(rhs.annotation_type);
							break;
						case TK_STRUCTURE:
							new (&struct_type) CompleteStructType(rhs.struct_type);
							break;
						case TK_UNION:
							new (&union_type) CompleteUnionType(rhs.union_type);
							break;
						case TK_BITSET:
							new (&bitset_type) CompleteBitsetType(rhs.bitset_type);
							break;
						case TK_SEQUENCE:
							new (&sequence_type) CompleteSequenceType(rhs.sequence_type);
							break;
						case TK_ARRAY:
							new (&array_type) CompleteArrayType(rhs.array_type);
							break;
						case TK_MAP:
							new (&map_type) CompleteMapType(rhs.map_type);
							break;
						case TK_ENUM:
							new (&enumerated_type) CompleteEnumeratedType(rhs.enumerated_type);
							break;
						case TK_BITMASK:
							new (&bitmask_type) CompleteBitmaskType(rhs.bitmask_type);
							break;
						default:
							new (&extended_type) CompleteExtendedType(rhs.extended_type);
							break;
						}
					}

					Union(const CompleteAliasType& rhs)
						: alias_type{ rhs } {
					}

					Union(const CompleteStructType& rhs)
						: struct_type{ rhs } {
					}

					Union(const CompleteUnionType& rhs)
						: union_type{ rhs } {
					}

					Union(const CompleteSequenceType& rhs)
						: sequence_type{ rhs } {
					}

					Union(const CompleteArrayType& rhs)
						: array_type{ rhs } {
					}

					Union(const CompleteMapType& rhs)
						: map_type{ rhs } {
					}


					~Union() {}

					CompleteAliasType alias_type;
					CompleteAnnotationType annotation_type;
					CompleteStructType struct_type;
					CompleteUnionType union_type;
					CompleteBitsetType bitset_type;
					CompleteSequenceType sequence_type;
					CompleteArrayType array_type;
					CompleteMapType map_type;
					CompleteEnumeratedType enumerated_type;
					CompleteBitmaskType bitmask_type;
					CompleteExtendedType extended_type;
				};
			public:
				CompleteTypeObject(Octet d)
					: _d{ d }
					, value(d) {
				}

				CompleteTypeObject(const CompleteTypeObject& rhs)
					: _d{ rhs._d }
					, value(rhs._d, rhs.value) {
				}

				CompleteTypeObject(const CompleteAliasType& rhs)
					: _d{ TK_ALIAS }
					, value(rhs) {
				}

				CompleteTypeObject(const CompleteStructType& rhs)
					: _d{ TK_STRUCTURE }
					, value(rhs) {
				}

				CompleteTypeObject(const CompleteUnionType& rhs)
					: _d{ TK_UNION }
					, value(rhs) {
				}

				void d(Octet discriminator) {
					if (_d == discriminator) {
						return;
					}

					this->~CompleteTypeObject();
					new(this) CompleteTypeObject(discriminator);
				}

				void d(Octet discriminator, const CompleteAliasType& rhs) {
					if (_d == discriminator) {
						return;
					}

					if (discriminator != TK_ALIAS) {
						throw std::runtime_error("");
					}

					this->~CompleteTypeObject();
					new(this) CompleteTypeObject(rhs);
				}

				void d(Octet discriminator, const CompleteStructType& rhs) {
					if (_d == discriminator) {
						return;
					}

					if (discriminator != TK_STRUCTURE) {
						throw std::runtime_error("");
					}
					
					this->~CompleteTypeObject();
					new(this) CompleteTypeObject(rhs);
				}

				void d(Octet discriminator, const CompleteUnionType& rhs) {
					if (_d == discriminator) {
						return;
					}
					if (discriminator != TK_UNION) {
						throw std::runtime_error("");
					}
					this->~CompleteTypeObject();
					new(this) CompleteTypeObject(rhs);
				}

				Octet d() const {
					return _d;
				}

				CompleteAliasType& alias_type() {
					if (_d != TK_ALIAS) {
						throw std::runtime_error("");
					}
					return value.alias_type;
				}

				const CompleteAliasType& alias_type() const {
					if (_d != TK_ALIAS) {
						throw std::runtime_error("");
					}
					return value.alias_type;
				}

				CompleteAnnotationType& annotation_type() {
					if (_d != TK_ANNOTATION) {
						throw std::runtime_error("");
					}
					return value.annotation_type;
				}

				const CompleteAnnotationType& annotation_type() const {
					if (_d != TK_ANNOTATION) {
						throw std::runtime_error("");
					}
					return value.annotation_type;
				}

				CompleteStructType& struct_type() {
					if (_d != TK_STRUCTURE) {
						throw std::runtime_error("");
					}
					return value.struct_type;
				}

				const CompleteStructType& struct_type() const {
					if (_d != TK_STRUCTURE) {
						throw std::runtime_error("");
					}
					return value.struct_type;
				}

				CompleteUnionType& union_type() {
					if (_d != TK_UNION) {
						throw std::runtime_error("");
					}
					return value.union_type;
				}

				const CompleteUnionType& union_type() const {
					if (_d != TK_UNION) {
						throw std::runtime_error("");
					}
					return value.union_type;
				}

				CompleteBitsetType& bitset_type() {
					if (_d != TK_BITSET) {
						throw std::runtime_error("");
					}
					return value.bitset_type;
				}

				const CompleteBitsetType& bitset_type() const {
					if (_d != TK_BITSET) {
						throw std::runtime_error("");
					}
					return value.bitset_type;
				}

				CompleteSequenceType& sequence_type() {
					if (_d != TK_SEQUENCE) {
						throw std::runtime_error("");
					}
					return value.sequence_type;
				}

				const CompleteSequenceType& sequence_type() const {
					if (_d != TK_SEQUENCE) {
						throw std::runtime_error("");
					}
					return value.sequence_type;
				}

				CompleteArrayType& array_type() {
					if (_d != TK_ARRAY) {
						throw std::runtime_error("");
					}
					return value.array_type;
				}

				const CompleteArrayType& array_type() const {
					if (_d != TK_ARRAY) {
						throw std::runtime_error("");
					}
					return value.array_type;
				}

				CompleteMapType& map_type() {
					if (_d != TK_MAP) {
						throw std::runtime_error("");
					}
					return value.map_type;
				}

				const CompleteMapType& map_type() const {
					if (_d != TK_MAP) {
						throw std::runtime_error("");
					}
					return value.map_type;
				}

				CompleteEnumeratedType& enumerated_type() {
					if (_d != TK_ENUM) {
						throw std::runtime_error("");
					}
					return value.enumerated_type;
				}

				const CompleteEnumeratedType& enumerated_type() const {
					if (_d != TK_ENUM) {
						throw std::runtime_error("");
					}
					return value.enumerated_type;
				}

				CompleteBitmaskType& bitmask_type() {
					if (_d != TK_BITMASK) {
						throw std::runtime_error("");
					}
					return value.bitmask_type;
				}

				const CompleteBitmaskType& bitmask_type() const {
					if (_d != TK_BITMASK) {
						throw std::runtime_error("");
					}
					return value.bitmask_type;
				}

				~CompleteTypeObject() {
					switch (_d) {
					case TK_ANNOTATION:
						value.annotation_type.~CompleteAnnotationType();
						break;
					case TK_BITSET:
						value.bitset_type.~CompleteBitsetType();
						break;
					case TK_ENUM:
						value.enumerated_type.~CompleteEnumeratedType();
						break;
					case TK_BITMASK:
						value.bitmask_type.~CompleteBitmaskType();
						break;
					default:
						// Others are trivially destructible
						break;
					}
				}

			private:
				Octet _d;
				Union value;
			};

			struct MinimalExtendedType
			{
				// Empty. Available for future extension
			};

			template<>
			struct XTypesTrait<MinimalExtendedType> {
				static constexpr Extensibility extensibility = Extensibility::Mutable;
			};

			struct MinimalTypeObject
			{
				uint8_t _d;

				// TK_ALIAS
				MinimalAliasType alias_type;
				// TK_ANNOTATION
				MinimalAnnotationType annotation_type;
				// TK_STRUCTURE
				MinimalStructType struct_type;
				// TK_UNION
				MinimalUnionType union_type;
				// TK_BITSET
				MinimalBitsetType bitset_type;
				// TK_SEQEUNCE
				MinimalSequenceType sequence_type;
				// TK_ARRAY
				MinimalArrayType array_type;
				// TK_MAP
				MinimalMapType map_type;
				// TK_ENUM
				MinimalEnumeratedType enumerated_type;
				// TK_BITMASK
				MinimalBitmaskType bitmask_type;
				// Else
				MinimalExtendedType extended_type;
			};

			class TypeObject
			{
				union Union {
					Union(const CompleteTypeObject& other)
						: complete_type{ other } {
					}

					Union(const MinimalTypeObject& other)
						: minimal_type{ other } {
					}

					Union(Octet d, const Union& rhs) {
						switch (d) {
						case EK_COMPLETE:
							new (&complete_type) CompleteTypeObject(rhs.complete_type);
							break;
						case EK_MINIMAL:
							new (&minimal_type) MinimalTypeObject(rhs.minimal_type);
							break;
						}
					}
					~Union() {}
					CompleteTypeObject complete_type;
					MinimalTypeObject minimal_type;
				};
			public:
				TypeObject(Octet d)
					: _d{ d }
					, value(d, value) {
				}
				TypeObject(const CompleteTypeObject& other)
					: _d{ EK_COMPLETE }
					, value(other) {
				}
				TypeObject(const MinimalTypeObject& other)
					: _d{ EK_MINIMAL }
					, value(other) {
				}
				void d(Octet discriminator) {
					if (_d == discriminator) {
						return;
					}
					this->~TypeObject();
					new(this) TypeObject(discriminator);
				}
				void d(Octet discriminator, const CompleteTypeObject& rhs) {
					if (_d == discriminator) {
						return;
					}
					if (discriminator != EK_COMPLETE) {
						throw std::runtime_error("");
					}
					this->~TypeObject();
					new(this) TypeObject(rhs);
				}
				void d(Octet discriminator, const MinimalTypeObject& rhs) {
					if (_d == discriminator) {
						return;
					}
					if (discriminator != EK_MINIMAL) {
						throw std::runtime_error("");
					}
					this->~TypeObject();
					new(this) TypeObject(rhs);
				}
				Octet d() const {
					return _d;
				}
				CompleteTypeObject& complete_type() {
					if (_d != EK_COMPLETE) {
						throw std::runtime_error("");
					}
					return value.complete_type;
				}
				const CompleteTypeObject& complete_type() const {
					if (_d != EK_COMPLETE) {
						throw std::runtime_error("");
					}
					return value.complete_type;
				}
				MinimalTypeObject& minimal_type() {
					if (_d != EK_MINIMAL) {
						throw std::runtime_error("");
					}
					return value.minimal_type;
				}
				const MinimalTypeObject& minimal_type() const {
					if (_d != EK_MINIMAL) {
						throw std::runtime_error("");
					}
					return value.minimal_type;
				}
				~TypeObject() {
					switch (_d) {
					case EK_COMPLETE:
						value.complete_type.~CompleteTypeObject();
						break;
					case EK_MINIMAL:
						value.minimal_type.~MinimalTypeObject();
						break;
					}
				}

			private:
				uint8_t _d;
				Union value;
				// EK_COMPLETE
				// CompleteTypeObject complete_type;
				// EK_MINIMAL
				// MinimalTypeObject minimal_type;
			};

			template<>
			struct XTypesTrait<TypeObject> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			using TypeObjectSeq = BoundedSequence<TypeObject>;

			// Set of TypeObjects representing a strongly connected component
			// Equivalence class
			// for the Strong Connectivity relationship
			// (mutal reachability betwen types).
			// Ordered by fully quialified typename lexicographic order
			using StronglyConnectedComponent = TypeObjectSeq;

			struct TypeIdentifierTypeObjectPair
			{
				TypeIdentifier type_identifier;
				TypeObject type_object;
			};

			using TypeIdentifierTypeObjectPairSeq = BoundedSequence<TypeIdentifierTypeObjectPair>;

			struct TypeIdentifierPair
			{
				TypeIdentifier type_identifier1;
				TypeIdentifier type_identifier2;
			};

			using TypeIdentifierPairSeq = BoundedSequence<TypeIdentifierPair>;

			struct TypeIdentifierWithSize
			{
				TypeIdentifier type_identifier;
				uint32_t typeobject_serialized_size;
			};

			template<>
			struct XTypesTrait<TypeIdentifierWithSize> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			struct TypeIdentifierWithDependencies
			{
				TypeIdentifierWithSize typeid_with_size;
				// The total additional types related to minimal_type
				int32_t dependent_typeid_count;
				BoundedSequence<TypeIdentifierWithSize> dependent_typeids;
			};

			template<>
			struct XTypesTrait<TypeIdentifierWithDependencies> {
				static constexpr Extensibility extensibility = Extensibility::Appendable;
			};

			using TypeIdentifierWithDependenciesSeq = BoundedSequence<TypeIdentifierWithDependencies>;

			// This appears in the builtin DDS topics PublicationBuiltinTopicData and
			// SubscriptionBuiltinTopicData
			struct TypeInformation
			{
				TypeIdentifierWithDependencies minimal;
				TypeIdentifierWithDependencies complete;
			};

			template<>
			struct XTypesTrait<TypeInformation> {
				static constexpr Extensibility extensibility = Extensibility::Mutable;
			};

			using TypeInformationSeq = BoundedSequence<TypeInformation>;
		}
	}
}
