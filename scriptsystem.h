#ifndef SCRIPTSYSTEM_H
#define SCRIPTSYSTEM_H

#include <QObject>
#include <QJSValue>
#include <memory>
#include <vector>
#include "world.h"
#include "qentity.h"
#include "qjsengine.h"
#include "componentdata.h"

class HitInfo;

/**
 * @brief The singleton class responsible of handling all script functionality.
 * Note that this class does not have a QJSEngine, all script components have one each instead to help keep global spaces separated.
 * The instance of this class is given to all QJSEngines as a global object under the name 'engine'.
 * This means that any public slots or Q_INVOKABLE public functions in this class are callable from JS.
 * Example JS code: let entity = engine.spawnCube();
 */
class ScriptSystem : public QObject
{
    Q_OBJECT
    friend class QScriptSystemPointer;
public:
    static ScriptSystem* get()
    {
        static ScriptSystem instance;
        return &instance;
    }

    /** How often the script resource garbage collection
     * should run, in frames. (10 is every tenth frame)
     * @brief garbageCollectionFrequency
     */
    unsigned int garbageCollectionFrequency = 15;

    /**
     * @brief Update function for all scripts. Runs all functions necessary every frame.
     */
    void update(std::vector<ScriptComponent>& scripts, std::vector<InputComponent>& inputs, const std::vector<QString>& pressed, const std::vector<QString> &released, const QPoint& point, std::vector<HitInfo> hitInfos, float deltaTime);

    /**
     * @brief Calls the end play function on all scripts. Called when the stop button is pressed.
     */
    void endPlay(std::vector<ScriptComponent>& comps);

    /**
     * @brief Propagate changes done in C++ back to JS
     */
    void updateJSComponents(std::vector<ScriptComponent>& comps);

    /**
     * @brief Propagate changes done in JS back to C++
     */
    void updateCPPComponents(std::vector<ScriptComponent>& comps);

    QString checkError(QJSValue value);

    QEntity* getEntityWrapper(unsigned int entity);

    /** Garbage collection
     * Removes unwanted resources from the script engine.
     * @brief Garbage collection
     */
    void takeOutTheTrash(std::vector<ScriptComponent>& comps);

    /** Find all the global variable names in a script file.
     * @brief findGlobalsInFile
     * @param file - file to search
     * @return a list of all variable names in the file.
     */
    std::vector<QString> findGlobalsInFile(const std::string& file) const;

    /** Saves the variable names from the script in com and
     * adds them to globalVariables list.
     * @param comp - script component
     */
    void cacheGlobalVariables(ScriptComponent& comp);

    /** Clears garbage from last garbage collection run.
     * Might seem a bit ironic that the garbage collection
     * needs to run it's own garbage collection...
     * @brief initGarbageCollection
     */
    void initGarbageCollection();

    /**
     * @brief Returns hardcoded functions provided in all scripts
     */
    const QString getEntityHelperFunctions() { return mHelperFuncs; }

    /**
     * @brief Loads the js file given. Returns true if the file is successfully evaluated and set.
     */
    bool load(ScriptComponent& comp, const std::string& file);

    /**
     * @brief Calls a JS function on the current file loaded. Returns true if successful.
     */
    void call(ScriptComponent& comp, const std::string& function);

    /**
     * @brief Calls a JS function on the current file loaded. Returns true if successful.
     */
    QJSValue call(const std::string& function);

    /**
     * @brief Calls a JS function with params on the current file loaded. Returns true if successful.
     */
    QJSValue call(ScriptComponent& comp, const std::string& function, QJSValueList params);


    /**
     * @brief Calls a JS function with params on the current file loaded. Returns true if successful.
     */
    QJSValue call(const std::string& function, QJSValueList params);

