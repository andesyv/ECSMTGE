#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "meshdata.h"
#include <QtMath> // temp for qDegreesRadians in spot light component

// For ScriptComponent
#include <QFile>
#include <QJSEngine>

#include "qentity.h"

#include <QJsonObject>

enum class ComponentType
{
    Transform,
    Physics,
    Mesh,
    Camera,
    Input,
    Sound,
    LightPoint,
    LightDirectional,
    LightSpot,
    Script,
    Collider,
    Particle,
    Other
};

// Used by UI so it knows what components are available to add if the entity doesnt have one.
const std::vector<ComponentType> ComponentTypes = {ComponentType::Mesh, ComponentType::Transform, ComponentType::Physics,
                                                   ComponentType::Input, ComponentType::Sound, ComponentType::LightPoint,
                                                   ComponentType::LightDirectional, ComponentType::LightSpot, ComponentType::Script,
                                                   ComponentType::Collider};

/** Base component class in the ECS system.
 * Every component that would like to be included
 * in the ECS system must inherit from this struct.
 * @brief Base component class in the ECS system.
 */
struct Component
{
    unsigned int entityId;
    bool valid : 1;
    ComponentType type;

    Component(unsigned int _eID = 0, bool _valid = false, ComponentType typeIn = ComponentType::Other)
        : entityId{_eID}, valid(_valid), type(typeIn)
    {}

    virtual void reset()=0;

    virtual QJsonObject toJSON();
    virtual void fromJSON(QJsonObject object);


    virtual ~Component(){}
};

/** Extra info regarding entities that is mainly used by the editor.
 * The ECS also use it to make sure if entities exist or not.
 * @brief Extra info regarding entities that is mainly used by the editor.
 */
struct EntityInfo : public Component
{
    std::string name{};
    bool shouldShowInEditor{};

    EntityInfo(unsigned int _eID = 0, bool _valid = false, bool _shouldShowInEditor = true)
        : Component(_eID, _valid, ComponentType::Other), shouldShowInEditor(_shouldShowInEditor)
    {}

    virtual void reset() override
    {
        name = "";
        shouldShowInEditor = false;
    }

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component describing position, rotation, scale and children for entity.
 * Every entity needs a transform to be able to be visible or interact with the physics system.
 * @brief Component describing position, rotation, scale and children for entity.
 */
struct TransformComponent : public Component
{
    bool updated : 1;
    std::vector<unsigned int> children;
    gsl::Vector3D position{};
    gsl::Quaternion rotation{};
    gsl::Vector3D scale{1,1,1};
    bool meshBoundsOutdated : 1;
    bool colliderBoundsOutdated : 1;

    TransformComponent(unsigned int _eID = 0, bool _valid = false,
                    const gsl::vec3& _pos = gsl::vec3{},
                    const gsl::vec3& _scale = gsl::vec3{1.f, 1.f, 1.f},
                    const gsl::quat& _rot = gsl::quat{})
        : Component (_eID, _valid, ComponentType::Transform), updated{true}, position{_pos},
          rotation{_rot}, scale{_scale}, meshBoundsOutdated{true}, colliderBoundsOutdated{true}
    {}


    virtual void reset() override
    {
        updated = true;
        children.clear();
        position = gsl::vec3{};
        rotation = gsl::Quaternion{};
        scale = gsl::vec3{1};
        meshBoundsOutdated = true;
        colliderBoundsOutdated = true;
    }

    void addPosition(const gsl::vec3& pos);
    void addRotation(const gsl::quat& rot);
    void addScale(const gsl::vec3& scl);
    void setPosition(const gsl::vec3& pos);
    void setRotation(const gsl::quat& rot);
    void setScale(const gsl::vec3& scl);

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component describing velocity, acceleration, mass and other physics stuff for entity.
 * Entities with a collision component without a physics component is static.
 * @brief Component describing velocity, acceleration, mass and other physics stuff for entity.
 */
struct PhysicsComponent : public Component
{
    gsl::vec3 velocity{};
    gsl::vec3 acceleration{};
    float mass{1.f};

    PhysicsComponent(unsigned int _eID = 0, bool _valid = false,
                     const gsl::vec3& _velocity = gsl::vec3{})
        : Component (_eID, _valid, ComponentType::Physics), velocity{_velocity}
    {}

    virtual void reset() override
    {
        velocity = gsl::vec3{};
        acceleration = gsl::vec3{};
        mass = 1.f;
    }

    void setVelocity(gsl::vec3 newVel);
    void setAcceleration(gsl::vec3 newAcc);

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component describing how the entity looks.
 * To be visible entities need to have a transform component as well.
 * @brief Component describing how the entity looks.
 */
struct MeshComponent : public Component
{
    bool isVisible : 1;
    MeshData meshData{};
    Material mMaterial{};
    bool renderWireframe : 1;

    /* World Space Bounds of meshComponent
     * If transform component is missing,
     * bounds will be the same meshdata's
     * local bounds.
     */
    MeshData::Bounds bounds{};

