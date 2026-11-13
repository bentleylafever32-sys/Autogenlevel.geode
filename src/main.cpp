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
		int n = rnd(1, std::min(4, 1 + diff / 2 + 1));
		int spacing = std::max(2, 6 - diff / 2);
		for (int i = 0; i < n; i++) put(ID::Spike, 4 + i * spacing, 0);
		if (diff >= 4 && rnd(0, 1)) put(ID::Spike, 4 + n * spacing, 0);
		return 10 + n * spacing;
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

	int platformPit() {
		int gap = std::clamp(3 + diff / 2, 3, 6);
		int gx = 3;
		for (int p = 0; p < 3; p++) {
			int w = rnd(2, 4);
			for (int i = 0; i < w; i++) put(ID::Block, gx + i, 0);
			for (int i = 0; i < w; i++) if (diff >= 4) put(ID::Block, gx + i, 1);
			gx += w;
			for (int i = 0; i < gap; i++) put(ID::Spike, gx + i, 0);
			gx += gap;
		}
		return gx + 3;
	}

	int orbPit() {
		int len = 5 + diff;
		for (int i = 0; i < len; i++) {
			if (i % std::max(2, 5 - diff / 2) == 0) put(ID::Spike, 4 + i, 0);
		}
		put(ID::YellowOrb, 5 + len / 2, 2);
		if (diff >= 3) put(ID::BlueOrb, 5 + len, 3);
		if (diff >= 5) put(ID::YellowOrb, 8, 4);
		return len + 10;
	}

	int padHop() {
		put(ID::YellowPad, 4, 0);
		int platform = 7 + diff / 2;
		for (int i = 0; i < 4; i++) put(ID::Block, platform + i, 3 + (i % 2));
		for (int i = 0; i < 5 + diff / 2; i++) put(ID::Spike, 6 + i, 0);
		if (diff >= 5) put(ID::YellowOrb, platform + 1, 5);
		return platform + 8;
	}

	int ship() {
		int len = 28 + diff * 4;
		put(ID::ShipPortal, 2, 4);
		for (int i = 4; i < len; i++) {
			put(ID::Block, i, 9);
			put(ID::Block, i, 10);
			put(ID::Block, i, 0);
		}
		int obstacleStep = std::max(4, 8 - diff / 2);
		for (int i = 8; i < len - 4; i += obstacleStep) {
			bool fromFloor = rnd(0, 1);
			int h = rnd(1, std::max(2, 8 - diff));
			if (fromFloor) {
				for (int y = 1; y <= h; y++) put(ID::Block, i, y);
			} else {
				for (int y = 8; y >= 9 - h; y--) put(ID::Block, i, y);
			}
		}
		put(ID::CubePortal, len + 1, 4);
		return len + 4;
	}

	int precision() {
		int len = 18 + diff * 2;
		int step = std::max(2, 4 - diff / 2);
		for (int i = 0; i < len; ++i) {
			if (i % step == 0) put(ID::Spike, 4 + i, 0);
			if (i % (step + 1) == 1) put(ID::Spike, 4 + i, 2, 180.f);
		}
		for (int i = 0; i < 4; ++i) put(ID::Block, 8 + i, 1);
		put(ID::YellowOrb, 10, 3);
		put(ID::BlueOrb, 16 + diff / 2, 2);
		if (diff >= 6) put(ID::YellowOrb, 20, 4);
		return len + 8;
	}

	int topOne() {
		int len = 30 + diff * 3;
		put(ID::SpeedFaster, 2, 2);
		for (int i = 0; i < len; ++i) {
			int gx = 4 + i;
			if (i % 2 == 0) put(ID::Spike, gx, 0);
			if (i % 3 == 0) put(ID::Spike, gx + 1, 2, 180.f);
			if (i % 5 == 2) put(ID::Block, gx, 1);
			if (diff >= 6 && i % 7 == 4) put(ID::Block, gx, 3);
		}
		for (int i = 0; i < 3; ++i) put(ID::Block, 12 + i, 3);
		put(ID::YellowOrb, 13, 5);
		put(ID::BlueOrb, 22, 3);
		put(ID::YellowPad, 28, 0);
		return len + 8;
	}

	// A transition is intentionally short: it resets visual rhythm and gives
	// the generator a clean place to change gameplay ideas.
	int transition() {
		put(rnd(0, 1) ? ID::SpeedFast : ID::SpeedFaster, 2, 2);
		for (int i = 0; i < 4; ++i) put(ID::Block, 5 + i, 0);
		put(ID::Spike, 10, 0);
		put(ID::YellowOrb, 12, 3);
		return 17;
	}

	// A chokepoint section is short and dense rather than simply being a long
	// field of hazards. It creates memorable "must hit this" moments.
	int chokepoint() {
		int len = 16 + diff;
		for (int i = 0; i < len; ++i) {
			if (i % 3 != 1) put(ID::Spike, 4 + i, 0);
			if (i % 4 == 0) put(ID::Spike, 5 + i, 2, 180.f);
		}
		put(ID::Block, 8, 1);
		put(ID::Block, 12, 1);
		put(ID::YellowOrb, 14, 4);
		return len + 7;
	}

	// Deliberately impassable section for the Impossible preset. This is a
	// creative/editor generator, not a verifier: the wall is intended to block
	// normal gameplay, and users should playtest the result on their target build.
	int impossible() {
		int wallX = 7 + rnd(0, 3);
		int wallWidth = rnd(3, 5);
		for (int gx = wallX; gx < wallX + wallWidth; ++gx) {
			for (int gy = -2; gy <= 16; ++gy) put(ID::Block, gx, gy);
		}
		for (int i = 0; i < 12; ++i) {
			put(ID::Spike, 3 + i, 0);
			if (i % 2 == 0) put(ID::Spike, 4 + i, 2, 180.f);
		}
		put(ID::YellowOrb, wallX + 1, 5);
		return wallX + wallWidth + 8;
	}

	int speedGate() {
		put(rnd(0, 1) ? ID::SpeedFast : ID::SpeedFaster, 2, 2);
		return spikes() + 2;
	}

	// ---- structured preset logic ----
	int buildSection(int type) {
		switch (type) {
			case 0: return spikes();
			case 1: return stairs();
			case 2: return platformPit();
			case 3: return orbPit();
			case 4: return padHop();
			case 5: return ship();
			case 6: return precision();
			case 7: return topOne();
			case 8: return speedGate();
			case 9: return transition();
			case 10: return chokepoint();
			case 11: return impossible();
			default: return chokepoint();
		}
	}

	int pickType(int preset, int section) {
		// Impossible: every segment contains a deliberately impassable wall.
		if (preset == 4) return 11;

		// Regular: broad variety.
		if (preset == 0) return rnd(0, 4);

		// Hard: precision appears often but isn't allowed to dominate.
		if (preset == 1) {
			static constexpr std::array<int, 8> pool = {6, 0, 1, 3, 4, 5, 8, 10};
			return pool[rnd(0, static_cast<int>(pool.size()) - 1)];
		}

		// Extreme: alternate recognizable section types.
		if (preset == 2) {
			static constexpr std::array<int, 8> pool = {6, 5, 7, 3, 8, 10, 1, 9};
			return pool[(section + rnd(0, 2)) % pool.size()];
		}

		// Top 1 Style macro: intro -> precision -> chokepoint -> mode change ->
		// speed -> ship -> climax. The exact patterns remain seeded/randomized.
		static constexpr std::array<int, 10> macro = {9, 6, 10, 7, 8, 5, 6, 10, 7, 7};
		return macro[section % macro.size()];
	}

	void decorate(int width, int preset) {
		if (!decor) return;
		int density = preset >= 3 ? 4 : preset >= 2 ? 5 : 6;
		for (int i = 0; i < width; i += rnd(density, density + 3)) {
			int h = rnd(1, preset >= 3 ? 4 : 3);
			for (int y = 0; y < h; y++) put(ID::Block, i, 13 + y);
			put(ID::Spike, i, 12, 180.f);
			if (rnd(0, 2) == 0) put(ID::Slab, i, 16);
		}
		if (preset < 3) {
			for (int i = 1; i < width; i += rnd(3, 5)) put(ID::SmallSpike, i, -1);
		}
	}

	bool run(int segments, int preset) {
		if (!lel || segments < 1) return false;
		put(ID::CubePortal, -2, 2);
		const int requestedDiff = diff;

		for (int s = 0; s < segments && !aborted; s++) {
			if (preset >= 2 && preset != 4) setSectionDifficulty(s, segments, requestedDiff);
			else diff = requestedDiff;

			int type = pickType(preset, s);
			if (neverRepeat && preset >= 1 && type == lastSegment) {
				for (int tries = 0; tries < 6 && type == lastSegment; ++tries)
					type = pickType(preset, s + tries + 1);
			}
			lastSegment = type;

			int w = buildSection(type);
			decorate(w, preset);

			// Tighten spacing as difficulty rises. High presets get deliberate
			// transitions so the level has readable macro-structure.
			int buffer = preset >= 3 ? (diff >= 6 ? 0 : 1) : (preset >= 2 ? 1 : 2);
			x += (w + buffer) * U;

			if (preset >= 3 && (s % 4 == 3 || type == 9)) put(ID::SpeedNormal, 0, 2);
			else if (preset < 3 && s % 5 == 4) put(ID::SpeedNormal, 0, 2);
		}
		return !aborted && createdObjects > 0;
	}
};

