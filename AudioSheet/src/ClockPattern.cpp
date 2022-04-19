#include "ClockPattern.h"
#include "ofApp.h"
#include <random>

ClockPattern::ClockPattern(ofBaseApp* baseApp)
	: baseApp(baseApp)
{}

ClockPattern::~ClockPattern()
{}

void ClockPattern::setup()
{
	shader.load("shaders/pattern");

	std::default_random_engine gen;
	gen.seed(std::time(nullptr));
	std::uniform_int_distribution<uint32_t> dis(0, 255);
	auto rand = std::bind(dis, gen);

	noiseTexture.allocate(1024, 1024, GL_RGBA, false);
	std::vector<uint8_t> noiseBuffer(noiseTexture.getWidth()*noiseTexture.getHeight());
	std::generate(noiseBuffer.begin(), noiseBuffer.end(), rand);
	noiseTexture.loadData(noiseBuffer.data(), noiseTexture.getWidth(), noiseTexture.getHeight(), GL_RED, GL_UNSIGNED_BYTE);

	m_Tracks = 8;
	m_Bandguard = 40;
	m_MarginX = glm::vec2(40);
	m_MarginY = glm::vec2(40);
}

void ClockPattern::draw(float x, float y, float w, float h) const
{
	draw(ofRectangle(x, y, w, h));
}

void ClockPattern::draw(const ofRectangle & rect) const
{
	if (plane.getWidth() != rect.getWidth() || plane.getHeight() != rect.getHeight())
	{
		plane.set(rect.getWidth(), rect.getHeight());
		plane.mapTexCoords(1, 1, 0, 0);
	}

	if (shader.isLoaded())
	{
		shader.begin();
		shader.setUniformTexture("channel0.tex", noiseTexture, 0);
		shader.setUniform2f("window.pos", rect.getPosition());
		shader.setUniform2f("window.siz", glm::vec2(rect.getWidth(), rect.getHeight()));
		shader.setUniform1i("page.tracks", m_Tracks);
		shader.setUniform1i("page.bandguard", m_Bandguard);
		shader.setUniform2f("page.margin.xrange", m_MarginX);
		shader.setUniform2f("page.margin.yrange", m_MarginY);
		plane.setPosition(rect.getCenter());
		plane.draw();
		shader.end();
	}
}

float ClockPattern::getHeight() const
{
	return 0.0f;
}

float ClockPattern::getWidth() const
{
	return 0.0f;
}
