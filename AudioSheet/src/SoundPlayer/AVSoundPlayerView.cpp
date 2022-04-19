#include "AVSoundPlayerView.h"
#include "AVSoundPlayer.h"
#include <ofGraphics.h> // draw methods
#include <ofEvents.h> // ofAddListener
#include <ofAppRunner.h>
#include <ofAppBaseWindow.h>
#include "../DSP/CContext.h"

GLint getGLSizedInternalFormatFromAVFormat(AVSampleFormat sampleFormat, int channels)
{
	switch (av_get_packed_sample_fmt(sampleFormat))
	{
		case AV_SAMPLE_FMT_U8:
			switch (channels)
			{
				case 1:	return GL_R8UI;
				case 2:	return GL_RG8UI;
				case 3:	return GL_RGB8UI;
				case 4:	return GL_RGBA8UI;
			}
		case AV_SAMPLE_FMT_S16:
			switch (channels)
			{
				case 1:	return GL_R16I;
				case 2:	return GL_RG16I;
				case 3:	return GL_RGB16I;
				case 4:	return GL_RGBA16I;
			}
		case AV_SAMPLE_FMT_S32:
			switch (channels)
			{
				case 1:	return GL_R32I;
				case 2:	return GL_RG32I;
				case 3:	return GL_RGB32I;
				case 4:	return GL_RGBA32I;
			}
		case AV_SAMPLE_FMT_FLT:
			switch (channels)
			{
				case 1:	return GL_R32F;
				case 2:	return GL_RG32F;
				case 3:	return GL_RGB32F;
				case 4:	return GL_RGBA32F;
			}
	}
	return GL_NONE;
}
//https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
GLint getGLBaseInternalFormat(GLint internalFormat)
{
	switch (internalFormat)
	{
		case GL_R8I:
		case GL_R8UI:
		case GL_R16I:
		case GL_R16UI:
		case GL_R16F:
		case GL_R32I:
		case GL_R32UI:
		case GL_R32F:
			return GL_R;
		case GL_RG8I:
		case GL_RG8UI:
		case GL_RG16I:
		case GL_RG16UI:
		case GL_RG16F:
		case GL_RG32I:
		case GL_RG32UI:
		case GL_RG32F:
			return GL_RG;
		case GL_RGB8I:
		case GL_RGB8UI:
		case GL_RGB16I:
		case GL_RGB16UI:
		case GL_RGB16F:
		case GL_RGB32I:
		case GL_RGB32F:
			return GL_RGB;
		case GL_RGBA8I:
		case GL_RGBA8UI:
		case GL_RGBA16I:
		case GL_RGBA16UI:
		case GL_RGBA16F:
		case GL_RGBA32I:
		case GL_RGBA32F:
			return GL_RGBA;
	}
	return GL_NONE;
}

void AVSoundPlayerView::onPlayerFill(SoundFrameEvent &e)
{
	m_SoundFifo.write(e.data, e.size);
}

void AVSoundPlayerView::onPlayerSeek()
{
	//Deverá ocorrer limpeza e preenchimento em seguida (fill)
	update();
}

void AVSoundPlayerView::onPlayerLoad()
{
	buildTiles();
}

void AVSoundPlayerView::onPlayerUnload()
{
	clearTiles();
}

void AVSoundPlayerView::onPlayerFinish()
{
	std::lock_guard<std::mutex> lock(m_Mutex);

	// Fills the remaning space with empty samples
	// so that the next tile is written
	if (m_SoundFifo.getAvailableRead() < m_SoundZero.size())
	{
		int targetSize = m_SoundZero.size() - m_SoundFifo.getAvailableRead();
		m_SoundFifo.write(m_SoundZero.data(), targetSize);
	}

	// Fills another empty tile (fix diplaying tile with past data)
	//m_SoundFifo.write(m_SoundZero.data(), m_SoundZero.size());
}

