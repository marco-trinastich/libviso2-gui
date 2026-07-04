#include "core_utils.h"
#include "gui_utils.h"

using namespace Gdiplus;

using namespace VisoGui::Core::Utils;

void GuiUtils::DrawLine(Graphics &graphics, int x1, int y1, int x2, int y2)
{
	auto linePen = GuiUtils::CreateLinePen();
	graphics.DrawLine(linePen.get(), x1, y1, x2, y2);
}

void GuiUtils::DrawRectangle(Graphics &graphics, int x1, int y1, int x2, int y2)
{
	auto defaultPen = GuiUtils::CreateDefaultPen();
	graphics.DrawRectangle(defaultPen.get(), x1, y1, x2, y2);
}

void GuiUtils::DrawString(Graphics &graphics, const std::string &text, int x, int y)
{
	auto defaultFont = GuiUtils::CreateDefaultFont();
	auto defaultBrush = GuiUtils::CreateDefaultBrush();
	auto w_text = CoreUtils::GetWchar(text);
	PointF pointF(x, y);
	graphics.DrawString(w_text.get(), -1, defaultFont.get(), pointF, defaultBrush.get());
}

void GuiUtils::ClearArea(Graphics &graphics, int x1, int y1, int width, int height, COLORREF color)
{
	auto backgroundBrush = GuiUtils::CreateBrushFromColor(color);
	graphics.FillRectangle(backgroundBrush.get(), x1, y1, width, height);
}

std::unique_ptr<Pen> GuiUtils::CreateDefaultPen()
{
	return std::make_unique<Pen>(Color(255, 0, 0, 0));
}

std::unique_ptr<Pen> GuiUtils::CreateLinePen()
{
	return std::make_unique<Pen>(Color(255, 0, 0, 255));
}

std::unique_ptr<SolidBrush> GuiUtils::CreateDefaultBrush()
{
	return std::make_unique<SolidBrush>(Color(255, 0, 0, 0));
}

std::unique_ptr<SolidBrush> GuiUtils::CreateBrushFromColor(COLORREF color)
{
	Color backgroundColor;
	backgroundColor.SetFromCOLORREF(color);
	return std::make_unique<SolidBrush>(backgroundColor);
}

std::unique_ptr<Font> GuiUtils::CreateDefaultFont()
{
	auto fontFamily = std::make_unique<FontFamily>(L"Arial");
	return std::make_unique<Font>(
		fontFamily.get(),
		12,
		FontStyleRegular,
		Gdiplus::UnitPixel);
}