#include "SheetCanvasView.h"
#include <ofGraphics.h>

SheetCanvasView::SheetCanvasView(SheetCanvas *sheetCanvas)
	: m_SheetCanvas(sheetCanvas)
{
	setName("SheetCanvas");
}

const glm::vec2 & SheetCanvasView::getDimension() { return m_SheetCanvas->getDimension(); }

const glm::vec2 & SheetCanvasView::getResolution() { return m_SheetCanvas->getResolution(); }

const int & SheetCanvasView::getDotsPerInch() { return m_SheetCanvas->getDPI(); }

void SheetCanvasView::update()
{
	m_SheetCanvas->update();
}

void SheetCanvasView::draw()
{
	auto fbo = m_SheetCanvas->getFbo();
	auto tex = fbo.getTexture();

	if (tex.isAllocated())
	{
		auto viewRect = getRect();
		viewRect = ofRectangle(glm::vec2(), viewRect.width, viewRect.height);

		auto texRect = ofRectangle(glm::vec2(), tex.getWidth(), tex.getHeight());

		auto texDrawRect = texRect;
		texDrawRect.scale(viewRect.getHeight() / texDrawRect.getHeight());
		texDrawRect.setX((viewRect.getCenter() - texDrawRect.getCenter()).x);

		if (viewRect.getWidth() < texDrawRect.getWidth())
		{
			texDrawRect = texRect;
			texDrawRect.scale(viewRect.getWidth() / texDrawRect.getWidth());
			texDrawRect.setY((viewRect.getCenter() - texDrawRect.getCenter()).y);
		}

		tex.draw(texDrawRect);
	}
}