void AVSoundPlayerView::update()
{	
	if (m_Player->isLoaded() && m_Mutex.try_lock())
	{
		auto texSize = m_TileTextureSize;
		auto texFormat = m_TexInternalFormat;
		auto elapsedTime = ofGetElapsedTimef();

		while (m_SoundFifo.read(m_SoundSwap.data(), m_SoundSwap.size()))
		{
			elapsedTime = ofGetElapsedTimef();

			int k = getWritePosition();

			// Uploads sound data
			texTileAt(k).loadData(m_SoundSwap.data(), texSize.x, texSize.y, texFormat);
			// Reusing sound data to generate envelope data
			m_DSPContext->pull(m_SoundSwap.data(), m_SoundSwap.size());
			// Uploads envelope data
			texDataAt(k).loadData(m_SoundSwap.data(), texSize.x, texSize.y, texFormat);

			m_TileWriteCursor.position++;
			m_TileWriteCursor.time = elapsedTime;

			if (!m_Player->isPlaying())
			{
				m_Player->play();
			}
		}
		{
			float y, x = static_cast<float>(m_Player->getNumSamplesPlayed()) / getNumSamplesPerTile();
			m_TileReadCursor.progress = modff(x, &y);
			m_TileReadCursor.position = static_cast<int>(y);
		}

		m_Mutex.unlock();
	}
}

void AVSoundPlayerView::draw()
{
	/*
	ofSetupScreen(); // Required for ofPlanePrimitive

	ofPushStyle();
	{
		ofDisableAlphaBlending();
		ofSetColor(ofColor::slateGray);
		ofDrawRectangle(m_WindowRect);
	}
	ofPopStyle();

	if (m_Player->isLoaded())
	{
		ofPushMatrix();
		{
			ofPushStyle();
			{
				ofSetColor(ofColor::white);
					
				//ofRotateZDeg(0.0f);
				//ofTranslate(-glm::vec2(glm::ivec2(0,m_TileDisplaySize.y) * glm::ivec2(1, m_TilesOffScreen)));

				// Orientation to stay at horizontal
				ofRotateZDeg(-90.0f);
				// Position at top-right corner(x), hides offscreen tiles(y)
				ofTranslate(-glm::vec2(m_TileDisplaySize * glm::ivec2(1, m_TilesOffScreen)));
				// Offset for tiles (floor fixes moiré)
				ofTranslate(glm::floor(glm::vec2(0, ofLerp(m_TileDisplaySize.y, 0, m_TileProgress))));

				drawTilesBG();
				drawTilesFG();
			}
			ofPopStyle();
		}
		ofPopMatrix();
	}
	*/

	static const glm::vec2 fontOffset(0, 10);

	if (m_Player->isLoaded())
	{
		ofRectangle displayRect;

		int k = getReadPosition(0);

		if (m_RenderFbo.isAllocated())
		{
			displayRect.set(glm::vec2(), m_RenderFbo.getWidth(), m_RenderFbo.getHeight());

			m_RenderFbo.begin();
			{
				ofClear(0);
				ofSetColor(ofColor::white);
				drawTile(displayRect, k);
			}
			m_RenderFbo.end();

			displayRect = ofGetWindowRect();

			m_RenderFbo.draw(displayRect);
		}
		else // fallback
		{
			displayRect = ofGetWindowRect();
			// Required for drawing ofPlanePrimitive correctly
			ofSetupScreen();

			ofPushMatrix();
			{
				ofPushStyle();
				{
					ofSetColor(ofColor::white);
					drawTile(displayRect, k);
				}
				ofPopStyle();
			}
			ofPopMatrix();
		}

		ofPopStyle();
	}
}

void AVSoundPlayerView::exit()
{
	unregisterEvents();
}

void AVSoundPlayerView::viewResized()
{
	//buildTiles(); // não fazer por enquanto
}