    MeshComponent(unsigned int _eID = 0, bool _valid = false,
                  const MeshData& _meshData = MeshData{}, const Material& _material = Material{}, bool _visible = false, bool _renderWireframe = false)
        : Component (_eID, _valid, ComponentType::Mesh), isVisible{_visible}, meshData{_meshData}, mMaterial{_material}, renderWireframe{_renderWireframe}
    {}

    virtual void reset() override
    {
        isVisible = true;
        meshData = MeshData{};
        mMaterial = Material{};
        renderWireframe = false;
    }

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component defining a camera entity.
 * Pitch and yaw are used by the camerasystem to set the rotation,
 * which is the one used for the viewmatrix (the camera viewport).
 * @brief Component defining a camera entity.
 */
struct CameraComponent : public Component
{
    bool isEditorCamera : 1;
    float pitch{0.f}, yaw{0.f};
    gsl::Matrix4x4 viewMatrix;
    gsl::Matrix4x4 projectionMatrix;
    gsl::Matrix4x4 invProjectionMatrix;

    CameraComponent(unsigned int _eID = 0, bool _valid = false, bool editorCamera = false,
                    const gsl::mat4& vMat = gsl::mat4{}, const gsl::mat4& pMat = gsl::mat4{},
                    const gsl::mat4& invPMat = gsl::mat4{})
        : Component (_eID, _valid, ComponentType::Camera), isEditorCamera{editorCamera},
          viewMatrix{vMat}, projectionMatrix{pMat}, invProjectionMatrix{invPMat}
    {}

    virtual void reset() override
    {
        pitch = yaw = 0.f;
        viewMatrix = gsl::mat4{};
        projectionMatrix = gsl::mat4{};
    }

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component that enables input being sent to a entity.
 * Entities with this component will receive input to
 * the script functions or in the editor viewport
 * depending on if conrolledWhilePlaying is true or false.
 * @brief Component that enables input being sent to a entity.
 */
struct InputComponent : public Component
{
    bool controlledWhilePlaying : 1;

    InputComponent(unsigned int _eID = 0, bool _valid = false, bool _controlledWhilePlaying = true)
        : Component(_eID, _valid, ComponentType::Input), controlledWhilePlaying{_controlledWhilePlaying}
    {}

    virtual void reset() override
    {
        controlledWhilePlaying = false;
    }
    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component that describes sound of entity.
 * Entities with this component can play sounds that can be heard
 * by the camera entity. The listener is always placed at the
 * camera entity's position.
 * @brief Component that describes sound of entity.
 */
struct SoundComponent : public Component
{
    bool isLooping : 1;
    bool isMuted : 1;
    int mSource;
    float pitch = 1.f;
    float gain = .7f; //JT was here, Savner deg Ole, 3D er bra men vi mangler deg. *Smask*
    bool autoplay{false};
    std::string name;

    SoundComponent(unsigned int _eID = 0, bool _valid = false, bool _isLooping = false, bool _isMuted = false)
        : Component(_eID, _valid, ComponentType::Sound), isLooping{_isLooping}, isMuted{_isMuted}, mSource{-1}
    {}

    virtual void reset() override
    {
        isLooping = false;
        isMuted = false;
        mSource = -1;
        pitch = 1.f;
        gain = .7f;
        name = "";
    }

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component describing a point light.
 * Entities with a point light component will emit light
 * in all directions from their centre.
 * @brief Component describing a point light.
 */
struct PointLightComponent : public Component
{
    gsl::vec3 color;
    float intensity;
    float radius;
    float maxBrightness; // Not used, but in theory to clamp attuenation

    PointLightComponent(unsigned int _eID = 0, bool _valid = false,
                        gsl::vec3 _color = gsl::vec3(1.f, 1.f, 1.f),
                        float _intensity = 1.f,
                        float _radius = 1.f,
                        float _maxBrightness = 2.f)
        : Component(_eID, _valid, ComponentType::LightPoint),
          color(_color), intensity(_intensity),
          radius{_radius}, maxBrightness(_maxBrightness)
    {}

    virtual void reset() override
    {
        color = gsl::vec3{1};
        intensity = 1.f;
        maxBrightness = 2.f;
        radius = 1.f;
    }

    float calculateRadius() const
    {
        // Note: We don't bother about manualy calculating attuenation falloff anymore and we just pass the radius directly.
//        float constant = 1.0f;
//        return (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * brightness)))
//                / (2.0f * quadratic);
        return 0.f;
    }

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component describing a spot light.
 * Entities with a spot light component will emit light
 * in a directional cone.
 * @brief Component describing a spot light.
 */
struct SpotLightComponent : public Component
{
    gsl::vec3 color;
    float intensity;
    float cutOff;
    float outerCutOff;
    float linear;
    float quadratic;
    float constant;