class $modify(AutoGenEditorUI, EditorUI) {
	bool init(LevelEditorLayer* lel) {
		if (!EditorUI::init(lel)) return false;
		auto menu = CCMenu::create();
		if (!menu) return true;
		menu->setID("nextbot.autolevelgen/menu");
		auto btn = CCMenuItemSpriteExtra::create(
			ButtonSprite::create("AI Gen"), this, menu_selector(AutoGenEditorUI::onAIGen));
		if (!btn) return true;
		btn->setID("nextbot.autolevelgen/button");
		menu->addChild(btn);
	auto win = CCDirector::get()->getWinSize();
menu->setPosition(ccp(win.width * 0.32f, win.height - 120.f));
menu->setScale(0.85f);
		this->addChild(menu);
		return true;
	}

	void generatePreset(int preset) {
		auto mod = Mod::get();
		int seed = (int)mod->getSettingValue<int64_t>("seed");
		Gen g;
		g.lel = m_editorLayer;
		g.rng.seed(seed ? seed : std::random_device{}());
		g.diff = (int)mod->getSettingValue<int64_t>("difficulty");
		g.decor = mod->getSettingValue<bool>("decor");
		g.neverRepeat = mod->getSettingValue<bool>("never_repeat");

		int segments = (int)mod->getSettingValue<int64_t>("segments");
		if (preset == 1) {
			g.diff = std::max(g.diff, 4);
			segments = std::max(segments, 14);
		} else if (preset == 2) {
			g.diff = 5;
			segments = std::max(segments, 18);
		} else if (preset == 3) {
			g.diff = 5;
			segments = std::max(segments, 24);
		} else if (preset == 4) {
			g.diff = 7;
			segments = std::max(segments, 24);
		}

		if (!g.run(segments, preset)) {
			FLAlertLayer::create(
				"AI Level Generator",
				"Generation stopped safely because the current object set did not accept enough objects. No crash was triggered. Check the Geode/GD compatibility target before trying again.",
				"OK"
			)->show();
			return;
		}

		const char* name = preset == 0 ? "Regular" :
			preset == 1 ? "Hard" : preset == 2 ? "Extreme" :
			preset == 3 ? "Top 1 Style" : "Impossible";
		FLAlertLayer::create(
			"AI Level Generator",
			fmt::format("{} layout generated: {} objects. Playtest every section before publishing; Top 1 Style is a difficulty target, not a verification claim.", name, g.createdObjects).c_str(),
			"OK"
		)->show();
	}

	void onAIGen(CCObject*) {
		createQuickPopup(
			"AI Level Gen",
			"Choose a generation style. Regular = broad variety. More = advanced challenge presets.",
			"Regular", "More",
			[this](auto, bool more) {
				if (!more) {
					generatePreset(0);
					return;
				}
				createQuickPopup(
					"AI Challenge Presets",
					"Hard = tight mixed gameplay. More = Extreme, Top 1 Style, and Impossible.",
					"Hard", "More",
					[this](auto, bool more2) {
						if (!more2) {
							generatePreset(1);
							return;
						}
						createQuickPopup(
							"Extreme Presets",
							"Extreme = sustained difficulty. Top 1 Style = escalating precision. Impossible = deliberately blocked sections.",
							"Extreme", "Top 1",
							[this](auto, bool topOne) {
								if (topOne) {
									createQuickPopup(
										"Top 1 / Impossible",
										"Choose Top 1-style playable-target patterns or deliberately impossible wall sections.",
										"Top 1 Style", "Impossible",
										[this](auto, bool impossibleMode) { generatePreset(impossibleMode ? 4 : 3); }
									);
								} else generatePreset(2);
							}
						);
					}
				);
			}
		);
	}
};
