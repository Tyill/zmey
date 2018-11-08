#pragma once

#include <cstdint>
#include <string>

namespace rapidjson
{
	namespace helper_extraction
	{
		///	\brief ��������� ��������
		///	\param [in] tValue: ��������.
		///	\param [in] pcName: ��� ������������ ��������.
		///	\return: ����������� ��������.
		template<typename TValue, typename TSymbol>
		const TValue* ExtractMemberValue(const TValue &tValue, const TSymbol *pcName)
		{
			const auto itMember = tValue.FindMember(pcName);
			if (itMember != tValue.MemberEnd())
				return &itMember->value;
			return nullptr;
		}

		///	\brief ��������� �������� ���� bool
		///	\param [in] tValue: ��������.
		///	\param [in] pcName: ��� ������������ ��������.
		///	\return: ����������� ��������.
		template<typename TValue, typename TSymbol>
		bool ExtractMemberValueOrDefaultBool(const TValue &tValue, const TSymbol *pcName)
		{
			const auto* pValue = ExtractMemberValue(tValue, pcName);
			return pValue != nullptr && pValue->IsBool() ? pValue->GetBool() : false;
		}

		///	\brief ��������� �������� ���� Int32
		///	\param [in] tValue: ��������.
		///	\param [in] pcName: ��� ������������ ��������.
		///	\return: ����������� ��������.
		template<typename TValue, typename TSymbol>
		int32_t ExtractMemberValueOrDefaultInt32(const TValue &tValue, const TSymbol *pcName)
		{
			const auto* pValue = ExtractMemberValue(tValue, pcName);
			return pValue != nullptr && pValue->IsInt() ? pValue->GetInt() : 0;
		}

		///	\brief ��������� �������� ���� UInt32
		///	\param [in] tValue: ��������.
		///	\param [in] pcName: ��� ������������ ��������.
		///	\return: ����������� ��������.
		template<typename TValue, typename TSymbol>
		uint32_t ExtractMemberValueOrDefaultUInt32(const TValue &tValue, const TSymbol *pcName)
		{
			const auto* pValue = ExtractMemberValue(tValue, pcName);
			return pValue != nullptr && pValue->IsUint() ? pValue->GetUint() : 0;
		}

		///	\brief ��������� �������� ���� float
		///	\param [in] tValue: ��������.
		///	\param [in] pcName: ��� ������������ ��������.
		///	\return: ����������� ��������.
		template<typename TValue, typename TSymbol>
		float ExtractMemberValueOrDefaultFloat(const TValue &tValue, const TSymbol *pcName)
		{
			const auto* pValue = ExtractMemberValue(tValue, pcName);
			return pValue != nullptr && pValue->IsFloat() ? pValue->GetFloat() : 0;
		}

		///	\brief ��������� �������� ���� double
		///	\param [in] tValue: ��������.
		///	\param [in] pcName: ��� ������������ ��������.
		///	\return: ����������� ��������.
		template<typename TValue, typename TSymbol>
		double ExtractMemberValueOrDefaultDouble(const TValue &tValue, const TSymbol *pcName)
		{
			const auto* pValue = ExtractMemberValue(tValue, pcName);
			return pValue != nullptr && pValue->IsDouble() ? pValue->GetDouble() : 0;
		}

		///	\brief ��������� �������� ���� wstring
		///	\param [in] tValue: ��������.
		///	\param [in] pcName: ��� ������������ ��������.
		///	\return: ����������� ��������.
		template<typename TValue, typename TSymbol>
		std::wstring ExtractMemberValueOrDefaultString(const TValue &tValue, const TSymbol *pcName)
		{
			const auto* pValue = ExtractMemberValue(tValue, pcName);
			return pValue != nullptr && pValue->IsString() ? pValue->GetString() : std::wstring();
		}

		///	\brief ��������� �������� ���� enum
		///	\param [in] tValue: ��������.
		///	\param [in] pcName: ��� ������������ ��������.
		///	\param [in] tMap: ����� ������ ��������.
		///	\return: ����������� ��������.
		template<typename TEnum, typename TValue, typename TSymbol, typename TMap>
		TEnum ExtractMemberValueOrDefaultEnum(const TValue &tValue, const TSymbol *pcName, const TMap &tMap)
		{
			const auto strValue(ExtractMemberValueOrDefaultString(tValue, pcName));
			for (auto it = tMap.cbegin(); it != tMap.cend(); ++it)
			{
				if (it->second == strValue)
					return it->first;
			}
			TEnum tEnum;
			memset(&tEnum, 0, sizeof(tEnum));
			return tEnum;
		}
	}
}