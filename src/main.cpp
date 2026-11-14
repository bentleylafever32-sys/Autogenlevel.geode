#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <random>
#include <algorithm>
#include <array>

using namespace geode::prelude;

// Object IDs used by the 2.2 object set. Keep these centralized so future
// Geode/GD updates are easier to audit.
namespace ID {
	constexpr int Block = 1, Spike = 8, Slab = 40, SmallSpike = 39;
	constexpr int YellowPad = 35, YellowOrb = 36, BlueOrb = 84;
	constexpr int ShipPortal = 12, CubePortal = 13;
	constexpr int SpeedNormal = 201, SpeedFast = 202, SpeedFaster = 203;
}

static LevelEditorLayer* s_lastEditorLayer = nullptr;
static float s_nextGeneratedX = 300.f;

struct Gen {
	LevelEditorLayer* lel = nullptr;
	std::mt19937 rng;
	float x = 300.f;
	int diff = 2;
	bool decor = true;
	bool neverRepeat = true;
	int lastSegment = -1;
	int createdObjects = 0;
	int failedObjects = 0;
	bool aborted = false;
	static constexpr float U = 30.f;

	int rnd(int a, int b) {
		if (b < a) std::swap(a, b);
		return std::uniform_int_distribution<int>(a, b)(rng);
	}

	GameObject* put(int id, int gx, int gy, float rot = 0.f) {
		if (!lel || aborted) return nullptr;
		// A hard cap prevents an extreme segment setting from flooding the editor.
		if (createdObjects >= 8000) {
			aborted = true;
			return nullptr;
		}
		auto o = lel->createObject(id, ccp(x + gx * U, 15.f + gy * U), false);
		if (!o) {
			++failedObjects;
			// A single missing object should not crash the game. Abort only after
			// repeated failures, which usually means the target object set is not
			// compatible with the current GD/Geode bindings.
			if (failedObjects >= 8) aborted = true;
			return nullptr;
		}
		++createdObjects;
		if (rot != 0.f) o->setRotation(rot);
		return o;
	}

	// Difficulty ramp: the highest presets start controlled and become harder.
	void setSectionDifficulty(int section, int total, int base) {
		if (total <= 1) {
			diff = base;
			return;
		}
		float t = static_cast<float>(section) / static_cast<float>(total - 1);
		int ramp = static_cast<int>(t * 4.f);
		diff = std::clamp(base + ramp, 1, 7);
	}

	// ---- gameplay sections ----
	int spikes() {
		// Use short, readable hazard groups with recovery space between them.
		const int groups = rnd(2, std::min(4, 2 + diff / 2));
		int gx = 4;
		for (int g = 0; g < groups; ++g) {
			const int count = diff >= 6 && rnd(0, 2) == 0 ? 2 : 1;
			for (int i = 0; i < count; ++i) put(ID::Spike, gx + i, 0);
			gx += count + rnd(std::max(3, 7 - diff / 2), std::max(4, 9 - diff / 2));
		}
		return gx + 3;
	}

	int stairs() {
		int h = rnd(2, std::min(5, 2 + diff / 2));
		for (int s = 0; s < h; s++)
			for (int w = 0; w < 2; w++)
				for (int y = 0; y <= s; y++) put(ID::Block, 4 + s * 2 + w, y);
		int top = 4 + h * 2;
		for (int w = 0; w < 3; w++)
			for (int y = 0; y < h; y++) put(ID::Block, top + w, y);
		put(ID::Spike, top + 3, 0);
		if (diff >= 5) put(ID::Spike, top + 5, 0);
		return top + 8;
	}
