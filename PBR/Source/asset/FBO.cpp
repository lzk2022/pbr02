#include "FBO.h"
#include <memory>
#include "VAO.h"
#include "Shader.h"
#include "../utils/Global.h"
#include "../utils/Log.h"
namespace asset {
    // ͨ�����ⲻ��Ҫ�İ󶨺ͽ�����Ż��������л�
    static GLuint gCurrBoundRenderbuffer = 0;  // ��ǰ�󶨵���Ⱦ������
    static GLuint gCurrBoundFramebuffer = 0;   // ��ǰ�󶨵�֡������

    static std::unique_ptr<VAO> gInternalVAO = nullptr;         // �ڲ�ʹ�õ� VAO ��Դ
    static std::unique_ptr<Shader> gInternalShader = nullptr;   // �ڲ�ʹ�õ���ɫ����Դ
    RBO::RBO(GLuint width, GLuint height, bool isMultisample) : Asset(){
        LOG_TRACK;
        glCreateRenderbuffers(1, &mId);  // ����һ����Ⱦ�������
        if (isMultisample) {
            glNamedRenderbufferStorageMultisample(mId, 4, GL_DEPTH24_STENCIL8, width, height);  // ���ö��ز�����Ⱦ����洢
        }
        else {
            glNamedRenderbufferStorage(mId, GL_DEPTH24_STENCIL8, width, height);  // ���ó�����Ⱦ����洢
        }
    }
    RBO::~RBO(){
        LOG_TRACK;
        glDeleteRenderbuffers(1, &mId);  // ����������ɾ����Ⱦ�������
    }

    void RBO::Bind() const{
        LOG_TRACK;
        if (mId != gCurrBoundRenderbuffer) {  // ��鵱ǰ�Ƿ��Ѱ󶨸���Ⱦ�������
            glBindRenderbuffer(GL_RENDERBUFFER, mId);  // ����Ⱦ�������
            gCurrBoundRenderbuffer = mId;  // ���µ�ǰ�󶨵���Ⱦ������� ID
        }
    }

    void RBO::UnBind() const{
        LOG_TRACK;
        if (gCurrBoundRenderbuffer == mId) {  // ��鵱ǰ�Ƿ��Ѱ󶨸���Ⱦ�������
            gCurrBoundRenderbuffer = 0;  // ���õ�ǰ�󶨵���Ⱦ������� ID
            glBindRenderbuffer(GL_RENDERBUFFER, 0);  // �����Ⱦ�������
        }
    }

