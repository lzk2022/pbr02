#pragma once
#include <string>
#include <vector>
#include "Scene.h"
#include "../example/Scene01.h"
#include "../example/Scene02.h"

namespace scene::factory {
    inline const std::vector<std::string> gkTitles{
        "欢迎界面",
        "前向渲染界面",
        "布料模拟渲染界面",
    };
    inline Scene* LoadScene(const std::string& title) {
        if (title == "欢迎界面") return new Scene(title);
        if (title == "前向渲染界面") return new Scene01(title);
        if (title == "布料模拟渲染界面") return new Scene02(title);
        // 如果未找到对应标题的场景，输出错误并退出程序
        LOG_ASSERT_TRUE(true, "场景 {0} 未在工厂中注册", title);
    }
}
