
#include "Just/Tool/AssetsManager.h"
#include "Just/Camera/ProjectiveCamera.h"
#include "Just/Render/RenderContext.h"
#include "Just/Render/Scene.h"
#include "Just/Rasterizer/SimpleRasterizer.h"
#include "Just/Tool/Timer.h"

int main()
{
    //数据
    //==================================================================================================
    Point3f target(0.0f, 0.0f, 0.0f);
    Point3f origin(0.0f, 0.0f, 1.0f);
    Vector3f up(0, 1, 0);
    Point2i res(768, 768);
    float fov = 45;
    float zNear = 1e-4f;
    float zFar = 1e4f;
    //资源
    //==================================================================================================
    auto texture_diffuse = std::shared_ptr<Texture2D>(AssetsManager::LoadTexture2D("res/texture2.png"));
    Timer timer;
    timer.Begin();
    auto mesh_african = std::shared_ptr<Mesh>(AssetsManager::LoadMesh("res/model_african.obj"));
    timer.End();
    std::cout << "[load time]: " << timer.time << "ms" << std::endl;
    //相机
    //==================================================================================================
    auto camera = std::make_shared<PerspectiveCamera>(res, LookAt(origin, target, up), fov, zNear, zFar);
    auto modelTransform = Translate(Vector3f(0, 0, -3));
    auto MVP = camera->GetProjective() * camera->GetView() * modelTransform;
    //帧缓冲
    //==================================================================================================
    auto frameBuffer = std::make_shared<FrameBuffer>(res);
    //上下文
    //==================================================================================================
    auto context = std::make_shared<RenderContext>(camera, frameBuffer);
    context->SetUniform("MVP", MVP.matrix);
    context->AddTexture(0, texture_diffuse);
    //构建场景
    //==================================================================================================
    timer.Begin();
    auto accel = std::make_shared<NaiveAccel>();
    auto scene = std::make_shared<Scene>(accel);
    scene->AddMesh(mesh_african);
    scene->BuildAccel();
    timer.End();
    std::cout << "[build time]: " << timer.time << "ms" << std::endl;
    //渲染场景
    //==================================================================================================
    timer.Begin();
    auto rasterizer = std::make_shared<SimpleRasterizer>(scene, context);
    rasterizer->Render();
    timer.End();
    std::cout << "[render time]: " << timer.time << "ms" << std::endl;
    //==================================================================================================
    //保存
    SaveImageToBMP("output/rasterize.bmp", res.x, res.y, 4, context->frameBuffer->colorBuffer);
    return 0;
}
