// PhysicsWorld: Jolt-backed implementation. This is the only translation unit
// that includes Jolt, so it is the only one that needs the Jolt include path
// and C++17 (see the NucEngine.vcxproj settings scoped to this file and the
// Jolt vendor sources).

#include "JoltPhysics/PhysicsWorld.h"

// Jolt.h must be included before any other Jolt header.
#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <thread>

JPH_SUPPRESS_WARNINGS

using namespace JPH;

namespace {

// --- Trace / assert callbacks ----------------------------------------------
void TraceImpl(const char* fmt, ...)
{
	va_list list;
	va_start(list, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), fmt, list);
	va_end(list);
	fprintf(stderr, "[Jolt] %s\n", buffer);
}

#ifdef JPH_ENABLE_ASSERTS
bool AssertFailedImpl(const char* expr, const char* message, const char* file, uint line)
{
	fprintf(stderr, "[Jolt] %s:%u: (%s) %s\n", file, line, expr, message ? message : "");
	return true; // trigger a breakpoint in a debugger
}
#endif

// --- Collision layers -------------------------------------------------------
// Two object layers: static obstacles and moving bodies. A 1:1 broad-phase
// layer mapping keeps the static tree from being rebuilt every frame.
namespace Layers {
	static constexpr ObjectLayer NON_MOVING = 0;
	static constexpr ObjectLayer MOVING = 1;
	static constexpr ObjectLayer NUM_LAYERS = 2;
}

namespace BroadPhaseLayers {
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
	static constexpr uint NUM_LAYERS(2);
}

class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

	BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		return (BroadPhaseLayer::Type)inLayer == (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING
			? "NON_MOVING" : "MOVING";
	}
#endif

private:
	BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

class ObjectVsBroadPhaseLayerFilterImpl final : public ObjectVsBroadPhaseLayerFilter
{
public:
	bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1) {
		case Layers::NON_MOVING: return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:     return true;
		default:                 JPH_ASSERT(false); return false;
		}
	}
};

class ObjectLayerPairFilterImpl final : public ObjectLayerPairFilter
{
public:
	bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{
		switch (inObject1) {
		case Layers::NON_MOVING: return inObject2 == Layers::MOVING;
		case Layers::MOVING:     return true;
		default:                 JPH_ASSERT(false); return false;
		}
	}
};

// Jolt's global factory/type registration is process-wide; do it once and
// leave it installed for the process lifetime so worlds can be recreated.
bool gJoltGlobalsReady = false;

void EnsureJoltGlobals()
{
	if (gJoltGlobalsReady)
		return;

	RegisterDefaultAllocator();
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)
	Factory::sInstance = new Factory();
	RegisterTypes();
	gJoltGlobalsReady = true;
}

BodyID ToBodyID(PhysicsWorld::BodyId id) { return BodyID(id); }

} // namespace

// ---------------------------------------------------------------------------

struct PhysicsWorld::Impl
{
	// PhysicsSystem keeps references to these interfaces, so they must outlive it.
	BPLayerInterfaceImpl broadPhaseLayers;
	ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhase;
	ObjectLayerPairFilterImpl objectLayerPairFilter;

	PhysicsSystem system;
	TempAllocatorImpl* tempAllocator = nullptr;
	JobSystemThreadPool* jobSystem = nullptr;

	float stepAccumulator = 0.0f;
	bool initialized = false;
};

PhysicsWorld::PhysicsWorld() : impl(new Impl()) {}

PhysicsWorld::~PhysicsWorld() { Shutdown(); }

bool PhysicsWorld::Init()
{
	if (impl->initialized)
		return true;

	EnsureJoltGlobals();

	impl->tempAllocator = new TempAllocatorImpl(16 * 1024 * 1024);

	unsigned threads = std::thread::hardware_concurrency();
	int workerThreads = std::max(1, (int)threads - 1);
	impl->jobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, workerThreads);

	const uint cMaxBodies = 4096;
	const uint cNumBodyMutexes = 0;
	const uint cMaxBodyPairs = 4096;
	const uint cMaxContactConstraints = 4096;

	impl->system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
	                   impl->broadPhaseLayers, impl->objectVsBroadPhase, impl->objectLayerPairFilter);

	impl->initialized = true;
	return true;
}

