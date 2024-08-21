
### explicit 
防止隐式转换：使用 explicit 关键字修饰的构造函数，只有在明确调用时才会被使用，而不会在不经意间进行隐式转换。

没有 explicit 的构造函数：
```cpp
class Scene {
public:
    Scene(const std::string& title) {
        // 构造函数实现
    }
};

void createScene(Scene scene) {
    // do something with scene
}

int main() {
    Scene scene1 = "My Scene"; // 隐式转换，合法
    createScene("Another Scene"); // 隐式转换，合法
    return 0;
}
```

使用 explicit 的构造函数：
```cpp
class Scene {
public:
    explicit Scene(const std::string& title) {
        // 构造函数实现
    }
};

void createScene(Scene scene) {
    // do something with scene
}

int main() {
    Scene scene1("My Scene"); // 必须显式调用构造函数
    // createScene("Another Scene"); // 编译错误，隐式转换被禁止
    createScene(Scene("Another Scene")); // 合法，显式调用构造函数
    return 0;
}
```

### 禁用拷贝操作
为什么要禁用拷贝操作
1. 防止资源复制：如果类管理资源（如文件句柄、网络连接、内存指针等），复制这些资源可能导致资源冲突或泄漏。禁用拷贝操作可以避免这种问题。
2. 确保唯一性：有些类设计为具有唯一性，即一个类实例在程序中应该是唯一的，无法被复制。
3. 提高代码安全性：显式禁用拷贝操作可以防止意外的对象复制，减少潜在的错误。
```cpp
class Asset {
public:
    Asset() {
        // 构造函数
    }

    // 禁用拷贝构造函数
    Asset(const Asset&) = delete;

    // 禁用拷贝赋值运算符
    Asset& operator=(const Asset&) = delete;

    // 其他成员函数和变量
};

int main() {
    Asset a;
    // Asset b(a);  // 错误：拷贝构造函数被禁用
    // Asset c;
    // c = a;       // 错误：拷贝赋值运算符被禁用
    return 0;
}

```

### 移动构造函数
noexcept：这个关键字表示移动构造函数不会抛出异常。
```cpp
class Asset {
public:
    // 移动构造函数
    Asset(Asset&& other) noexcept {
        // 将 other 的资源转移到当前对象
        this->resource = other.resource;
        // 将 other 的资源指针设为 nullptr，以防止其析构时释放资源
        other.resource = nullptr;
    }

    // 移动赋值运算符
    Asset& operator=(Asset&& other) noexcept {
        if (this != &other) {
            // 释放当前对象的资源
            delete this->resource;
            // 转移资源
            this->resource = other.resource;
            // 将 other 的资源指针设为 nullptr，以防止其析构时释放资源
            other.resource = nullptr;
        }
        return *this;
    }

    // 析构函数
    ~Asset() {
        delete resource;
    }

private:
    int* resource; // 假设资源是一个动态分配的整数
};

```

### static inline 和 inline static  的区别
`static inline`：通常用于函数。static 限制函数的链接性到当前编译单元，而 inline 提示编译器内联该函数的代码。

`inline static`：通常用于静态数据成员。inline 使得静态成员可以在多个翻译单元中定义而不会产生链接错误，而 static 表示这个成员属于类，而不是类的实例。