void AVSoundPlayerView::drawTile(ofRectangle displayRect, int index)
{
	auto displaySize = glm::vec2(displayRect.getWidth(), displayRect.getHeight());

	if (m_EncoderProgStatus == GL_TRUE)
	{	
		m_EncoderProg.begin();
		
		m_EncoderProg.setUniform1f(
			"app.time", ofGetElapsedTimef());
		m_EncoderProg.setUniform1i(
			"app.frame", ofGetFrameNum());
		m_EncoderProg.setUniform2f(
			"app.mouse", ofGetMouseX(), ofGetMouseY());
		m_EncoderProg.setUniform2f(
			"app.windowSize", ofGetWindowSize());

		m_EncoderProg.setUniform1i(
			"tile.index", index);
		m_EncoderProg.setUniform1i(
			"tile.count", getNumTiles());
		m_EncoderProg.setUniform1i(
			"tile.channels", getNumChannels());
		m_EncoderProg.setUniform2f(
			"tile.textureSize", m_TileTextureSize);
		m_EncoderProg.setUniform2f(
			"tile.displaySize", displaySize);
		m_EncoderProg.setUniform1f(
			"tile.progress", m_TileReadCursor.progress);

		m_EncoderProg.setUniformTexture(
			"channel[0].tex", texTileAt(index), 0);
		m_EncoderProg.setUniform2f(
			"channel[0].siz", m_TileTextureSize);
		m_EncoderProg.setUniformTexture(
			"channel[1].tex", texDataAt(index), 1);
		m_EncoderProg.setUniform2f(
			"channel[1].siz", m_TileTextureSize);

		m_Quad.set(displaySize.x, displaySize.y);
		m_Quad.setPosition(glm::vec3(displaySize / 2, 0));
		m_Quad.mapTexCoords(1, 1, 0, 0);
		m_Quad.draw();

		m_EncoderProg.end();
	}
	else // fallback
	{
		texTileAtWritePos(index).draw(displayRect);
	}
}

void AVSoundPlayerView::drawTilesBG()
{
	size_t i;
	int k;
	
	if (m_EncoderProgStatus == GL_TRUE)
	{
		m_EncoderProg.begin();

		for (i = 0; i < m_TexTileArray.size(); i++)
		{
			k = getWritePosition(i);

			if (k >= m_TileWriteCursor.position) continue;

			ofPushMatrix();
			{
				ofTranslate(m_BaseTileRect.getBottomRight() * glm::vec2(0, i));

				m_EncoderProg.setUniform1f(
					"app.time", ofGetElapsedTimef());
				m_EncoderProg.setUniform1i(
					"app.frame", ofGetFrameNum());
				m_EncoderProg.setUniform2f(
					"app.mouse", ofGetMouseX(), ofGetMouseY());
				m_EncoderProg.setUniform2f(
					"app.windowSize", ofGetWindowSize());

				m_EncoderProg.setUniform1i(
					"tile.id", k);
				m_EncoderProg.setUniform1i(
					"tile.count", getNumTiles());
				m_EncoderProg.setUniform1i(
					"tile.channels", getNumChannels());
				m_EncoderProg.setUniform2f(
					"tile.textureSize", m_TileTextureSize);
				m_EncoderProg.setUniform2f(
					"tile.displaySize", getSize());
				m_EncoderProg.setUniform1f(
					"tile.progress", m_TileReadCursor.progress);
				
				m_EncoderProg.setUniformTexture(
					"channel[0].tex", texTileAt(k), 0);
				m_EncoderProg.setUniform2f(
					"channel[0].siz", m_TileTextureSize);
				m_EncoderProg.setUniformTexture(
					"channel[1].tex", texDataAt(k), 1);
				m_EncoderProg.setUniform2f(
					"channel[1].siz", m_TileTextureSize);

				m_Quad.draw();
			}
			ofPopMatrix();
		}

		m_EncoderProg.end();
	}
	else // fallback
	{
		for (i = 0; i < m_TexTileArray.size(); i++)
		{
			ofPushMatrix();
			{
				ofTranslate(m_BaseTileRect.getBottomRight() * glm::vec2(0, i));
				texTileAtWritePos(i).draw(m_BaseTileRect);
			}
			ofPopMatrix();
		}
	}
}