void PhysicsWorld::Shutdown()
{
	if (!impl->initialized)
		return;

	delete impl->jobSystem;
	impl->jobSystem = nullptr;
	delete impl->tempAllocator;
	impl->tempAllocator = nullptr;
	impl->initialized = false;
	// Jolt globals (factory/types) are intentionally left registered so a new
	// world can be created without re-initializing them.
}

bool PhysicsWorld::IsInitialized() const { return impl->initialized; }

void PhysicsWorld::SetGravity(const glm::vec3& gravity)
{
	if (impl->initialized)
		impl->system.SetGravity(Vec3(gravity.x, gravity.y, gravity.z));
}

void PhysicsWorld::Step(float deltaTime)
{
	if (!impl->initialized || deltaTime <= 0.0f)
		return;

	// Fixed-timestep accumulator: Jolt is stable at 60 Hz, and the engine's
	// frame delta is variable, so we advance the sim in 1/60 s slices and carry
	// the remainder. Cap the catch-up to avoid a spiral of death after a stall.
	const float fixedStep = 1.0f / 60.0f;
	const int maxSubSteps = 6;

	impl->stepAccumulator += deltaTime;
	int steps = 0;
	while (impl->stepAccumulator >= fixedStep && steps < maxSubSteps) {
		impl->system.Update(fixedStep, 1, impl->tempAllocator, impl->jobSystem);
		impl->stepAccumulator -= fixedStep;
		++steps;
	}
	if (steps == maxSubSteps)
		impl->stepAccumulator = 0.0f; // drop the backlog
}

PhysicsWorld::BodyId PhysicsWorld::CreateBox(const glm::vec3& position, const glm::vec3& halfExtents, Motion motion)
{
	if (!impl->initialized)
		return InvalidBody;

	BoxShapeSettings shapeSettings(Vec3(halfExtents.x, halfExtents.y, halfExtents.z));
	shapeSettings.SetEmbedded();
	ShapeSettings::ShapeResult result = shapeSettings.Create();
	if (result.HasError())
		return InvalidBody;

	const bool dynamic = motion == Motion::Dynamic;
	BodyCreationSettings settings(result.Get(), RVec3(position.x, position.y, position.z),
	                              Quat::sIdentity(),
	                              dynamic ? EMotionType::Dynamic : EMotionType::Static,
	                              dynamic ? Layers::MOVING : Layers::NON_MOVING);

	BodyID id = impl->system.GetBodyInterface().CreateAndAddBody(
		settings, dynamic ? EActivation::Activate : EActivation::DontActivate);
	if (id.IsInvalid())
		return InvalidBody;
	return id.GetIndexAndSequenceNumber();
}

PhysicsWorld::BodyId PhysicsWorld::CreateSphere(const glm::vec3& position, float radius, Motion motion)
{
	if (!impl->initialized)
		return InvalidBody;

	const bool dynamic = motion == Motion::Dynamic;
	BodyCreationSettings settings(new SphereShape(radius), RVec3(position.x, position.y, position.z),
	                              Quat::sIdentity(),
	                              dynamic ? EMotionType::Dynamic : EMotionType::Static,
	                              dynamic ? Layers::MOVING : Layers::NON_MOVING);

	BodyID id = impl->system.GetBodyInterface().CreateAndAddBody(
		settings, dynamic ? EActivation::Activate : EActivation::DontActivate);
	if (id.IsInvalid())
		return InvalidBody;
	return id.GetIndexAndSequenceNumber();
}

void PhysicsWorld::RemoveBody(BodyId id)
{
	if (!impl->initialized || id == InvalidBody)
		return;
	BodyInterface& bodyInterface = impl->system.GetBodyInterface();
	BodyID bodyId = ToBodyID(id);
	bodyInterface.RemoveBody(bodyId);
	bodyInterface.DestroyBody(bodyId);
}

glm::vec3 PhysicsWorld::GetPosition(BodyId id) const
{
	if (!impl->initialized || id == InvalidBody)
		return glm::vec3(0.0f);
	RVec3 p = impl->system.GetBodyInterface().GetPosition(ToBodyID(id));
	return glm::vec3((float)p.GetX(), (float)p.GetY(), (float)p.GetZ());
}

glm::quat PhysicsWorld::GetRotation(BodyId id) const
{
	if (!impl->initialized || id == InvalidBody)
		return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	Quat q = impl->system.GetBodyInterface().GetRotation(ToBodyID(id));
	return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
}
