#include <stdlib.h>
#include "client/entities/ClientEntityFactory.h"
#include "caveexpress/shared/CaveExpressEntityType.h"
#include "common/Singleton.h"
#include "common/String.h"
#include "sound/Sound.h"
#include "common/FileSystem.h"
#include "common/Log.h"

using namespace caveexpress;
static SoundMappingCache soundMappingCache;

static inline bool exists (const std::string& sound)
{
	return FS.exists(FS.getSoundsDir() + sound + ".ogg");
}

static bool checkSound (const EntityType* type, const std::string& prefix, const Animation* animation)
{
	if (exists(prefix + animation->name)) {
		const std::string sound = prefix + animation->name;
		soundMappingCache[type][animation] = sound;
		Log::info2(LOG_CLIENT, "use sound %s for animation %s", sound.c_str(), animation->name.c_str());
		return true;
	} else if (animation->hasDirection()) {
		const std::string& sound = prefix + animation->getNameWithoutDirection();
		if (exists(sound)) {
			soundMappingCache[type][animation] = sound;
			Log::info2(LOG_CLIENT, "use sound %s for animation %s", sound.c_str(), animation->name.c_str());
			return true;
		}
	} else if (exists(type->name)) {
		soundMappingCache[type][animation] = type->name;
		Log::info2(LOG_CLIENT, "use sound %s", type->name.c_str());
	}

	// cut of the 'multiple-types-part' - e.g. "-01", "-02" - this is not about the frames!
	if (prefix.length() > 3 && prefix[prefix.length() - 3] == '-') {
		return checkSound(type, prefix.substr(0, prefix.length() - 3), animation);
	}
	return false;
}

#define CHECKSOUND(type, prefix, animation) if (checkSound(type, prefix, animation)) continue;

static void fillSounds ()
{
	for (EntityType::TypeMapConstIter eIter = EntityType::begin(); eIter != EntityType::end(); ++eIter) {
		const EntityType* type = eIter->second;
		const std::string& typeName = type->name;
		for (Animation::TypeMapConstIter i = Animation::begin(); i != Animation::end(); ++i) {
			const Animation* animation = i->second;
			CHECKSOUND(type, typeName + "-", animation)
			if (type->hasTheme()) {
				CHECKSOUND(type, type->getNameWithoutTheme() + "-", animation)
			}
			if (EntityTypes::isNpc(*type)) {
				if (EntityTypes::isNpcCave(*type)) {
					CHECKSOUND(type, "npc-cave-", animation)
				} else if (EntityTypes::isNpcAttacking(*type)) {
					CHECKSOUND(type, "npc-attacking-", animation)
				} else if (EntityTypes::isNpcAggressive(*type)) {
					CHECKSOUND(type, "npc-aggressive-", animation)
				}
				CHECKSOUND(type, "npc-", animation)
			} else if (EntityTypes::isCollectable(*type)) {
				CHECKSOUND(type, "collectable-", animation)
			}
		}
	}
}

extern "C" int main(int argc, char* argv[]) {
	fillSounds();

	std::stringstream s;
	for (EntityType::TypeMapConstIter eIter = EntityType::begin(); eIter != EntityType::end(); ++eIter) {
		SoundMappingCacheConstIter mIter = soundMappingCache.find(eIter->second);
		if (mIter == soundMappingCache.end())
			continue;
		if (!s.str().empty())
			s << "\n";
		String name = eIter->second->name;
		name = name.replaceAll("-", "");
		s << name << " = {\n";
		const SoundMapping& soundMapping = mIter->second;
		for (Animation::TypeMapConstIter aIter = Animation::begin(); aIter != Animation::end(); ++aIter) {
			SoundMappingConstIter sIter = soundMapping.find(aIter->second);
			if (sIter == soundMapping.end())
				continue;
			name = aIter->second->name;
			name = name.replaceAll("-", "");
			s << "\t" << name << " = \"" << sIter->second << "\",\n";
		}
		s << "}\n";
	}
	const std::string lua = s.str();
	if (FS.writeSysFile("entitysounds.lua", (const unsigned char*)lua.c_str(), lua.length(), true) == -1) {
		Log::error2(LOG_CLIENT, "failed to write the file");
	}

	return EXIT_SUCCESS;
}