void AVSoundPlayerView::drawTilesFG() 
{
	static const glm::vec2 fontOffset(0, 10);

	size_t i;
	int k;

	ofPushStyle();
	{
		ofSetStyle(m_FGStyle);
		for (i = 0; i < m_TexTileArray.size(); i++)
		{
			k = getWritePosition(i);

			if (k >= m_TileWriteCursor.position) continue;

			ofPushMatrix();
			{
				ofTranslate(m_BaseTileRect.getBottomRight() * glm::vec2(0, i));
				ofDrawBitmapString(std::to_string(k), fontOffset);
				//ofDrawRectangle(m_BaseTileRect);
				ofDrawLine(m_BaseTileRect.getTopLeft(), m_BaseTileRect.getTopRight());
			}
			ofPopMatrix();
		}
	}
	ofPopStyle();
}

void AVSoundPlayerView::buildTiles()
{
	//std::lock_guard<std::mutex> lock(m_Mutex); // causando quebra?
	
	auto viewSize		= getSize();
	auto tileSizeBase	= glm::ivec2(512);
	auto tileSizeDown	= glm::ivec2(1);
	
	m_TilesOnScreen		= int(ceil((viewSize.x / tileSizeBase.y) * tileSizeDown.y));
	m_TilesOffScreen	= 1;
	m_TileTextureSize	= tileSizeBase / tileSizeDown;
	//m_TilePeriod		= static_cast<float>(getNumSamplesPerTile()) / m_Player->getSampleRate();
	
	m_TileWriteCursor.clear();
	m_TileReadCursor.clear();

	auto bufferPadd = 1; // times
	auto bufferSize = getNumSamplesPerTile() * m_Player->getNumChannels();
	m_SoundSwap.resize(bufferSize);
	m_SoundZero.resize(bufferSize);
	m_SoundFifo.resize(bufferSize * (1 + bufferPadd));

	auto sizedInternalFormat = getGLSizedInternalFormatFromAVFormat(
		m_Player->getSampleFormat(), m_Player->getNumChannels());

	m_TexInternalFormat = getGLBaseInternalFormat(sizedInternalFormat);

	auto initTexArray = [&](std::vector<ofTexture> &arr)
	{
		arr.resize(getNumTiles());

		for (auto &tex : arr)
		{
			if (tex.isAllocated())
				tex.loadData(m_SoundZero.data(), m_TileTextureSize.x, m_TileTextureSize.y, m_TexInternalFormat);
			else
				tex.allocate(m_TileTextureSize.x, m_TileTextureSize.y, sizedInternalFormat, false);
		}
	};

	initTexArray(m_TexTileArray);
	initTexArray(m_TexDataArray);

	m_BaseTileRect.set(glm::vec2(), viewSize);

	///////////////////////////////////////////////////////////////

	if (!m_DSPContext)
	{
		m_DSPContext.reset(new DSP::CContext());
		m_DSPContext->insert(new DSP::EnvelopeFilter());
	}

	m_DSPContext->initialize(
		m_Player->getNumChannels(),
		m_Player->getSampleRate(), 
		m_SoundSwap.size());
	
	////////////////////////////////////////////////////////////////

	ofFboSettings fboSettings;
	fboSettings.width = m_TileTextureSize.x;
	fboSettings.height = m_TileTextureSize.y;
	fboSettings.internalformat = GL_RGBA32F;
	m_RenderFbo.allocate(fboSettings);
}

void AVSoundPlayerView::clearTiles()
{

}

void AVSoundPlayerView::registerEvents()
{
	auto player = static_cast<AVSoundPlayer*>(m_Player);
	auto &events = player->getEvents();
	ofAddListener(events.fill, this, &AVSoundPlayerView::onPlayerFill);
	ofAddListener(events.seek, this, &AVSoundPlayerView::onPlayerSeek);
	ofAddListener(events.load, this, &AVSoundPlayerView::onPlayerLoad);
	ofAddListener(events.unload, this, &AVSoundPlayerView::onPlayerUnload);
	ofAddListener(events.finish, this, &AVSoundPlayerView::onPlayerFinish);
}

void AVSoundPlayerView::unregisterEvents()
{
	auto player = static_cast<AVSoundPlayer*>(m_Player);
	auto &events = player->getEvents();
	ofRemoveListener(events.fill, this, &AVSoundPlayerView::onPlayerFill);
	ofRemoveListener(events.seek, this, &AVSoundPlayerView::onPlayerSeek);
	ofRemoveListener(events.load, this, &AVSoundPlayerView::onPlayerLoad);
	ofRemoveListener(events.unload, this, &AVSoundPlayerView::onPlayerUnload);
	ofRemoveListener(events.finish, this, &AVSoundPlayerView::onPlayerFinish);
}

