#pragma once
#include <string>
#include <vector>
#include "Scene.h"
#include "../example/Scene01.h"
#include "../example/Scene02.h"

namespace scene::factory {
    inline const std::vector<std::string> gkTitles{
        "��ӭ����",
        "ǰ����Ⱦ����",
        "����ģ����Ⱦ����",
    };
    inline Scene* LoadScene(const std::string& title) {
        if (title == "��ӭ����") return new Scene(title);
        if (title == "ǰ����Ⱦ����") return new Scene01(title);
        if (title == "����ģ����Ⱦ����") return new Scene02(title);
        // ���δ�ҵ���Ӧ����ĳ�������������˳�����
        LOG_ASSERT_TRUE(true, "���� {0} δ�ڹ�����ע��", title);
    }
}
