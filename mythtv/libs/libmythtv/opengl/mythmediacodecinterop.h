#ifndef MYTHMEDIACODECINTEROP_H
#define MYTHMEDIACODECINTEROP_H

// Qt
#include <QMutex>
#include <QWaitCondition>
#include <QJniEnvironment>
#include <QJniObject>

// MythTV
#include "mythopenglinterop.h"

extern "C" MTV_PUBLIC void Java_org_mythtv_video_SurfaceTextureListener_frameAvailable(JNIEnv*, jobject, jlong Wait, jobject);

class MythMediaCodecInterop : public MythOpenGLInterop
{
  public:
    static MythMediaCodecInterop* CreateMediaCodec(MythPlayerUI* Player, MythRenderOpenGL* Context, QSize Size);
    virtual std::vector<MythVideoTextureOpenGL*> Acquire(MythRenderOpenGL *Context,
                                                         MythVideoColourSpace *ColourSpace,
                                                         MythVideoFrame *Frame, FrameScanType Scan) override;
    void* GetSurface(void);

  protected:
    MythMediaCodecInterop(MythPlayerUI* Player, MythRenderOpenGL *Context);
   ~MythMediaCodecInterop() override;
    bool Initialise(QSize Size);

  private:
    QWaitCondition    m_frameWait;
    QMutex            m_frameWaitLock;
    bool              m_colourSpaceInitialised;
    QJniObject        m_surface;
    QJniObject        m_surfaceTexture;
    QJniObject        m_surfaceListener;
    jfloatArray       m_textureTransform;
    QMatrix4x4        m_transform;
};

#endif // MYTHMEDIACODECINTEROP_H