    /**
     * @brief Called from QEntity when a component is added through JS. Only adds one if it's not already added already.
     * If a new component is added, it is deferred until the end of the update loop.
     */
    template<class Comp, typename std::enable_if<std::is_base_of<Component, Comp>::value>::type* = nullptr>
    QJSValue addComponent(unsigned entity)
    {
        if(!currentComp)
            return QJSValue();

        Comp comp{};
        QJsonObject object = comp.toJSON();

        auto componentArray = currentComp->engine->globalObject().property("accessedComponents");
        auto length = componentArray.property("length").toInt();
        if(length > 0)
        {
            for(unsigned i = 0; i < static_cast<unsigned>(length); ++i)
            {
                auto property = componentArray.property(i);
                auto jsObject = property.toVariant().toJsonObject();
                if(jsObject["ID"].toInt() == static_cast<int>(entity) && jsObject["ComponentType"].toInt() == object["ComponentType"].toInt())
                {
                    return componentArray.property(i);
                }
            }
        }

        object.insert("ID", QJsonValue(static_cast<int>(entity)));
        auto value = currentComp->engine->toScriptValue(object);
        currentComp->engine->globalObject().property("accessedComponents").setProperty(static_cast<unsigned>(length), value);

        return value;
    }

    /**
     * @brief Called from QEntity when a component is retrieved through JS.
     */
    template<class Comp, typename std::enable_if<std::is_base_of<Component, Comp>::value>::type* = nullptr>
    QJSValue getComponent(unsigned entity)
    {
        if(!currentComp)
            return QJSValue();

        auto comp = World::getWorld().getEntityManager()->getComponent<Comp>(entity);
        if(!comp)
            return QJSValue();

        auto object = comp->toJSON();

        auto componentArray = currentComp->engine->globalObject().property("accessedComponents");
        auto length = componentArray.property("length").toInt();
        if(length > 0)
        {
            for(unsigned i = 0; i < static_cast<unsigned>(length); ++i)
            {
                auto jsObject = componentArray.property(i).toVariant().toJsonObject();

                if(jsObject["ID"].toInt() == static_cast<int>(entity) && jsObject["ComponentType"].toInt() == object["ComponentType"].toInt())
                {
                    return componentArray.property(i);
                }
            }
        }

        object.insert("ID", QJsonValue(static_cast<int>(entity)));
        auto value = currentComp->engine->toScriptValue(object);
        currentComp->engine->globalObject().property("accessedComponents").setProperty(static_cast<unsigned>(length), value);
        return value;
    }

    /**
     * @brief Executes one off raw js code from the mini editor. Returns true if successfull.
     * @deprecated No longer supported :( Will be missed. RIP. F.
     */
    bool execute(ScriptComponent& comp, QString function, QString contents, QString fileName);

    Q_PROPERTY(float deltaTime MEMBER mDeltaTime)

public slots:
    /**
     * @brief Spawns and returns a cube at (0,0,0). This includes a transform- and mesh component. Mesh is set to a cube. Defaults to visible.
     * Example: let cube = engine.spawnCube();
     */
    QObject* spawnCube();
    /**
     * @brief Spawns and returns an entity with no components.
     * Example: let spawnedEntity = engine.spawnEntity();
     */
    QObject* spawnEntity();

    /**
     * @brief Returns an array of all entity IDs that has the given component.
     * Example: let entityIDs = engine.getAllEntityIDsByComponent("mesh");
     */
    QJSValue getAllEntityIDsByComponent(const QString& name);

    /**
     * @brief Returns an array of all entityIDs.
     * Example: let entityIDs = engine.getAllEntityIDs();
     */
    QJSValue getAllEntityIDs();

    /**
     * @brief Returns an entity with the given ID.
     * Example: let entity = engine.getEntity(2);
     */
    QObject* getEntity(unsigned int id);

    /**
     * @brief Destroys the passed in entity.
     */
    void destroyEntity(unsigned entity);

private:
    ScriptSystem(){}

    /**
     * @brief Called on play or when a new script component is added to an entity on runtime.
     */
    void beginPlay(std::vector<ScriptComponent>& comps);
    /**
     * @brief Called every frame.
     */
    void tick(float deltaTime, std::vector<ScriptComponent>& comps);

    /**
     * @brief Called on all script components with an input pressed function and if any input is registered as pressed.
     */
    void runKeyPressedEvent(std::vector<ScriptComponent>& scripts, std::vector<InputComponent>& inputs, const std::vector<QString>& keys);