ofTexture& AVSoundPlayerView::texTileAt(int index)
{
	return m_TexTileArray.at(index);
}

ofTexture& AVSoundPlayerView::texTileAtWritePos(int offset)
{
	return m_TexTileArray.at(getWritePosition(offset));
}

ofTexture& AVSoundPlayerView::texDataAt(int index)
{
	return m_TexDataArray.at(index);
}

ofTexture& AVSoundPlayerView::texDataAtWritePos(int offset)
{
	return m_TexDataArray.at(getWritePosition(offset));
}

int AVSoundPlayerView::getWritePosition(int offset)
{
	int len = (int)m_TexTileArray.size();
	int ret = (m_TileWriteCursor.position + offset) % len;
	return ret >= 0 ? ret : ret + len;
}

int AVSoundPlayerView::getReadPosition(int offset)
{
	int len = (int)m_TexTileArray.size();
	int ret = (m_TileReadCursor.position + offset) % len;
	return ret >= 0 ? ret : ret + len;
}

AVSoundPlayerView::AVSoundPlayerView(SoundPlayer *player)
	: m_Player(player)
	, m_EncoderProgStatus(0)
	, m_TilesOnScreen(0)
	, m_TilesOffScreen(0)
	//, m_TilePeriod(0)
	, m_TexInternalFormat(0)
	, m_Initialized(false)
{
	player->setAutoPlayEnabled(false);
}

AVSoundPlayerView::~AVSoundPlayerView()
{
	//unregisterEvents(); // problema: causando quebra?
}

void AVSoundPlayerView::setup()
{
	if (!m_Initialized)
	{
		setName("AVSoundPlayerView");
		setBackgroundColor(ofColor::slateGray);
		setForegroundColor(ofColor::darkSlateGray);
		{
			ofStyle style;
			style.bFill = false;
			style.color = ofFloatColor(ofFloatColor::darkSlateGray, 1.0f);
			style.lineWidth = 4.0f;
			style.blendingMode = ofBlendMode::OF_BLENDMODE_DISABLED;
			style.drawBitmapMode = ofDrawBitmapMode::OF_BITMAPMODE_MODEL;
			m_FGStyle = style;
		}
		registerEvents();
		reload();
		m_Initialized = true;
	}
}
  
void AVSoundPlayerView::reload()
{
	m_EncoderProgStatus = 0;

	if (m_EncoderProg.load("shaders/tile"))
	{
		glGetProgramiv(m_EncoderProg.getProgram(), GL_LINK_STATUS, &m_EncoderProgStatus);

		if (m_EncoderProgStatus)
		{
			m_EncoderProg.printActiveAttributes();
			m_EncoderProg.printActiveUniformBlocks();
			m_EncoderProg.printActiveUniforms();
		}
	}
}

int AVSoundPlayerView::getSampleSize()
{
	return sizeof(BaseSampleType);
}

glm::ivec2 AVSoundPlayerView::getTileSize()
{
	return m_TileTextureSize;
}

int AVSoundPlayerView::getNumTiles()
{
	return m_TilesOnScreen + m_TilesOffScreen;
}

int AVSoundPlayerView::getNumSamples()
{
	return getNumTiles() * getNumSamplesPerTile();
}

int AVSoundPlayerView::getNumSamplesPerTile()
{
	return m_TileTextureSize.x * m_TileTextureSize.y;
}

int AVSoundPlayerView::getNumChannels()
{
	return m_Player->getNumChannels();
}

size_t AVSoundPlayerView::getBufferSize() 
{ 
	return m_SoundFifo.getSize(); 
}

size_t AVSoundPlayerView::getBufferReadAvailable()
{
	return m_SoundFifo.getAvailableRead();  
}

size_t AVSoundPlayerView::getBufferFillAvailable() 
{ 
	return m_SoundFifo.getAvailableWrite(); 
}
