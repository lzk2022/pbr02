#include "Ext.h"
#include <locale>
#include <codecvt>

namespace utils
{
	std::string ToU8(const std::string str)
	{
        //GBK��windows�µ�locale name(.936, CHS ), linux�µ�locale��������"zh_CN.GBK",��ƽ̨��Ҫ����ͬ����ϵͳ������
        const char* GBK_LOCALE_NAME = "CHS";

        std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>>
            conv(new std::codecvt<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
        std::wstring wString = conv.from_bytes(str);            // string => wstring

        std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
        std::string utf8str = convert.to_bytes(wString);        // wstring => utf-8
        return utf8str;
	}
}