    /*****************************************************FBO**************************************************************/
	FBO::FBO(GLuint width, GLuint height) : Asset(), mWidth(width),mHeight(height),mStatus(0){
        LOG_TRACK;
        glDisable(GL_FRAMEBUFFER_SRGB);     // ȫ�ֹر���ɫ�ռ�У����ȷ��֡��������е���ɫֵ��������ɫ�ռ�ת��
        glCreateFramebuffers(1, &mId);      // ����һ��֡������󣬲�����ID�洢�ڳ�Ա����id��

        // ���ȫ��VAO����Ϊ�գ���������ʼ��һ��
        if (!gInternalVAO) {
            gInternalVAO = std::make_unique<VAO>();
        }

        // ���ȫ����ɫ������Ϊ�գ�����ָ��·���µ���ɫ������
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
        static size_t maxColorBuffs = gHardware.glMaxColorBuffs;    // ��ȡ֧�ֵ������ɫ��������
        size_t colorBuffsNum = mColorAttachments.size();            // ��ǰ����ӵ���ɫ��������

        // ����Ƿ�������������������ɫ����
        if (colorBuffsNum + count > maxColorBuffs) {
            LOG_ERROR("�޷��� {0} ����ɫ������ӵ�֡������", count);
            LOG_ERROR("һ��֡�������������� {0} ����ɫ����", maxColorBuffs);
            return;
        }

        mColorAttachments.reserve(colorBuffsNum + count);  // Ԥ�ȷ���洢�ռ��������µ���ɫ����

        // Ϊÿ����ɫ���������������������
        for (GLuint i = 0; i < count; i++) {
            GLenum target = multisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;  // ȷ������Ŀ������
            auto& texture = mColorAttachments.emplace_back(target, mWidth, mHeight, 1, GL_RGBA16F, 1);  // ��color_attachments������µ��������
            GLuint tid = texture.getId();  // ��ȡ����������ID

            static const float border[] = { 0.0f, 0.0f, 0.0f, 1.0f };  // �߽���ɫ����

            // ��ԷǶ��ز����������ò�����״̬
            if (!multisample) {
                glTextureParameteri(tid, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTextureParameteri(tid, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTextureParameteri(tid, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
                glTextureParameteri(tid, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
                glTextureParameterfv(tid, GL_TEXTURE_BORDER_COLOR, border);
            }

            // ���������󶨵�֡����������ɫ����λ��
            glNamedFramebufferTexture(mId, GL_COLOR_ATTACHMENT0 + colorBuffsNum + i, tid, 0);
        }

        SetDrawBuffers();  // Ĭ�����������������ȾĿ���Խ���д����
        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // ���֡��������������״̬
    }

    void FBO::SetColorTexture(GLenum index, GLuint texture_2d){
        LOG_TRACK;
        static size_t maxColorBuffs = gHardware.glMaxColorBuffs;;   // ��ȡ֧�ֵ������ɫ��������
        size_t n_color_buffs = mColorAttachments.size();            // ��ǰ����ӵ���ɫ��������

        LOG_ASSERT_TRUE(index < maxColorBuffs, "��ɫ�������� {0} ������Χ��", index);
        LOG_ASSERT_TRUE(index >= n_color_buffs, "��ɫ���� {0} �ѱ�ռ�ã�", index);

        // ��2D�����ӵ�֡��������ָ����ɫ����λ��
        glNamedFramebufferTexture(mId, GL_COLOR_ATTACHMENT0 + index, texture_2d, 0);

        SetDrawBuffers();  // ������ȾĿ��״̬��ȷ��������ȾĿ�궼����д����
        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // ���֡��������������״̬
    }

    void FBO::SetColorTexture(GLenum index, GLuint textureCubemap, GLuint face){
        LOG_TRACK;
        static size_t maxColorBuffs = gHardware.glMaxColorBuffs;  // ��ȡ֧�ֵ������ɫ��������
        size_t n_color_buffs = mColorAttachments.size();  // ��ǰ����ӵ���ɫ��������

        LOG_ASSERT(index < maxColorBuffs, "��ɫ�������� {0} �����˷�Χ��", index);
        LOG_ASSERT(index >= n_color_buffs, "��ɫ���� {0} �Ѿ���ռ�ã�", index);
        LOG_ASSERT(face < 6, "��Ч����������ͼ�� ID�������� 0 �� 5 ֮������֣�");

        // ʹ��DSA��ʽ����������ͼ��ָ���渽�ӵ�֡��������ָ����ɫ����λ��
        if constexpr (true) {
            // ĳЩIntel��������֧�����DSA����
            glNamedFramebufferTextureLayer(mId, GL_COLOR_ATTACHMENT0 + index, textureCubemap, 0, face);
        }
        else {
            // ����ʹ�ô�ͳ�ķ�ʽ��2D�����ӵ�֡��������ָ����ɫ����λ��
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, textureCubemap, 0);
        }

        SetDrawBuffers();  // ������ȾĿ��״̬��ȷ��������ȾĿ�궼����д����
        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // ���֡��������������״̬
    }
    void FBO::AddDepStTexture(bool multisample){
        LOG_TRACK;
        // ֡�������ֻ����һ�����ģ�建����������������Ҳ��������Ⱦ������
        LOG_ASSERT(!mDepstRenderbuffer, "֡�������Ѿ���һ�����ģ����Ⱦ��������");
        LOG_ASSERT(!mDepstTexture, "֡������ֻ�ܸ���һ�����ģ������");

        // ���ģ�������޷����й��ˣ���˲�֧�ֶ��ز���
        if (multisample) {
            LOG_ERROR("���ز��������ģ��������֧�֣����Ƕ��ڴ���˷ѣ�");
            LOG_ERROR("�����Ҫ���ز�������� (MSAA)�����Ϊ���һ�����ز�������Ⱦ������ (RBO)");
            return;
        }

        // �����������ֵ��ģ��ֵ�Ĳ��ɱ��ʽ����
        mDepstTexture = std::make_unique<Texture>(GL_TEXTURE_2D, mWidth, mHeight, 1, GL_DEPTH24_STENCIL8, 1);
        glTextureParameteri(mDepstTexture->getId(), GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);

        GLint immutableFormat;
        glGetTextureParameteriv(mDepstTexture->getId(), GL_TEXTURE_IMMUTABLE_FORMAT, &immutableFormat);
        LOG_ASSERT(immutableFormat == GL_TRUE, "�޷����Ӳ��ɱ�����ģ������");

        // ��Ҫ��GLSL�з���ģ��ֵ����Ҫ����һ��������������ͼ
        mStencilView = std::make_unique<TexView>(*mDepstTexture);
        mStencilView->SetView(GL_TEXTURE_2D, 0, 1, 0, 1);
        glTextureParameteri(mStencilView->getId(), GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);

        glNamedFramebufferTexture(mId, GL_DEPTH_STENCIL_ATTACHMENT, mDepstTexture->getId(), 0);
        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // ���֡��������������״̬

    }

    void FBO::AddDepStRenderBuffer(bool multisample){
        LOG_TRACK;
        // ֡�������ֻ����һ�����ģ�建����������������Ҳ��������Ⱦ������
        LOG_ASSERT(!mDepstTexture, "֡�������Ѿ���һ�����ģ��������");
        LOG_ASSERT(!mDepstRenderbuffer, "֡������ֻ�ܸ���һ�����ģ����Ⱦ������");

        // ��Ⱥ�ģ��ֵ���ϲ��ڵ�����Ⱦ��������RBO����
        // ÿ��32λ���ذ���24λ���ֵ��8λģ��ֵ

        mDepstRenderbuffer = std::make_unique<RBO>(mWidth, mHeight, multisample);  // �������ģ����Ⱦ����������
        mDepstRenderbuffer->Bind();  // �����ģ����Ⱦ������
        // ����Ⱦ���������ӵ�֡�����������ģ�帽��
        glNamedFramebufferRenderbuffer(mId, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepstRenderbuffer->getId());  

        // ��Ⱦ��������ʽ����Ⱥ�ģ�建������ֻд��
        // ֮���ܶ�ȡ����˲���Ҫ����ģ��������ͼ

        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // ���֡��������������״̬
    }

    void FBO::AddDepthCubemap(){
        LOG_TRACK;
        // ֡�������ֻ����һ�����ģ�建����������������Ҳ��������Ⱦ������
        LOG_ASSERT(!mDepstRenderbuffer, "֡�������Ѿ���һ�����ģ����Ⱦ��������");
        LOG_ASSERT(!mDepstTexture, "֡������ֻ�ܸ���һ�����ģ������");

        // ����ȫ����Ӱӳ�䣬����ֻ��Ҫһ���߾��ȵ��������������
        // ���ǿ���ʹ��`GL_DEPTH_COMPONENT32F`�������Ѿ��ȣ�����ͨ����������
        // ��ʵ���У�����ͨ��ʹ��`GL_DEPTH_COMPONENT24/16`

        mDepstTexture = std::make_unique<Texture>(GL_TEXTURE_CUBE_MAP, mWidth, mHeight, 6, GL_DEPTH_COMPONENT24, 1);  // ��������������������
        GLuint tid = mDepstTexture->getId();

        glTextureParameteri(tid, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
        glTextureParameteri(tid, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(tid, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(tid, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(tid, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(tid, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(mId, GL_DEPTH_ATTACHMENT, tid, 0);  // ����������������ӵ�֡����������ȸ���
        const GLenum null[] = { GL_NONE };
        glNamedFramebufferReadBuffer(mId, GL_NONE);  // ���ö�ȡ������
        glNamedFramebufferDrawBuffers(mId, 1, null);  // ���û��ƻ�����

        mStatus = glCheckNamedFramebufferStatus(mId, GL_FRAMEBUFFER);  // ���֡��������������״̬
    }

    const Texture& FBO::GetColorTexture(GLenum index) const{
        LOG_TRACK;
        LOG_ASSERT(index < mColorAttachments.size(), "��Ч����ɫ����������{0}", index);
        return mColorAttachments[index];
    }

    const Texture& FBO::GetDepthTexture() const{
        LOG_ASSERT(mDepstTexture, "֡������û���������");
        return *mDepstTexture;
    }

    const TexView& FBO::GetStencilTexView() const{
        LOG_ASSERT(mStencilView, "֡������û��ģ��������ͼ");
        return *mStencilView;
    }

    void FBO::Bind() const{
        LOG_TRACK;
        if (mId != gCurrBoundFramebuffer) {
            LOG_ASSERT(mStatus == GL_FRAMEBUFFER_COMPLETE, "֡������״̬��������{0}", mStatus);
            if (mDepstRenderbuffer) {
                mDepstRenderbuffer->Bind();  // �����ģ����Ⱦ������
            }
            glBindFramebuffer(GL_FRAMEBUFFER, mId);  // ��֡�������
            gCurrBoundFramebuffer = mId;
        }
    }

    void FBO::UnBind() const{
        LOG_TRACK;
        if (gCurrBoundFramebuffer == mId) {
            gCurrBoundFramebuffer = 0;
            glBindFramebuffer(GL_FRAMEBUFFER, 0);  // ���֡�������
        }
    }

    void FBO::SetDrawBuffer(GLuint index) const{
        LOG_TRACK;
        // �ú���������д�뵥����ɫ����
        LOG_ASSERT(index < mColorAttachments.size(), "��ɫ����������������Χ��");
        const GLenum buffers[] = { GL_COLOR_ATTACHMENT0 + index };
        glNamedFramebufferDrawBuffers(mId, 1, buffers);
    }

    void FBO::SetDrawBuffers(std::vector<GLuint> indices) const{
        LOG_TRACK;
        // �ú����������������б��еĶ����ɫ�������ڻ���
        size_t buffsNum = mColorAttachments.size();
        size_t indexNum = indices.size();
        GLenum* buffers = new GLenum[indexNum];

        for (size_t i = 0; i < indexNum; i++) {
            // `layout(location = i) out` ������д��˸���
            GLuint index = indices[i];
            LOG_ASSERT(index < buffsNum, "��ɫ���������� {0} ������Χ��", index);
            buffers[i] = GL_COLOR_ATTACHMENT0 + index;
        }

        glNamedFramebufferDrawBuffers(mId, indexNum, buffers);
        delete[] buffers;
    }

    void FBO::SetDrawBuffers() const{
        LOG_TRACK;
        // ���������ɫ������������������ɫ��������д��
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
        gInternalVAO->Bind();  // ���ڲ�VAO
        gInternalShader->Bind();  // ���ڲ���ɫ������

        GLuint subroutineIndex = 0;  // ������������Ĭ��Ϊ0

        // ���ӻ�����һ����ɫ����
        if (index >= 0 && index < static_cast<GLint>(mColorAttachments.size())) {
            subroutineIndex = 0;  // ʹ������������0
            mColorAttachments[index].Bind(0);  // ��ָ����������ɫ��������
        }
        // ���ӻ�������Ȼ�����
        else if (index == -1) {
            subroutineIndex = 1;  // ʹ������������1
            if (mDepstTexture != nullptr) {
                mDepstTexture->Bind(0);  // ���������
            }
            else {
                LOG_ERROR("�޷����ӻ���Ȼ�����������������ã�");
            }
        }
        // ���ӻ�ģ�建����
        else if (index == -2) {
            subroutineIndex = 2;  // ʹ������������2
            if (mStencilView != nullptr) {
                mStencilView->Bind(1);  // ��ģ��������ͼ��ʹ������Ԫ1��
            }
            else {
                LOG_ERROR("�޷����ӻ�ģ�建������ģ����ͼ�����ã�");
            }
        }
        else {
            LOG_ERROR("���������� {0} ��֡����������Ч��", index);
            LOG_ERROR("��Ч��������0-{0}����ɫ����-1����ȣ���-2��ģ�壩", mColorAttachments.size() - 1);
        }

        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutineIndex);  // ����Ƭ����ɫ������������

        glDrawArrays(GL_TRIANGLES, 0, 3);  // �޻����������ı���

    }

    void FBO::Clear(GLint index) const{
        LOG_TRACK;
        const GLfloat clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  // �����ɫ
        const GLfloat clearDepth = 1.0f;  // ������
        const GLint clearStencil = 0;  // ���ģ��

        const size_t maxColorBuffs = gHardware.glMaxColorBuffs;  // �����ɫ������

        // ���һ����ɫ����
        if (index >= 0 && index < static_cast<GLint>(maxColorBuffs)) {
            glClearNamedFramebufferfv(mId, GL_COLOR, index, clearColor);
        }
        // �����Ȼ�����
        else if (index == -1) {
            glClearNamedFramebufferfv(mId, GL_DEPTH, 0, &clearDepth);
        }
        // ���ģ�建����
        else if (index == -2) {
            glClearNamedFramebufferiv(mId, GL_STENCIL, 0, &clearStencil);
        }
        else {
            LOG_ERROR("���������� {0} ��֡����������Ч��", index);
            LOG_ERROR("��Ч��������0-{0}����ɫ����-1����ȣ���-2��ģ�壩", maxColorBuffs - 1);
        }
    }

    void FBO::Clear() const{
        LOG_TRACK;
        for (int i = 0; i < static_cast<int>(mColorAttachments.size()); i++) {
            Clear(i);  // ���������ɫ����
        }

        Clear(-1);  // �����Ȼ�����
        Clear(-2);  // ���ģ�建����
    }

    void FBO::CopyColor(const FBO& src, GLuint srcIndex, const FBO& dst, GLuint dstIndex){
        LOG_TRACK;
        LOG_ASSERT(srcIndex < src.mColorAttachments.size(), "��ɫ���������� {0} ������Χ", srcIndex);
        LOG_ASSERT(dstIndex < dst.mColorAttachments.size(), "��ɫ���������� {0} ������Χ", dstIndex);

        // ���Դ��Ŀ���������Ĵ�С��ͬ����Ӧ�ò�ֵ
        GLuint fw = src.mWidth, fh = src.mHeight;
        GLuint tw = dst.mWidth, th = dst.mHeight;

        glNamedFramebufferReadBuffer(src.mId, GL_COLOR_ATTACHMENT0 + srcIndex);
        glNamedFramebufferDrawBuffer(dst.mId, GL_COLOR_ATTACHMENT0 + dstIndex);
        glBlitNamedFramebuffer(src.mId, dst.mId, 0, 0, fw, fh, 0, 0, tw, th, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    }

    void FBO::CopyDepth(const FBO& src, const FBO& dst){
        LOG_TRACK;
        // ȷ���ڵ��ô˺���ʱȫ�ֽ�����GL_FRAMEBUFFER_SRGB��
        // �����������ɫ�ռ�У�������ڸ��ƹ��������ֵ������Gamma����...
        GLuint fw = src.mWidth, fh = src.mHeight;
        GLuint tw = dst.mWidth, th = dst.mHeight;
        glBlitNamedFramebuffer(src.mId, dst.mId, 0, 0, fw, fh, 0, 0, tw, th, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    }

    void FBO::CopyStencil(const FBO& src, const FBO& dst){
        LOG_TRACK;
        // ȷ���ڵ��ô˺���ʱȫ�ֽ�����GL_FRAMEBUFFER_SRGB��
        // �����������ɫ�ռ�У�������ڸ��ƹ�����ģ��ֵ������Gamma����...
        GLuint fw = src.mWidth, fh = src.mHeight;
        GLuint tw = dst.mWidth, th = dst.mHeight;
        glBlitNamedFramebuffer(src.mId, dst.mId, 0, 0, fw, fh, 0, 0, tw, th, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    }
    
}