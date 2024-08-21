#include "FBO.h"
#include <memory>
#include "VAO.h"
#include "Shader.h"
#include "../utils/Global.h"
#include "../utils/Log.h"
namespace asset {
    // 通过避免不必要的绑定和解绑来优化上下文切换
    static GLuint gCurrBoundRenderbuffer = 0;  // 当前绑定的渲染缓冲区
    static GLuint gCurrBoundFramebuffer = 0;   // 当前绑定的帧缓冲区

    static std::unique_ptr<VAO> gInternalVAO = nullptr;         // 内部使用的 VAO 资源
    static std::unique_ptr<Shader> gInternalShader = nullptr;   // 内部使用的着色器资源
    RBO::RBO(GLuint width, GLuint height, bool isMultisample) : Asset(){
        LOG_TRACK;
        glCreateRenderbuffers(1, &mId);  // 创建一个渲染缓冲对象
        if (isMultisample) {
            glNamedRenderbufferStorageMultisample(mId, 4, GL_DEPTH24_STENCIL8, width, height);  // 设置多重采样渲染缓冲存储
        }
        else {
            glNamedRenderbufferStorage(mId, GL_DEPTH24_STENCIL8, width, height);  // 设置常规渲染缓冲存储
        }
    }
    RBO::~RBO(){
        LOG_TRACK;
        glDeleteRenderbuffers(1, &mId);  // 析构函数，删除渲染缓冲对象
    }

    void RBO::Bind() const{
        LOG_TRACK;
        if (mId != gCurrBoundRenderbuffer) {  // 检查当前是否已绑定该渲染缓冲对象
            glBindRenderbuffer(GL_RENDERBUFFER, mId);  // 绑定渲染缓冲对象
            gCurrBoundRenderbuffer = mId;  // 更新当前绑定的渲染缓冲对象 ID
        }
    }

    void RBO::UnBind() const{
        LOG_TRACK;
        if (gCurrBoundRenderbuffer == mId) {  // 检查当前是否已绑定该渲染缓冲对象
            gCurrBoundRenderbuffer = 0;  // 重置当前绑定的渲染缓冲对象 ID
            glBindRenderbuffer(GL_RENDERBUFFER, 0);  // 解绑渲染缓冲对象
        }
    }

    /*****************************************************FBO**************************************************************/
	FBO::FBO(GLuint width, GLuint height) : Asset(), mWidth(width),mHeight(height),mStatus(0){
        LOG_TRACK;
        glDisable(GL_FRAMEBUFFER_SRGB);     // 全局关闭颜色空间校正，确保帧缓冲对象中的颜色值不进行颜色空间转换
        glCreateFramebuffers(1, &mId);      // 创建一个帧缓冲对象，并将其ID存储在成员变量id中

        // 如果全局VAO对象为空，创建并初始化一个
        if (!gInternalVAO) {
            gInternalVAO = std::make_unique<VAO>();
        }

        // 如果全局着色器对象为空，加载指定路径下的着色器程序
        if (!gInternalShader) {
            gInternalShader = std::make_unique<Shader>("core\\framebuffer.glsl");
        }
	}
    FBO::~FBO(){
        LOG_TRACK;
        UnBind();                      
        glDeleteFramebuffers(1, &mId);
    }