    SpotLightComponent(unsigned int _eID = 0, bool _valid = false,
                       gsl::vec3 _color = gsl::vec3(1.f, 1.f, 1.f),
                       float _intensity = 1.f,
                       float _cutOff = qDegreesToRadians(25.f),
                       float _outerCutOff = qDegreesToRadians(35.f),
                       float _linear = 0.045f,
                       float _quadratic = 0.0075f,
                       float _constant = 1.0f)
        : Component(_eID, _valid, ComponentType::LightSpot),
           color(_color), intensity(_intensity),
          cutOff(_cutOff), outerCutOff(_outerCutOff), linear(_linear),
          quadratic(_quadratic), constant(_constant)
    {}

    virtual void reset() override
    {
        color = gsl::vec3{1};
        intensity = 1.f;
        cutOff = qDegreesToRadians(25.f);
        outerCutOff = qDegreesToRadians(35.f);
        linear = 0.045f;
        quadratic = 0.0075f;
        constant = 1.f;
    }

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component describing a directional light.
 * Entities with a directional light will emit light in
 * direction. Directional lights are global across the scene.
 * @brief Component describing a directional light.
 */
struct DirectionalLightComponent : public Component
{
    gsl::vec3 color;
    float intensity;

    DirectionalLightComponent(unsigned int _eID = 0, bool _valid = false,
                              gsl::vec3 _color = gsl::vec3(1.f, 1.f, 1.f),
                              float _intensity = 1.f)
        : Component(_eID, _valid, ComponentType::LightDirectional),
          color(_color), intensity(_intensity)
    {}

    virtual void reset() override
    {
        color = gsl::vec3{1};
        intensity = 1.f;
    }

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component describing a entity that can be scripted.
 * All components with a script component can have a custom
 * script file be run each frame with callback functions
 * that will be run according the events they're associated with.
 * Each scriptcomponent can run 1 scriptfile, but multiple
 * scriptcomponents can run the same file.
 * @brief Component describing a entity that can be scripted.
 */
struct ScriptComponent : public Component
{
    QJSEngine* engine{nullptr};
    std::string filePath;
    QEntity* JSEntity{};
    bool beginplayRun : 1;

    ScriptComponent(unsigned int _eID = 0, bool _valid = false);
    ScriptComponent(const ScriptComponent& rhs) = delete;
    ScriptComponent(ScriptComponent&& rhs);

    ScriptComponent& operator= (const ScriptComponent& rhs) = delete;
    ScriptComponent& operator= (ScriptComponent&& rhs);

    virtual void reset() override;

    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;

    virtual ~ScriptComponent() override;
};

/** Component describing collision handling of an entity.
 * Entities with a collider component will collide with
 * other entities with a collider component. Does not need
 * to have a physics component but is static without one.
 *
 * Note: Only AABB and Sphere collision types are implemented yet.
 * @brief Component describing collision handling of an entity.
 */
struct ColliderComponent : public Component
{
    enum Type : char
    {
        None = 0,
        /** Axis Aligned Bounding Box collision.
         * AABB extents is a vec3 defining the
         * length of the bounding box in each
         * direction from min to max.
         * Meaning the distance from the centre
         * out to the maximum point is defined by
         * max = centre + 0.5 * extents
         */
        AABB,
        /** Box collision.
         * Box collision works the same
         * as AABB collision, but is not axis
         * aligned meaning it can rotate in all directions.
         */
        BOX,
        /** Sphere collision.
         * Sphere extents is a float describing the radius
         * from the centre to the edges of the sphere in
         * all directions.
         */
        SPHERE,
        CAPSULE
    };
    static constexpr const char* typeNames[]{"None", "AABB", "Box", "Sphere", "Capsule"};


    Type collisionType;

    ColliderComponent(unsigned int _eID = 0, bool _valid = false)
        : Component(_eID, _valid, ComponentType::Collider), collisionType(None)
    {}

    virtual void reset() override
    {
        collisionType = None;
    }

    std::variant<gsl::vec3, float, std::pair<float, float>> extents;
    /** Collision box describing maximum extent of collider.
     * A simple AABB collision box that is used for early collision testing
     * before more accurate collision testing occours.
     * @brief Collision box describing maximum extent of collider.
     */
    struct Bounds
    {
        gsl::vec3 centre{0.f, 0.f, 0.f};

        /* Note: Bounds extend by default 0.5 units
         * away from the centre.
         */
        gsl::vec3 extents{1.f, 1.f, 1.f};

        std::pair<gsl::vec3, gsl::vec3> minMax() const;
    } bounds;


    virtual QJsonObject toJSON() override;
    virtual void fromJSON(QJsonObject object) override;
};

/** Component describing a particle emitter.
 * An entity with a particle component will emit particles
 * from it's centre.
 * Note: Not implemented yet.
 * @brief Component describing a particle emitter.
 */
struct ParticleComponent : public Component
{
    ParticleComponent(unsigned int _eID = 0, bool _valid = false)
        : Component{_eID, _valid, ComponentType::Particle}
    {}

    void reset() override;
    QJsonObject toJSON() override;
    void fromJSON(QJsonObject object) override;
};

// .. etc

#endif // COMPONENTS_H
