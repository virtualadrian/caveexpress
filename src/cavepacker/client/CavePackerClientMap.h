#pragma once

#include "engine/client/ClientMap.h"

class CavePackerClientMap: public ClientMap {
public:
	CavePackerClientMap (int x, int y, int width, int height, IFrontend *frontend, ServiceProvider& serviceProvider,
			int referenceTileWidth);

	int getWaterSurface() const override { return 0; }
	int getWaterGround() const override { return 0; }

	void start () override;
	void render (int x, int y) const override;
};
