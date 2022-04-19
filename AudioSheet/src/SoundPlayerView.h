#pragma once
#include <glm/glm.hpp>
#include "SoundPlayer.h"
#include "View.h"

class SoundPlayerView : public View
{
public:
	virtual				~SoundPlayerView() = default;
	virtual	void		setup() = 0;
	virtual void		reload() = 0;
	virtual int			getSampleSize() = 0;
	virtual glm::ivec2	getTileSize() = 0;
	virtual int			getNumTiles() = 0;
	virtual int			getNumSamples() = 0;
	virtual int			getNumSamplesPerTile() = 0;
	virtual int			getNumChannels() = 0;

	virtual size_t		getBufferSize() = 0;
	virtual size_t		getBufferReadAvailable() = 0;
	virtual size_t		getBufferFillAvailable() = 0;
};