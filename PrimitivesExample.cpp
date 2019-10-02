#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/MeshTools/Compile.h>
#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Shaders/Phong.h>
#include <Magnum/Trade/MeshData3D.h>
#include <Magnum/Math/Quaternion.h>

#include "externals/entt.hpp"

namespace Magnum { namespace Examples {

// --------------------------------------------------------------
//
// Components
//
// --------------------------------------------------------------

using Position = Math::Vector3<float>;
using Orientation = Math::Quaternion<float>;
using Radian = Math::Rad<float>;
using Color = Color4;

struct Scale : public Vector3 {
    using Vector3::Vector3;
};

struct Identity {
    std::string name;
};

struct Witness {
    Radian fov;
    float aspectRatio;
    float near;
    float far;
    Vector2i viewport;
};

struct Drawable {
    GL::Mesh mesh { NoCreate };
    Shaders::Phong shader{ NoCreate };
    Color4 color;
};

// ---------------------------------------------------------
//
// Systems
//
// ---------------------------------------------------------

static void MouseMoveSystem(entt::registry& registry, Vector2 distance) {
    registry.view<Orientation>().each([distance](auto& ori) {
        ori = (
            Quaternion::rotation(Rad{ distance.y() }, Vector3(1.0f, 0, 0)) *
            ori *
            Quaternion::rotation(Rad{ distance.x() }, Vector3(0, 1.0f, 0))
        ).normalized();
    });
}

static void MouseReleaseSystem(entt::registry& registry) {
    registry.view<Drawable>().each([](auto& drawable) {
        drawable.color = Color3::fromHsv({ drawable.color.hue() + 50.0_degf, 1.0f, 1.0f });
    });
}

static void AnimationSystem(entt::registry& registry) {
    Debug() << "Animating..";
}

static void PhysicsSystem(entt::registry& registry) {
    Debug() << "Simulating..";
}

static void RenderSystem(entt::registry& registry, Matrix4 projection) {
    Debug() << "Rendering..";

    registry.view<Identity, Position, Orientation, Scale, Drawable>().each(
        [projection](auto& id, auto& pos, auto& ori, auto& scale, auto& drawable)
    {
        auto transform = (
            Matrix4::scaling(scale) *
            Matrix4::rotation(ori.angle(), ori.axis().normalized()) *
            Matrix4::translation(pos)
        );

        // Problem area 1: Shader program with function and data combined
        // Ideal solution: Uniforms a separate component
        drawable.shader.setLightPosition({7.0f, 7.0f, 2.5f})
                       .setLightColor(Color3{1.0f})
                       .setDiffuseColor(drawable.color)
                       .setAmbientColor(Color3::fromHsv({drawable.color.hue(), 1.0f, 0.3f}))
                       .setTransformationMatrix(transform)
                       .setNormalMatrix(transform.rotationScaling())
                       .setProjectionMatrix(projection);

        // Problem area 2: Vertex data and rendering function combined
        // Ideal solution: Vertex data a separate component, shader takes mesh as component
        drawable.mesh.draw(drawable.shader);
    });
}

// ---------------------------------------------------------
//
// Implementation
//
// ---------------------------------------------------------

using namespace Magnum::Math::Literals;

class ECSExample : public Platform::Application {
public:
    explicit ECSExample(const Arguments& arguments);

private:
    void drawEvent() override;
    void mousePressEvent(MouseEvent& event) override;
    void mouseReleaseEvent(MouseEvent& event) override;
    void mouseMoveEvent(MouseMoveEvent& event) override;

    GL::Mesh _mesh;
    Shaders::Phong _shader;
    entt::registry _registry;

    Matrix4 _projection;
    Vector2i _previousMousePosition;
};

ECSExample::ECSExample(const Arguments& arguments) :
    Platform::Application{ arguments, Configuration{}
        .setTitle("Magnum Primitives Example") }
{
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    _projection =
        Matrix4::perspectiveProjection(
            35.0_degf, Vector2{ windowSize() }.aspectRatio(), 0.01f, 100.0f) *
        Matrix4::translation(Vector3::zAxis(-10.0f));

    // Create entities
    auto box = _registry.create();

    // Assign components
    _registry.assign<Identity>(box, "Box");
    _registry.assign<Position>(box, 0.0f, 0.0f, 0.0f);
    _registry.assign<Orientation>(box, Quaternion::rotation(30.0_degf, Vector3(0, 1.0f, 0)));
    _registry.assign<Scale>(box, 1.0f);
    _registry.assign<Drawable>(box,
        MeshTools::compile(Primitives::cubeSolid()),
        Shaders::Phong{},
        Color4(.4f, .2f, .9f)
    );
}

void ECSExample::drawEvent() {
    GL::defaultFramebuffer.clear(
        GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    // Should the system take _projection as argument?
    RenderSystem(_registry, _projection);

    swapBuffers();
}

void ECSExample::mousePressEvent(MouseEvent& event) {
    if (event.button() != MouseEvent::Button::Left) return;
    _previousMousePosition = event.position();
    event.setAccepted();
}

void ECSExample::mouseReleaseEvent(MouseEvent& event) {
    if (event.button() != MouseEvent::Button::Left) return;

    // Should the system handle all mouse events, instead of individual ones?
    MouseReleaseSystem(_registry);

    event.setAccepted();
    redraw();
}

void ECSExample::mouseMoveEvent(MouseMoveEvent& event) {
    if (!(event.buttons() & MouseMoveEvent::Button::Left)) return;

    const float sensitivity = 3.0f;
    const Vector2 distance = (
        Vector2{ event.position() - _previousMousePosition } /
        Vector2{ GL::defaultFramebuffer.viewport().size() }
    ) * sensitivity;

    // Should the system compute delta?
    // If so, where does state go, i.e. _previousMousePosition?
    MouseMoveSystem(_registry, distance);

    _previousMousePosition = event.position();
    event.setAccepted();

    redraw();
}

}}

MAGNUM_APPLICATION_MAIN(Magnum::Examples::ECSExample)