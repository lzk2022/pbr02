
### explicit 
��ֹ��ʽת����ʹ�� explicit �ؼ������εĹ��캯����ֻ������ȷ����ʱ�Żᱻʹ�ã��������ڲ�����������ʽת����

û�� explicit �Ĺ��캯����
```cpp
class Scene {
public:
    Scene(const std::string& title) {
        // ���캯��ʵ��
    }
};

void createScene(Scene scene) {
    // do something with scene
}

int main() {
    Scene scene1 = "My Scene"; // ��ʽת�����Ϸ�
    createScene("Another Scene"); // ��ʽת�����Ϸ�
    return 0;
}
```

ʹ�� explicit �Ĺ��캯����
```cpp
class Scene {
public:
    explicit Scene(const std::string& title) {
        // ���캯��ʵ��
    }
};

void createScene(Scene scene) {
    // do something with scene
}

int main() {
    Scene scene1("My Scene"); // ������ʽ���ù��캯��
    // createScene("Another Scene"); // ���������ʽת������ֹ
    createScene(Scene("Another Scene")); // �Ϸ�����ʽ���ù��캯��
    return 0;
}
```

### ���ÿ�������
ΪʲôҪ���ÿ�������
1. ��ֹ��Դ���ƣ�����������Դ�����ļ�������������ӡ��ڴ�ָ��ȣ���������Щ��Դ���ܵ�����Դ��ͻ��й©�����ÿ����������Ա����������⡣
2. ȷ��Ψһ�ԣ���Щ�����Ϊ����Ψһ�ԣ���һ����ʵ���ڳ�����Ӧ����Ψһ�ģ��޷������ơ�
3. ��ߴ��밲ȫ�ԣ���ʽ���ÿ����������Է�ֹ����Ķ����ƣ�����Ǳ�ڵĴ���
```cpp
class Asset {
public:
    Asset() {
        // ���캯��
    }

    // ���ÿ������캯��
    Asset(const Asset&) = delete;

    // ���ÿ�����ֵ�����
    Asset& operator=(const Asset&) = delete;

    // ������Ա�����ͱ���
};

int main() {
    Asset a;
    // Asset b(a);  // ���󣺿������캯��������
    // Asset c;
    // c = a;       // ���󣺿�����ֵ�����������
    return 0;
}

```

### �ƶ����캯��
noexcept������ؼ��ֱ�ʾ�ƶ����캯�������׳��쳣��
```cpp
class Asset {
public:
    // �ƶ����캯��
    Asset(Asset&& other) noexcept {
        // �� other ����Դת�Ƶ���ǰ����
        this->resource = other.resource;
        // �� other ����Դָ����Ϊ nullptr���Է�ֹ������ʱ�ͷ���Դ
        other.resource = nullptr;
    }

    // �ƶ���ֵ�����
    Asset& operator=(Asset&& other) noexcept {
        if (this != &other) {
            // �ͷŵ�ǰ�������Դ
            delete this->resource;
            // ת����Դ
            this->resource = other.resource;
            // �� other ����Դָ����Ϊ nullptr���Է�ֹ������ʱ�ͷ���Դ
            other.resource = nullptr;
        }
        return *this;
    }

    // ��������
    ~Asset() {
        delete resource;
    }

private:
    int* resource; // ������Դ��һ����̬���������
};

```

### static inline �� inline static  ������
`static inline`��ͨ�����ں�����static ���ƺ����������Ե���ǰ���뵥Ԫ���� inline ��ʾ�����������ú����Ĵ��롣

`inline static`��ͨ�����ھ�̬���ݳ�Ա��inline ʹ�þ�̬��Ա�����ڶ�����뵥Ԫ�ж��������������Ӵ��󣬶� static ��ʾ�����Ա�����࣬���������ʵ����