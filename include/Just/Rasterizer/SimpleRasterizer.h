#pragma once

#include "Just/Common.h"
#include "Just/Render/Renderer.h"
#include "Just/Math/Color.h"

struct SimpleRasterizer : public Rasterizer
{
public:
    SimpleRasterizer(const std::shared_ptr<Scene> &scene, const std::shared_ptr<RenderContext> &context)
            : Rasterizer(scene, context) {};
    ~SimpleRasterizer() override = default;
    virtual void Render() override;
private:
    virtual void DrawTriangle() override;
};

void SimpleRasterizer::Render()
{
    //遍历所有三角形网格
    for (const auto &mesh: scene->accel->meshes)
    {
        //遍历网格所有三角形
        for (int i = 0; i < mesh->indices.size(); i += 3)
        {
            //更新三角形顶点信息
            for (int j = 0; j < 3; j++)
            {
                auto index = mesh->indices[i + j];
                context->triangle[j].pos4 = Point4f{mesh->positions[index], 1};
                context->triangle[j].texcoord = mesh->texcoords[index];
                context->triangle[j].normal = mesh->normals[index];
            }
            //绘制当前三角形
            DrawTriangle();
        }
    }
}
void SimpleRasterizer::DrawTriangle()
{
    Bounds2i rect;
    //几何阶段
    for (auto &vertex: context->triangle)
    {
        //vertex shader
        {
            auto &MVP = context->GetUniform<Matrix4f>("MVP");
            vertex.pos4 = MVP * vertex.pos4;
        }
        float w = vertex.pos4.w;
        //CVV 剔除
        if (w == 0.0f) return;
        if (vertex.pos4.z < 0.0f || vertex.pos4.z > w) return;
        if (vertex.pos4.x < -w || vertex.pos4.x > w) return;
        if (vertex.pos4.y < -w || vertex.pos4.y > w) return;

        //w的倒数
        vertex.rhw = 1.0f / w;
        //透视除法
        vertex.pos4 *= vertex.rhw;

        //屏幕映射
        vertex.pos2f = Point2f{context->camera->screenToRaster(vertex.pos4)};
        //四舍五入
        vertex.pos2i = Point2i{(int) (vertex.pos2f.x + 0.5f), (int) (vertex.pos2f.y + 0.5f)};
        //vertex.pos2i = Point2i{(int) std::round(vertex.pos2f.x), (int) std::round(vertex.pos2f.y)};


        //设置三角形矩形范围
        rect.Expand(vertex.pos2i);
    }

    //限制在屏幕范围内
    rect.Clamp(context->frameBuffer->screenRect);

    //背面剔除
    if (Cross(context->triangle[1].pos4 - context->triangle[0].pos4,
              context->triangle[2].pos4 - context->triangle[0].pos4).z >= 0)
        return;

    //光栅化阶段
    for (int y = rect.pMin.y; y <= rect.pMax.y; y++)
    {
        for (int x = rect.pMin.x; x <= rect.pMax.x; x++)
        {

            //计算当前像素的重心坐标
            auto [alpha, beta, gamma] = CalcBarycentric(context->triangle, (float) x, (float) y);

            //检查点是否在三角形内
            if (beta < 0 || gamma < 0 || gamma + beta > 1.0f + kEpsilon) continue;

            //插值深度 w=z的倒数
            float rhw = alpha * context->triangle[0].rhw +
                        beta * context->triangle[1].rhw +
                        gamma * context->triangle[2].rhw;

            //early-z
            int index = x + y * context->frameBuffer->res.x;
            if (rhw < context->frameBuffer->depthBuffer[index]) continue;
            context->frameBuffer->depthBuffer[index] = rhw;

            //透视插值校正
            float w = 1.0f / ((rhw == 0.0f) ? 1.0f : rhw);
            alpha = alpha * w * context->triangle[0].rhw;
            beta = beta * w * context->triangle[1].rhw;
            gamma = gamma * w * context->triangle[2].rhw;


            //插值纹理坐标
            auto texcoord = alpha * context->triangle[0].texcoord + beta * context->triangle[1].texcoord +
                            gamma * context->triangle[2].
                                    texcoord;
            //插值法线
            //auto normal = alpha * context->triangle[0].normal + beta * context->triangle[1].normal + gamma * context->triangle[2].normal;
            RGBA32 fragColor;

            //片元着色
            {
                const auto &diffuseMap = context->GetTexture(0);
                fragColor = diffuseMap->SampleByBilinear(texcoord);
            }
            context->frameBuffer->colorBuffer[index] = fragColor;
        }
    }
}