    /**
     * @brief Called on all script components with an input released function and if any input is registered as released this frame.
     */
    void runKeyReleasedEvent(std::vector<ScriptComponent>& scripts, std::vector<InputComponent>& inputs, const std::vector<QString>& keys);

    /**
     * @brief Called on all script components with a mouse moved function and if the offset is greater than zero.
     */
    void runMouseOffsetEvent(std::vector<ScriptComponent>& scripts, std::vector<InputComponent>& inputs, const QPoint& point);

    /**
     * @brief Called on all script components with a hit event function and if there is any collisions associated with the respective component.
     */
    void runHitEvents(std::vector<ScriptComponent>& comps, std::vector<HitInfo> hitInfos);

    /**
     * @brief Updates all JS Components for a given JS engine from the respective CPP components. Called after ticking scripts.
     * @param Script component to update
     */
    void updateJSComponent(ScriptComponent& comp);

    /**
     * @brief Updates all CPP components based on the ones used in JS. Called at the before ticking scripts.
     */
    void updateCPPComponent(ScriptComponent& comp, std::vector<QJsonObject>& deferredSpawning);

    /**
     * @brief Sets up the global variables for a given script component.
     */
    void initializeJSEntity(ScriptComponent &comp);

    /**
     * @brief Cache the code in JSHelperFuncs.js and JSMath.js in mHelperFuncs. This string is added to the top of all script files.
     */
    void initializeHelperFuncs();
    QString mHelperFuncs;

    // Cached
    ScriptComponent* currentComp;
    QString currentFileName{};
    float mDeltaTime{0};
    std::map<unsigned int, std::vector<QString>> globalVariables{};
};




/** Pointer referencing class to the global ScriptSystem instance object.
 * A pointer class designed to prevent that the "this" object doesn't own the
 * scriptsystem. Because Qt requires objects to be placed on the heap but also
 * want's to manage the objects lifetime.
 * @brief ScriptSystem instance pointer.
 */
class QScriptSystemPointer : public QObject
{
    Q_OBJECT
private:
    ScriptSystem* mPtr = nullptr;

public:
    QScriptSystemPointer()
    {
        auto ptr = ScriptSystem::get();
        assert(ptr != nullptr);
        mPtr = ptr;
    }
    QScriptSystemPointer(const QScriptSystemPointer& rhs) { mPtr = rhs.mPtr; }
    QScriptSystemPointer& operator= (const QScriptSystemPointer& rhs) { mPtr = rhs.mPtr; return *this; }

    ScriptSystem* get() { return mPtr; }

    float getDeltaTime() const { return mPtr->mDeltaTime; }
    void setDeltaTime(float time) { mPtr->mDeltaTime = time; }
    Q_PROPERTY(float deltaTime READ getDeltaTime WRITE setDeltaTime)

public slots:
    /**
     * @brief Spawns and returns a cube at (0,0,0). This includes a transform- and mesh component. Mesh is set to a cube. Defaults to visible.
     * Example: let cube = engine.spawnCube();
     */
    QObject* spawnCube(){ return mPtr->spawnCube(); }
    /**
     * @brief Spawns and returns an entity with no components.
     * Example: let spawnedEntity = engine.spawnEntity();
     */
    QObject* spawnEntity(){ return mPtr->spawnEntity(); }

    /**
     * @brief Returns an array of all entity IDs that has the given component.
     * Example: let entityIDs = engine.getAllEntityIDsByComponent("mesh");
     */
    QJSValue getAllEntityIDsByComponent(const QString& name){ return mPtr->getAllEntityIDsByComponent(name); }

    /**
     * @brief Returns an array of all entityIDs.
     * Example: let entityIDs = engine.getAllEntityIDs();
     */
    QJSValue getAllEntityIDs(){ return mPtr->getAllEntityIDs(); }

    /**
     * @brief Returns an entity with the given ID.
     * Example: let entity = engine.getEntity(2);
     */
    QObject* getEntity(unsigned int id){ return mPtr->getEntity(id); }

    /**
     * @brief Destroys the passed in entity.
     */
    void destroyEntity(unsigned entity){ return mPtr->destroyEntity(entity); }

};

#endif // SCRIPTSYSTEM_H
