#pragma once
#include <vector>
#include <mutex>
#include <ofEvents.h>
#include <ofTexture.h>
#include <ofShader.h>
#include <ofFbo.h>
#include <of3dPrimitives.h>
#include <ofParameter.h>
#include "../SoundPlayerView.h"
#include "../RingBuffer.h"
#include "../DSP/Context.h"
#include "../DSP/EnvelopeFilter.h"

//https://forum.openframeworks.cc/t/ofxdatgui-a-new-user-interface-for-of/20553/21
template<typename T>
struct SynchedParameter
{
	ofParameter<T> parameter;
	T value;

	void update()
	{
		if (value != parameter)
		{
			parameter.set(parameter.getName(), value);
		}
	}
};

class AVSoundPlayerView : public SoundPlayerView
{
	friend class SoundPlayer;
	
	struct Cursor
	{
		float time, progress;
		int position;

		Cursor() 
		{ 
			clear(); 
		}
		void clear()
		{
			time = progress = 0;
			position = 0;
		}
	};

	using BaseSampleType = float;

	ofPlanePrimitive				m_Quad, m_CursorRef;
	ofStyle							m_FGStyle;
	ofRectangle						m_BaseTileRect;
	ofShader						m_EncoderProg, m_DecoderProg;
	ofFbo							m_RenderFbo;
	GLint							m_EncoderProgStatus, m_DecoderProgStatus;
	GLint							m_TexInternalFormat;
	std::vector<ofTexture>			m_TexTileArray;
	std::vector<ofTexture>			m_TexDataArray;
	SoundPlayer*					m_Player;
	Cursor							m_TileReadCursor, m_TileWriteCursor;
	bool							m_Initialized;
	int								m_TilesOnScreen, m_TilesOffScreen, m_TilesTotal;
	//float							m_TilePeriod;
	glm::ivec2						m_TileTextureSize;
	std::vector<BaseSampleType>		m_SoundSwap;
	std::vector<BaseSampleType>		m_SoundZero;
	RingBufferT<BaseSampleType>		m_SoundFifo;
	std::mutex						m_Mutex;

	//DSP Stuff
	DSP::EnvelopeFilter*			m_DSPEnvelopeFilter;
	std::unique_ptr<DSP::Context>	m_DSPContext;

	// Callback methods
	void onPlayerFill(SoundFrameEvent &e);
	void onPlayerSeek();
	void onPlayerLoad();
	void onPlayerUnload();
	void onPlayerFinish();
	void update() override;
	void draw() override;
	void exit() override;
	void viewResized() override;
	// Methods
	void drawTile(ofRectangle, int);
	void drawTilesBG();
	void drawTilesFG();
	void buildTiles();
	void clearTiles();
	void registerEvents();
	void unregisterEvents();

	ofTexture& texTileAt(int i);
	ofTexture& texTileAtWritePos(int offset = 0);
	ofTexture& texDataAt(int i);
	ofTexture& texDataAtWritePos(int offset = 0);
	int getWritePosition(int offset = 0);
	int getReadPosition(int offset = 0);

	
	size_t getTileWriteAvailable() 	
	{
		return m_SoundFifo.getAvailableWrite() / getNumSamplesPerTile();
	}
	size_t getTileReadAvailable() 	
	{
		return m_SoundFifo.getAvailableRead() / getNumSamplesPerTile();
	}

public:
	AVSoundPlayerView(SoundPlayer*);
	virtual ~AVSoundPlayerView();
	
	void		setup();
	void		reload();
	int			getSampleSize();
	glm::ivec2	getTileSize();
	int			getNumTiles();
	int			getNumSamples();
	int			getNumSamplesPerTile();
	int			getNumChannels();
	size_t		getBufferSize();
	size_t		getBufferReadAvailable();
	size_t		getBufferFillAvailable();
};