    void FBO::AddColorTexture(GLuint count, bool multisample){
        LOG_TRACK;
        static size_t maxColorBuffs = gHardware.glMaxColorBuffs;    // 获取支持的最大颜色附件数量
        size_t colorBuffsNum = mColorAttachments.size();            // 当前已添加的颜色附件数量

        // 检查是否可以添加所需数量的颜色附件
        if (colorBuffsNum + count > maxColorBuffs) {
            LOG_ERROR("无法将 {0} 个颜色附件添加到帧缓冲区", count);
            LOG_ERROR("一个帧缓冲区最多可以有 {0} 个颜色附件", maxColorBuffs);
            return;
        }

        mColorAttachments.reserve(colorBuffsNum + count);  // 预先分配存储空间以容纳新的颜色附件

        // 为每个颜色附件创建并配置纹理对象
        for (GLuint i = 0; i < count; i++) {
            GLenum target = multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;  // 确定纹理目标类型
            auto& texture = mColorAttachments.emplace_back(target, mWidth, mHeight, 1, GL_RGBA16F, 1);  // 在color_attachments中添加新的纹理对象
            GLuint tid = texture.getId();  // 获取新添加纹理的ID

            static const float border[] = { 0.0f, 0.0f, 0.0f, 1.0f };  // 边界颜色设置

            // 针对非多重采样纹理设置采样器状态
            if (!multisample) {
                glTextureParameteri(tid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(tid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(tid, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTextureParameteri(tid, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                glTextureParameterfv(tid, GL_TEXTURE_BORDER_COLOR, border);
            }

            // 将纹理附件绑定到帧缓冲对象的颜色附件位置
            glNamedFramebufferTexture(mId, GL_COLOR_ATTACHMENT0 + colorBuffsNum + i, tid, 0);
        }

        SetDrawBuffers();  // 默认情况下启用所有渲染目标以进行写操作
        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // 检查帧缓冲对象的完整性状态
    }

    void FBO::SetColorTexture(GLenum index, GLuint texture_2d){
        LOG_TRACK;
        static size_t maxColorBuffs = gHardware.glMaxColorBuffs;;   // 获取支持的最大颜色附件数量
        size_t n_color_buffs = mColorAttachments.size();            // 当前已添加的颜色附件数量

        LOG_ASSERT_TRUE(index < maxColorBuffs, "颜色附件索引 {0} 超出范围！", index);
        LOG_ASSERT_TRUE(index >= n_color_buffs, "颜色附件 {0} 已被占用！", index);

        // 将2D纹理附加到帧缓冲对象的指定颜色附件位置
        glNamedFramebufferTexture(mId, GL_COLOR_ATTACHMENT0 + index, texture_2d, 0);

        SetDrawBuffers();  // 更新渲染目标状态，确保所有渲染目标都启用写操作
        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // 检查帧缓冲对象的完整性状态
    }

    void FBO::SetColorTexture(GLenum index, GLuint textureCubemap, GLuint face){
        LOG_TRACK;
        static size_t maxColorBuffs = gHardware.glMaxColorBuffs;  // 获取支持的最大颜色附件数量
        size_t n_color_buffs = mColorAttachments.size();  // 当前已添加的颜色附件数量

        LOG_ASSERT(index < maxColorBuffs, "颜色附件索引 {0} 超出了范围！", index);
        LOG_ASSERT(index >= n_color_buffs, "颜色附件 {0} 已经被占用！", index);
        LOG_ASSERT(face < 6, "无效的立方体贴图面 ID，必须是 0 到 5 之间的数字！");

        // 使用DSA方式将立方体贴图的指定面附加到帧缓冲对象的指定颜色附件位置
        if constexpr (true) {
            // 某些Intel驱动程序不支持这个DSA函数
            glNamedFramebufferTextureLayer(mId, GL_COLOR_ATTACHMENT0 + index, textureCubemap, 0, face);
        }
        else {
            // 否则，使用传统的方式将2D纹理附加到帧缓冲对象的指定颜色附件位置
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, textureCubemap, 0);
        }

        SetDrawBuffers();  // 更新渲染目标状态，确保所有渲染目标都启用写操作
        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // 检查帧缓冲对象的完整性状态
    }
    void FBO::AddDepStTexture(bool multisample){
        LOG_TRACK;
        // 帧缓冲对象只能有一个深度模板缓冲区，可以是纹理也可以是渲染缓冲区
        LOG_ASSERT(!mDepstRenderbuffer, "帧缓冲区已经有一个深度模板渲染缓冲区了");
        LOG_ASSERT(!mDepstTexture, "帧缓冲区只能附加一个深度模板纹理");

        // 深度模板纹理无法进行过滤，因此不支持多重采样
        if (multisample) {
            LOG_ERROR("多重采样的深度模板纹理不受支持，这是对内存的浪费！");
            LOG_ERROR("如果需要多重采样抗锯齿 (MSAA)，请改为添加一个多重采样的渲染缓冲区 (RBO)");
            return;
        }

        // 创建包含深度值和模板值的不可变格式纹理
        mDepstTexture = std::make_unique<Texture>(GL_TEXTURE_2D, mWidth, mHeight, 1, GL_DEPTH24_STENCIL8, 1);
        glTextureParameteri(mDepstTexture->getId(), GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

        GLint immutableFormat;
        glGetTextureParameteriv(mDepstTexture->getId(), GL_TEXTURE_IMMUTABLE_FORMAT, &immutableFormat);
        LOG_ASSERT(immutableFormat == GL_TRUE, "无法附加不可变的深度模板纹理");

        // 若要在GLSL中访问模板值，需要创建一个单独的纹理视图
        mStencilView = std::make_unique<TexView>(*mDepstTexture);
        mStencilView->SetView(GL_TEXTURE_2D, 0, 1, 0, 1);
        glTextureParameteri(mStencilView->getId(), GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);

        glNamedFramebufferTexture(mId, GL_DEPTH_STENCIL_ATTACHMENT, mDepstTexture->getId(), 0);
        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // 检查帧缓冲对象的完整性状态

    }

    void FBO::AddDepStRenderBuffer(bool multisample){
        LOG_TRACK;
        // 帧缓冲对象只能有一个深度模板缓冲区，可以是纹理也可以是渲染缓冲区
        LOG_ASSERT(!mDepstTexture, "帧缓冲区已经有一个深度模板纹理了");
        LOG_ASSERT(!mDepstRenderbuffer, "帧缓冲区只能附加一个深度模板渲染缓冲区");

        // 深度和模板值被合并在单个渲染缓冲区（RBO）中
        // 每个32位像素包含24位深度值和8位模板值

        mDepstRenderbuffer = std::make_unique<RBO>(mWidth, mHeight, multisample);  // 创建深度模板渲染缓冲区对象
        mDepstRenderbuffer->Bind();  // 绑定深度模板渲染缓冲区
        // 将渲染缓冲区附加到帧缓冲对象的深度模板附件
        glNamedFramebufferRenderbuffer(mId, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepstRenderbuffer->getId());  

        // 渲染缓冲区形式的深度和模板缓冲区是只写的
        // 之后不能读取，因此不需要创建模板纹理视图

        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // 检查帧缓冲对象的完整性状态
    }

    void FBO::AddDepthCubemap(){
        LOG_TRACK;
        // 帧缓冲对象只能有一个深度模板缓冲区，可以是纹理也可以是渲染缓冲区
        LOG_ASSERT(!mDepstRenderbuffer, "帧缓冲区已经有一个深度模板渲染缓冲区了");
        LOG_ASSERT(!mDepstTexture, "帧缓冲区只能附加一个深度模板纹理");

        // 对于全向阴影映射，我们只需要一个高精度的立方体深度纹理
        // 我们可以使用`GL_DEPTH_COMPONENT32F`来获得最佳精度，但这通常过于冗余
        // 在实践中，人们通常使用`GL_DEPTH_COMPONENT24/16`

        mDepstTexture = std::make_unique<Texture>(GL_TEXTURE_CUBE_MAP, mWidth, mHeight, 6, GL_DEPTH_COMPONENT24, 1);  // 创建立方体深度纹理对象
        GLuint tid = mDepstTexture->getId();

        glTextureParameteri(tid, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
        glTextureParameteri(tid, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(tid, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(tid, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(tid, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(tid, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(mId, GL_DEPTH_ATTACHMENT, tid, 0);  // 将立方体深度纹理附加到帧缓冲对象的深度附件
        const GLenum null[] = { GL_NONE };
        glNamedFramebufferReadBuffer(mId, GL_NONE);  // 禁用读取缓冲区
        glNamedFramebufferDrawBuffers(mId, 1, null);  // 禁用绘制缓冲区

        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // 检查帧缓冲对象的完整性状态
    }

    const Texture& FBO::GetColorTexture(GLenum index) const{
        LOG_TRACK;
        LOG_ASSERT(index < mColorAttachments.size(), "无效的颜色附件索引：{0}", index);
        return mColorAttachments[index];
    }

    const Texture& FBO::GetDepthTexture() const{
        LOG_ASSERT(mDepstTexture, "帧缓冲区没有深度纹理");
        return *mDepstTexture;
    }

    const TexView& FBO::GetStencilTexView() const{
        LOG_ASSERT(mStencilView, "帧缓冲区没有模板纹理视图");
        return *mStencilView;
    }

    void FBO::Bind() const{
        LOG_TRACK;
        if (mId != gCurrBoundFramebuffer) {
            LOG_ASSERT(mStatus == GL_FRAMEBUFFER_COMPLETE, "帧缓冲区状态不完整：{0}", mStatus);
            if (mDepstRenderbuffer) {
                mDepstRenderbuffer->Bind();  // 绑定深度模板渲染缓冲区
            }
            glBindFramebuffer(GL_FRAMEBUFFER, mId);  // 绑定帧缓冲对象
            gCurrBoundFramebuffer = mId;
        }
    }

    void FBO::UnBind() const{
        LOG_TRACK;
        if (gCurrBoundFramebuffer == mId) {
            gCurrBoundFramebuffer = 0;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);  // 解绑帧缓冲对象
        }
    }

    void FBO::SetDrawBuffer(GLuint index) const{
        LOG_TRACK;
        // 该函数仅允许写入单个颜色附件
        LOG_ASSERT(index < mColorAttachments.size(), "颜色缓冲区索引超出范围！");
        const GLenum buffers[] = { GL_COLOR_ATTACHMENT0 + index };
        glNamedFramebufferDrawBuffers(mId, 1, buffers);
    }

    void FBO::SetDrawBuffers(std::vector<GLuint> indices) const{
        LOG_TRACK;
        // 该函数允许设置输入列表中的多个颜色附件用于绘制
        size_t buffsNum = mColorAttachments.size();
        size_t indexNum = indices.size();
        GLenum* buffers = new GLenum[indexNum];

        for (size_t i = 0; i < indexNum; i++) {
            // `layout(location = i) out` 变量将写入此附件
            GLuint index = indices[i];
            LOG_ASSERT(index < buffsNum, "颜色缓冲区索引 {0} 超出范围！", index);
            buffers[i] = GL_COLOR_ATTACHMENT0 + index;
        }

        glNamedFramebufferDrawBuffers(mId, indexNum, buffers);
        delete[] buffers;
    }

    void FBO::SetDrawBuffers() const{
        LOG_TRACK;
        // 如果存在颜色附件，则启用所有颜色附件进行写入
        if (size_t n = mColorAttachments.size(); n > 0) {
            GLenum* attachments = new GLenum[n];

            for (GLenum i = 0; i < n; i++) {
                attachments[i] = GL_COLOR_ATTACHMENT0 + i;
            }

            glNamedFramebufferDrawBuffers(mId, n, attachments);
            delete[] attachments;
        }
    }

    void FBO::Draw(GLint index) const{
        LOG_TRACK;
        gInternalVAO->Bind();  // 绑定内部VAO
        gInternalShader->Bind();  // 绑定内部着色器程序

        GLuint subroutineIndex = 0;  // 子例程索引，默认为0

        // 可视化其中一个颜色附件
        if (index >= 0 && index < static_cast<GLint>(mColorAttachments.size())) {
            subroutineIndex = 0;  // 使用子例程索引0
            mColorAttachments[index].Bind(0);  // 绑定指定索引的颜色附件纹理
        }
        // 可视化线性深度缓冲区
        else if (index == -1) {
            subroutineIndex = 1;  // 使用子例程索引1
            if (mDepstTexture != nullptr) {
                mDepstTexture->Bind(0);  // 绑定深度纹理
            }
            else {
                LOG_ERROR("无法可视化深度缓冲区，深度纹理不可用！");
            }
        }
        // 可视化模板缓冲区
        else if (index == -2) {
            subroutineIndex = 2;  // 使用子例程索引2
            if (mStencilView != nullptr) {
                mStencilView->Bind(1);  // 绑定模板纹理视图（使用纹理单元1）
            }
            else {
                LOG_ERROR("无法可视化模板缓冲区，模板视图不可用！");
            }
        }
        else {
            LOG_ERROR("缓冲区索引 {0} 在帧缓冲区中无效！", index);
            LOG_ERROR("有效的索引：0-{0}（颜色），-1（深度），-2（模板）", mColorAttachments.size() - 1);
        }

        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);  // 设置片段着色器子例程索引

        glDrawArrays(GL_TRIANGLES, 0, 3);  // 无缓冲区绘制四边形

    }

    void FBO::Clear(GLint index) const{
        LOG_TRACK;
        const GLfloat clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  // 清除颜色
        const GLfloat clearDepth = 1.0f;  // 清除深度
        const GLint clearStencil = 0;  // 清除模板

        const size_t maxColorBuffs = gHardware.glMaxColorBuffs;  // 最大颜色附件数

        // 清除一个颜色附件
        if (index >= 0 && index < static_cast<GLint>(maxColorBuffs)) {
            glClearNamedFramebufferfv(mId, GL_COLOR, index, clearColor);
        }
        // 清除深度缓冲区
        else if (index == -1) {
            glClearNamedFramebufferfv(mId, GL_DEPTH, 0, &clearDepth);
        }
        // 清除模板缓冲区
        else if (index == -2) {
            glClearNamedFramebufferiv(mId, GL_STENCIL, 0, &clearStencil);
        }
        else {
            LOG_ERROR("缓冲区索引 {0} 在帧缓冲区中无效！", index);
            LOG_ERROR("有效的索引：0-{0}（颜色），-1（深度），-2（模板）", maxColorBuffs - 1);
        }
    }

    void FBO::Clear() const{
        LOG_TRACK;
        for (int i = 0; i < static_cast<int>(mColorAttachments.size()); i++) {
            Clear(i);  // 清除所有颜色附件
        }

        Clear(-1);  // 清除深度缓冲区
        Clear(-2);  // 清除模板缓冲区
    }

    void FBO::CopyColor(const FBO& src, GLuint srcIndex, const FBO& dst, GLuint dstIndex){
        LOG_TRACK;
        LOG_ASSERT(srcIndex < src.mColorAttachments.size(), "颜色缓冲区索引 {0} 超出范围", srcIndex);
        LOG_ASSERT(dstIndex < dst.mColorAttachments.size(), "颜色缓冲区索引 {0} 超出范围", dstIndex);

        // 如果源和目标矩形区域的大小不同，将应用插值
        GLuint fw = src.mWidth, fh = src.mHeight;
        GLuint tw = dst.mWidth, th = dst.mHeight;

        glNamedFramebufferReadBuffer(src.mId, GL_COLOR_ATTACHMENT0 + srcIndex);
        glNamedFramebufferDrawBuffer(dst.mId, GL_COLOR_ATTACHMENT0 + dstIndex);
        glBlitNamedFramebuffer(src.mId, dst.mId, 0, 0, fw, fh, 0, 0, tw, th, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    }

    void FBO::CopyDepth(const FBO& src, const FBO& dst){
        LOG_TRACK;
        // 确保在调用此函数时全局禁用了GL_FRAMEBUFFER_SRGB！
        // 如果启用了颜色空间校正，则在复制过程中深度值将进行Gamma编码...
        GLuint fw = src.mWidth, fh = src.mHeight;
        GLuint tw = dst.mWidth, th = dst.mHeight;
        glBlitNamedFramebuffer(src.mId, dst.mId, 0, 0, fw, fh, 0, 0, tw, th, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    }

    void FBO::CopyStencil(const FBO& src, const FBO& dst){
        LOG_TRACK;
        // 确保在调用此函数时全局禁用了GL_FRAMEBUFFER_SRGB！
        // 如果启用了颜色空间校正，则在复制过程中模板值将进行Gamma编码...
        GLuint fw = src.mWidth, fh = src.mHeight;
        GLuint tw = dst.mWidth, th = dst.mHeight;
        glBlitNamedFramebuffer(src.mId, dst.mId, 0, 0, fw, fh, 0, 0, tw, th, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    }
    
}