#pragma once

#include "View.h"
#include "SheetCanvas.h"

class SheetCanvasView : public View
{
private:
	void update() override;
	void draw() override;
	SheetCanvas* m_SheetCanvas;

public:
	SheetCanvasView(SheetCanvas*);
	const glm::vec2& getDimension();
	const glm::vec2& getResolution();
	const int& getDotsPerInch();

};