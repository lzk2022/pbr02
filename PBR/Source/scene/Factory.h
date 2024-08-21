#pragma once
#include <string>
#include <vector>
#include "Scene.h"
#include "../example/Scene01.h"

namespace scene::factory {
    inline const std::vector<std::string> gkTitles{
        "��ӭ����",
        "ǰ����Ⱦ����",
    };
    inline Scene* LoadScene(const std::string& title) {
        if (title == "��ӭ����") return new Scene(title);
        if (title == "ǰ����Ⱦ����") return new Scene01(title);
        // ���δ�ҵ���Ӧ����ĳ�������������˳�����
        LOG_ASSERT_TRUE(true, "���� {0} δ�ڹ�����ע��", title);
    }
}
